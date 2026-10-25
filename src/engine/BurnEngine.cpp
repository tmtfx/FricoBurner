/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "BurnEngine.h"

#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>

#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>

#include "BackendCapabilities.h"
#include "BurnTask.h"
#include "DiscCopyTask.h"
#include "ImagePopulateTask.h"
#include "ImageProvisionTask.h"
#include "ScanTask.h"
#include "../util/SizeEstimator.h"

#include <cstdlib>
#include <regex>

namespace {

struct WorkerArgs {
	BurnEngine*				engine;
	int32					command;
	std::vector<BString>	sourcePaths;
	uint64					capacityBytes;
	BString					deviceAddress;
	BString					sourceDeviceAddress;
	bool					copyToDisc;
};

} // namespace


// Inoltra le righe di output di cdrecord/xorriso (Milestone 6) o readcd
// (Milestone 8) come progresso della fase indicata. Se una riga contiene il
// pattern "N of M MB written" (formato di avanzamento di cdrecord) calcola
// la percentuale reale, altrimenti mantiene l'ultima percentuale nota e
// inoltra solo il testo (readcd non usa questo formato, quindi la barra
// resta ferma ma il messaggio di stato si aggiorna comunque).
class BurnEngine::BurnReporter : public ProcessLineReporter {
public:
	BurnReporter(BurnEngine* engine, BurnPhase phase)
		:
		fEngine(engine),
		fPhase(phase),
		fPercent(0.0f)
	{
	}

	virtual void OnProcessLine(const char* line)
	{
		static const std::regex kWrittenPattern(
			R"((\d+) of (\d+) MB written)");

		std::cmatch match;
		if (std::regex_search(line, match, kWrittenPattern)) {
			double current = std::atof(match[1].str().c_str());
			double total = std::atof(match[2].str().c_str());
			if (total > 0)
				fPercent = (float)((current / total) * 100.0);
		}

		fEngine->_SendProgress(fPhase, fPercent, line);
	}

private:
	BurnEngine*	fEngine;
	BurnPhase	fPhase;
	float		fPercent;
};


BurnEngine::BurnEngine()
	:
	BLooper("BurnEngine"),
	fPhase(PHASE_IDLE),
	fAbortRequested(false),
	fWorkerThread(-1),
	fActiveProcess(NULL)
{
	Run();
}


BurnEngine::~BurnEngine()
{
}


void
BurnEngine::SetProgressTarget(const BMessenger& target)
{
	fProgressTarget = target;
}


void
BurnEngine::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_START_SCAN:
		case MSG_START_PROVISION:
		case MSG_START_POPULATE:
		case MSG_START_BURN:
		case MSG_START_DISC_COPY:
		{
			std::vector<BString> sourcePaths;
			const char* path;
			int32 i = 0;
			while (message->FindString(FIELD_SOURCE_PATH, i++, &path) == B_OK)
				sourcePaths.push_back(BString(path));

			int64 capacityBytes = 0;
			message->FindInt64(FIELD_CAPACITY_BYTES, &capacityBytes);

			BString deviceAddress;
			message->FindString(FIELD_DEVICE_ADDRESS, &deviceAddress);

			BString sourceDeviceAddress;
			message->FindString(FIELD_SOURCE_DEVICE_ADDRESS, &sourceDeviceAddress);

			bool copyToDisc = false;
			message->FindBool(FIELD_COPY_TO_DISC, &copyToDisc);

			_StartWorker(message->what, sourcePaths, (uint64)capacityBytes,
				deviceAddress, sourceDeviceAddress, copyToDisc);
			break;
		}

		case MSG_ABORT:
			fAbortRequested = true;
			// mkfs/mount/unmount sono quasi istantanei e non vale la pena
			// interromperli; cdrecord/xorriso/readcd invece possono girare
			// a lungo, quindi se uno di questi e' in esecuzione lo si
			// termina subito (SIGTERM) invece di aspettare che finisca da
			// solo prima di notare l'annullamento.
			if (fActiveProcess != NULL)
				fActiveProcess->Cancel();
			break;

		default:
			BLooper::MessageReceived(message);
			break;
	}
}


void
BurnEngine::_StartWorker(int32 command, const std::vector<BString>& sourcePaths,
	uint64 capacityBytes, const BString& deviceAddress,
	const BString& sourceDeviceAddress, bool copyToDisc)
{
	if (fWorkerThread >= 0)
		return;

	fAbortRequested = false;

	WorkerArgs* args = new WorkerArgs {
		this, command, sourcePaths, capacityBytes, deviceAddress,
		sourceDeviceAddress, copyToDisc };

	fWorkerThread = spawn_thread(_WorkerEntry, "BurnEngine worker",
		B_NORMAL_PRIORITY, args);
	if (fWorkerThread >= 0)
		resume_thread(fWorkerThread);
}


int32
BurnEngine::_WorkerEntry(void* data)
{
	WorkerArgs* args = static_cast<WorkerArgs*>(data);
	BurnEngine* engine = args->engine;
	int32 command = args->command;
	std::vector<BString> sourcePaths = args->sourcePaths;
	uint64 capacityBytes = args->capacityBytes;
	BString deviceAddress = args->deviceAddress;
	BString sourceDeviceAddress = args->sourceDeviceAddress;
	bool copyToDisc = args->copyToDisc;
	delete args;

	engine->_RunWorker(command, sourcePaths, capacityBytes, deviceAddress,
		sourceDeviceAddress, copyToDisc);
	engine->fWorkerThread = -1;
	return 0;
}


bool
BurnEngine::_PrepareWorkPaths(BString& imagePath, BString& mountPoint,
	const char* extension)
{
	BPath cacheDir;
	if (find_directory(B_USER_CACHE_DIRECTORY, &cacheDir) != B_OK)
		return false;

	cacheDir.Append("FricoBurner");
	if (mkdir(cacheDir.Path(), 0755) != 0 && errno != EEXIST)
		return false;

	// Nome univoco per team (processo): riusare sempre lo stesso path
	// "image.bfs" si e' dimostrato, in test ripetuti, occasionalmente
	// soggetto a uno stato "sporco" nel disk device manager di Haiku dopo
	// molti cicli di formattazione sullo stesso path (vedi piano, rischio
	// "cleanup dopo crash" - qui applicato in modo preventivo, non solo
	// reattivo). Verra' sostituito da una gestione piu' completa con
	// TempImageRegistry nella Milestone 9.
	BString imageFileName;
	imageFileName.SetToFormat("image-%" B_PRId32 ".%s", (int32)getpid(),
		extension);

	BPath imageFilePath(cacheDir);
	imageFilePath.Append(imageFileName.String());
	imagePath = imageFilePath.Path();

	BString mountDirName;
	mountDirName.SetToFormat("mount-%" B_PRId32, (int32)getpid());

	BPath mountDirPath(cacheDir);
	mountDirPath.Append(mountDirName.String());
	mountPoint = mountDirPath.Path();

	return true;
}


void
BurnEngine::_RunWorker(int32 command, const std::vector<BString>& sourcePaths,
	uint64 capacityBytes, const BString& deviceAddress,
	const BString& sourceDeviceAddress, bool copyToDisc)
{
	if (command == MSG_START_DISC_COPY) {
		_RunDiscCopy(sourceDeviceAddress, copyToDisc, deviceAddress);
		return;
	}

	fCurrentImagePath = "";
	fCurrentMountPoint = "";

	BString imagePath, mountPoint;
	if (!_PrepareWorkPaths(imagePath, mountPoint)) {
		_SendError("Impossibile preparare la cartella di lavoro temporanea.");
		return;
	}
	fCurrentImagePath = imagePath;
	fCurrentMountPoint = mountPoint;

	// Fase 2: scansione (ScanTask reale, Milestone 3).
	fPhase = PHASE_SCANNING;
	_SendProgress(PHASE_SCANNING, 0, "Analisi dei file selezionati...");

	ScanTask scanTask;
	ScanResult scanResult = scanTask.Scan(sourcePaths);
	uint64 combined = scanResult.dataBytes + scanResult.attrBytes;

	if (fAbortRequested) {
		_SendError("Operazione annullata dall'utente.");
		return;
	}

	SizeEstimate estimate = SizeEstimator::Estimate(combined, capacityBytes);
	if (estimate.exceedsCapacity) {
		BString message;
		message.SetToFormat(
			"I file selezionati richiedono circa %.1f MB, che superano la "
			"capacita' del supporto scelto (%.1f MB).",
			estimate.totalBytes / (1024.0 * 1024.0),
			capacityBytes / (1024.0 * 1024.0));
		_SendError(message.String());
		return;
	}

	BString scanMessage;
	scanMessage.SetToFormat("Trovati %u file in %u cartelle (%.1f MB).",
		scanResult.fileCount, scanResult.dirCount,
		combined / (1024.0 * 1024.0));
	_SendProgress(PHASE_SCANNING, 100, scanMessage.String());

	// Fase 3: provisioning immagine (ImageProvisionTask reale, Milestone 4).
	fPhase = PHASE_PROVISIONING;
	_SendProgress(PHASE_PROVISIONING, 0, "Creazione immagine BFS...");

	ImageProvisionTask provisionTask;
	ProvisionResult provisionResult = provisionTask.Run(imagePath,
		estimate.totalBytes, "FricoBurner");
	if (provisionResult.error != B_OK) {
		_SendError(provisionResult.errorMessage.String());
		return;
	}

	if (fAbortRequested) {
		_SendError("Operazione annullata dall'utente.");
		return;
	}
	_SendProgress(PHASE_PROVISIONING, 100, "Immagine creata.");

	// Fase 4: popolamento, mount+copia attributi+unmount (ImagePopulateTask
	// reale, Milestone 4).
	fPhase = PHASE_POPULATING;
	_SendProgress(PHASE_POPULATING, 0, "Copia dati con attributi estesi...");

	ImagePopulateTask populateTask;
	PopulateResult populateResult = populateTask.Run(imagePath, mountPoint,
		sourcePaths, &fAbortRequested);
	if (populateResult.error != B_OK) {
		_SendError(populateResult.errorMessage.String());
		return;
	}
	_SendProgress(PHASE_POPULATING, 100, "Copia completata.");

	if (command == MSG_START_BURN) {
		if (fAbortRequested) {
			_SendError("Operazione annullata dall'utente.");
			return;
		}

		if (!_RunBurnPhase(imagePath, deviceAddress))
			return; // _RunBurnPhase ha gia' inviato l'errore o l'abort

		BString doneMessage;
		doneMessage.SetToFormat(
			"Masterizzazione completata. Immagine: %s", imagePath.String());
		_SendComplete(PHASE_DONE, doneMessage.String(), imagePath);
	} else {
		BString doneMessage;
		doneMessage.SetToFormat("Immagine BFS generata: %s", imagePath.String());
		_SendComplete(PHASE_DONE, doneMessage.String(), imagePath);
	}
}


bool
BurnEngine::_RunBurnPhase(const BString& imagePath, const BString& deviceAddress)
{
	// Fase 5 SRS: usa il masterizzatore scelto dall'utente in UI
	// (deviceAddress). Si ri-rileva comunque lo stato del sistema adesso,
	// non al momento della scelta in UI, per essere sicuri che il device
	// sia ancora collegato e per scegliere il backend (cdrecord/xorriso)
	// corretto. Se deviceAddress e' vuoto (es. chiamante che non seleziona
	// nulla) si ripiega sul primo masterizzatore rilevato. Se non c'e'
	// alcun tool o alcun device, errore chiaro invece di un finto successo.
	fPhase = PHASE_BURNING;
	_SendProgress(PHASE_BURNING, 0, "Rilevamento masterizzatore...");

	BackendStatus status = BackendCapabilities::Detect();
	BurnBackend backend = status.PreferredBurnBackend();

	if (backend == BACKEND_NONE) {
		_SendError("Nessun tool di masterizzazione disponibile "
			"(cdrecord/xorriso non trovati sul sistema).");
		return false;
	}

	if (status.opticalDevices.empty()) {
		_SendError("Nessun masterizzatore ottico rilevato. Collega un "
			"masterizzatore CD/DVD e riprova.");
		return false;
	}

	const OpticalDevice* device = NULL;
	if (deviceAddress.Length() > 0) {
		for (size_t i = 0; i < status.opticalDevices.size(); i++) {
			if (status.opticalDevices[i].address == deviceAddress) {
				device = &status.opticalDevices[i];
				break;
			}
		}
		if (device == NULL) {
			_SendError("Il masterizzatore selezionato non e' piu' "
				"disponibile. Aggiorna l'elenco e riprova.");
			return false;
		}
	} else {
		device = &status.opticalDevices[0];
	}

	if (fAbortRequested) {
		_SendError("Operazione annullata dall'utente.");
		return false;
	}

	BString startMessage;
	startMessage.SetToFormat("Masterizzazione su %s %s (dev=%s)...",
		device->vendor.String(), device->model.String(), device->address.String());
	_SendProgress(PHASE_BURNING, 0, startMessage.String());

	BurnReporter reporter(this, PHASE_BURNING);
	BurnTask burnTask;
	BurnResult burnResult = burnTask.Run(backend, imagePath, device->address,
		false, &reporter, &fActiveProcess);

	// Controllato prima dell'esito del processo: se e' stato annullato,
	// cdrecord/xorriso terminati da Cancel() (SIGTERM) restituiscono
	// comunque un errore, ma all'utente va mostrato il motivo vero
	// (annullamento), non l'output grezzo del processo ucciso.
	if (fAbortRequested) {
		_SendError("Operazione annullata dall'utente.");
		return false;
	}

	if (burnResult.error != B_OK) {
		_SendError(burnResult.errorMessage.String());
		return false;
	}

	_SendProgress(PHASE_BURNING, 100, "Masterizzazione completata.");
	return true;
}


void
BurnEngine::_RunDiscCopy(const BString& sourceDeviceAddress, bool copyToDisc,
	const BString& destDeviceAddress)
{
	// Modulo Copia Disco (Milestone 8): a differenza del flusso file/dir,
	// la sorgente ha gia' un proprio filesystem (ISO9660/UDF), quindi non
	// si passa per l'incapsulamento BFS: readcd legge il disco raw in
	// un'immagine, che per il caso disco->disco viene poi passata a
	// _RunBurnPhase (stessa logica di masterizzazione del tab "Masterizza
	// File", nessuna duplicazione).
	fCurrentImagePath = "";
	fCurrentMountPoint = "";
	fPhase = PHASE_READING_DISC;
	_SendProgress(PHASE_READING_DISC, 0, "Rilevamento unita' disco...");

	BackendStatus status = BackendCapabilities::Detect();
	if (!status.hasReadcd) {
		_SendError("Tool 'readcd' non disponibile: impossibile leggere il "
			"disco sorgente.");
		return;
	}

	if (sourceDeviceAddress.Length() == 0) {
		_SendError("Nessun disco sorgente selezionato.");
		return;
	}

	BString imagePath, mountPoint;
	if (!_PrepareWorkPaths(imagePath, mountPoint, "img")) {
		_SendError("Impossibile preparare la cartella di lavoro temporanea.");
		return;
	}
	fCurrentImagePath = imagePath;
	fCurrentMountPoint = mountPoint;

	BString startMessage;
	startMessage.SetToFormat("Lettura disco sorgente (dev=%s)...",
		sourceDeviceAddress.String());
	_SendProgress(PHASE_READING_DISC, 0, startMessage.String());

	BurnReporter readReporter(this, PHASE_READING_DISC);
	DiscCopyTask copyTask;
	DiscCopyResult copyResult = copyTask.Run(sourceDeviceAddress, imagePath,
		status.hasReadcd, &readReporter, &fActiveProcess);

	// Vedi nota analoga in _RunBurnPhase: controllare l'abort prima
	// dell'esito del processo mostra il motivo vero quando readcd viene
	// terminato da Cancel().
	if (fAbortRequested) {
		_SendError("Operazione annullata dall'utente.");
		return;
	}

	if (copyResult.error != B_OK) {
		_SendError(copyResult.errorMessage.String());
		return;
	}
	_SendProgress(PHASE_READING_DISC, 100, "Lettura completata.");

	if (!copyToDisc) {
		BString doneMessage;
		doneMessage.SetToFormat("Immagine disco creata: %s", imagePath.String());
		_SendComplete(PHASE_DONE, doneMessage.String(), imagePath);
		return;
	}

	if (!_RunBurnPhase(imagePath, destDeviceAddress))
		return; // _RunBurnPhase ha gia' inviato l'errore o l'abort

	BString doneMessage;
	doneMessage.SetToFormat("Copia disco completata. Immagine: %s",
		imagePath.String());
	_SendComplete(PHASE_DONE, doneMessage.String(), imagePath);
}


void
BurnEngine::_SendProgress(BurnPhase phase, float percent, const char* message)
{
	if (!fProgressTarget.IsValid())
		return;

	BMessage progress(MSG_PHASE_PROGRESS);
	progress.AddInt32(FIELD_PHASE, phase);
	progress.AddFloat(FIELD_PROGRESS_PERCENT, percent);
	progress.AddString(FIELD_MESSAGE, message);
	fProgressTarget.SendMessage(&progress);
}


void
BurnEngine::_SendComplete(BurnPhase finalPhase, const char* message,
	const BString& imagePath)
{
	fPhase = finalPhase;
	if (!fProgressTarget.IsValid())
		return;

	BMessage complete(MSG_PHASE_COMPLETE);
	complete.AddInt32(FIELD_PHASE, finalPhase);
	complete.AddString(FIELD_MESSAGE, message);
	if (imagePath.Length() > 0)
		complete.AddString(FIELD_IMAGE_PATH, imagePath);
	if (fCurrentMountPoint.Length() > 0)
		complete.AddString(FIELD_MOUNT_POINT, fCurrentMountPoint);
	fProgressTarget.SendMessage(&complete);
}


void
BurnEngine::_SendError(const char* message)
{
	fPhase = PHASE_ERROR;
	if (!fProgressTarget.IsValid())
		return;

	BMessage error(MSG_PHASE_ERROR);
	error.AddInt32(FIELD_PHASE, (int32)PHASE_ERROR);
	error.AddString(FIELD_MESSAGE, message);
	// Se un'immagine e' gia' stata creata (anche parziale/annullata a
	// meta'), la UI deve saperlo per poterla eliminare alla chiusura --
	// altrimenti resterebbe come file temporaneo orfano fino al prossimo
	// avvio (TempImageRegistry la pulirebbe comunque, ma non subito).
	if (fCurrentImagePath.Length() > 0)
		error.AddString(FIELD_IMAGE_PATH, fCurrentImagePath);
	if (fCurrentMountPoint.Length() > 0)
		error.AddString(FIELD_MOUNT_POINT, fCurrentMountPoint);
	fProgressTarget.SendMessage(&error);
}
