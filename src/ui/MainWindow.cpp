/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "MainWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Entry.h>
#include <File.h>
#include <FilePanel.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <OS.h>
#include <Path.h>
#include <ScrollView.h>
#include <String.h>
#include <TabView.h>

#include <string.h>

#include "../engine/BackendCapabilities.h"


static BView*
BuildBurnFilesTab(FileListView* fileListView, BButton* addButton,
	BButton* removeButton, MediaPresetMenu* presetMenu,
	BurnerDeviceMenu* deviceMenu, BButton* refreshDevicesButton,
	CapacityBarView* capacityBar, BButton* generateButton, BButton* burnButton,
	BButton* saveImageButton)
{
	BScrollView* scrollView = new BScrollView("file-list-scroll", fileListView,
		0, false, true);

	BView* tab = new BView("burn-files-tab", 0);
	BLayoutBuilder::Group<>(tab, B_VERTICAL, 8)
		.SetInsets(12, 12, 12, 12)
		.Add(scrollView)
		.AddGroup(B_HORIZONTAL, 4)
			.Add(addButton)
			.Add(removeButton)
			.AddGlue()
		.End()
		.Add(presetMenu)
		.AddGroup(B_HORIZONTAL, 4)
			.Add(deviceMenu)
			.Add(refreshDevicesButton)
			.AddGlue()
		.End()
		.Add(capacityBar)
		.AddGroup(B_HORIZONTAL, 4)
			.Add(saveImageButton)
			.AddGlue()
			.Add(generateButton)
			.Add(burnButton)
		.End();
	return tab;
}


// Argomenti per il thread di rilevamento capacita' (vedi
// MainWindow::_RefreshBackendCapabilities): un BMessenger verso la finestra,
// cosi' il thread puo' rimanere libero da ogni altro stato della UI.
struct DetectionThreadArgs {
	BMessenger target;
};


static int32
DetectionThreadEntry(void* data)
{
	DetectionThreadArgs* args = static_cast<DetectionThreadArgs*>(data);
	BMessenger target = args->target;
	delete args;

	BackendStatus status = BackendCapabilities::Detect();

	BMessage result(MSG_BACKEND_DETECTED);
	for (size_t i = 0; i < status.opticalDevices.size(); i++) {
		result.AddString("device_address", status.opticalDevices[i].address);
		result.AddString("device_vendor", status.opticalDevices[i].vendor);
		result.AddString("device_model", status.opticalDevices[i].model);
	}
	target.SendMessage(&result);

	return 0;
}


MainWindow::MainWindow()
	:
	BWindow(BRect(100, 80, 760, 620), "FricoBurner",
		B_TITLED_WINDOW, B_QUIT_ON_WINDOW_CLOSE),
	fOperationRunning(false),
	fQuitAfterAbort(false)
{
	fEngine = new BurnEngine();
	fEngine->SetProgressTarget(BMessenger(this));

	fFileListView = new FileListView("file-list", BMessenger(this));
	fMediaPresetMenu = new MediaPresetMenu("media-preset", BMessenger(this));
	fDeviceMenu = new BurnerDeviceMenu("device-menu");
	fCapacityBar = new CapacityBarView("capacity-bar");
	fCapacityBar->SetValues(0, fMediaPresetMenu->SelectedCapacityBytes());

	fAddButton = new BButton("add-files", "Aggiungi...",
		new BMessage(MSG_ADD_FILES_CLICKED));
	fRemoveButton = new BButton("remove-files", "Rimuovi",
		new BMessage(MSG_REMOVE_FILES_CLICKED));
	fGenerateImageButton = new BButton("generate-image", "Genera immagine standalone",
		new BMessage(MSG_GENERATE_IMAGE_CLICKED));
	fBurnButton = new BButton("burn", "Masterizza",
		new BMessage(MSG_BURN_CLICKED));
	fAbortButton = new BButton("abort", "Annulla",
		new BMessage(MSG_ABORT_CLICKED));
	fAbortButton->SetEnabled(false);
	fRefreshDevicesButton = new BButton("refresh-devices", "Aggiorna",
		new BMessage(MSG_REFRESH_DEVICES_CLICKED));
	fSaveImageButton = new BButton("save-image", "Salva immagine...",
		new BMessage(MSG_SAVE_IMAGE_CLICKED));
	fSaveImageButton->SetEnabled(false);
	fSaveImagePanel = new SaveImagePanel(BMessenger(this));

	fDiscCopyPanel = new DiscCopyPanel("disc-copy-tab");
	fDiscCopyPanel->StartButton()->SetTarget(this);

	fProgressPanel = new ProgressPanel("progress-panel");

	BMenuBar* menuBar = new BMenuBar("menubar");
	BMenu* fileMenu = new BMenu("File");
	fileMenu->AddItem(new BMenuItem("About FricoBurner" B_UTF8_ELLIPSIS,
		new BMessage(MSG_SHOW_ABOUT)));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem("Esci", new BMessage(B_QUIT_REQUESTED), 'Q'));
	menuBar->AddItem(fileMenu);

	BTabView* tabView = new BTabView("tabview", B_WIDTH_FROM_LABEL);
	tabView->AddTab(BuildBurnFilesTab(fFileListView, fAddButton, fRemoveButton,
		fMediaPresetMenu, fDeviceMenu, fRefreshDevicesButton, fCapacityBar,
		fGenerateImageButton, fBurnButton, fSaveImageButton));
	tabView->TabAt(0)->SetLabel("Masterizza File");
	tabView->AddTab(fDiscCopyPanel);
	tabView->TabAt(1)->SetLabel("Copia Disco");

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar)
		.Add(tabView)
		.AddGroup(B_HORIZONTAL, 8)
			.SetInsets(8, 0, 8, 4)
			.Add(fProgressPanel)
			.Add(fAbortButton)
		.End();

	fAddFilePanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL,
		B_FILE_NODE | B_DIRECTORY_NODE, true, new BMessage(MSG_FILES_SELECTED_FOR_ADD));

	_RefreshBackendCapabilities();
}


MainWindow::~MainWindow()
{
	delete fAddFilePanel;
	delete fSaveImagePanel;
	if (fEngine->Lock())
		fEngine->Quit();
}


void
MainWindow::_CleanUpTempImage()
{
	// SRS 2.6: l'immagine temporanea va eliminata alla chiusura della
	// finestra, sia che l'utente l'abbia salvata altrove (in tal caso resta
	// solo la copia salvata) sia che non l'abbia salvata affatto. A questo
	// punto il volume e' gia' stato smontato da ImagePopulateTask (sempre,
	// anche su errore/annullamento), quindi la cartella di mount e' vuota
	// e si puo' eliminare direttamente.
	if (fLastImagePath.Length() > 0) {
		BEntry entry(fLastImagePath.String());
		if (entry.Exists())
			entry.Remove();
		fLastImagePath = "";
	}

	if (fLastMountPoint.Length() > 0) {
		BEntry mountEntry(fLastMountPoint.String());
		if (mountEntry.Exists())
			mountEntry.Remove();
		fLastMountPoint = "";
	}
}


bool
MainWindow::_OperationInProgress() const
{
	return fOperationRunning;
}


void
MainWindow::_RefreshBackendCapabilities()
{
	DetectionThreadArgs* args = new DetectionThreadArgs { BMessenger(this) };
	thread_id thread = spawn_thread(DetectionThreadEntry,
		"BackendCapabilities detect", B_LOW_PRIORITY, args);
	if (thread >= 0)
		resume_thread(thread);
}


void
MainWindow::_UpdateCapacityBar()
{
	uint64 used = fFileListView->TotalApparentSize();
	uint64 capacity = fMediaPresetMenu->SelectedCapacityBytes();
	if (capacity == 0)
		capacity = 1; // "Personalizzato" senza valore impostato: evita divisione per zero
	fCapacityBar->SetValues(used, capacity);
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_ADD_FILES_CLICKED:
			fAddFilePanel->Show();
			break;

		case MSG_FILES_SELECTED_FOR_ADD:
		{
			entry_ref ref;
			int32 i = 0;
			while (message->FindRef("refs", i++, &ref) == B_OK) {
				BEntry entry(&ref, true);
				if (!entry.Exists())
					continue;
				BPath path;
				entry.GetPath(&path);
				fFileListView->AddEntry(path, entry.IsDirectory());
			}
			break;
		}

		case MSG_REMOVE_FILES_CLICKED:
			fFileListView->RemoveSelectedEntries();
			break;

		case MSG_FILE_LIST_CHANGED:
		case MSG_MEDIA_PRESET_CHANGED:
			_UpdateCapacityBar();
			break;

		case MSG_GENERATE_IMAGE_CLICKED:
		case MSG_BURN_CLICKED:
		{
			if (fOperationRunning)
				break;

			std::vector<BString> paths = fFileListView->AllPaths();
			if (paths.empty()) {
				BAlert* alert = new BAlert("Nessun file",
					"Aggiungi almeno un file o una cartella da masterizzare.",
					"OK");
				alert->Go();
				break;
			}

			bool wantsToBurn = (message->what == MSG_BURN_CLICKED);
			if (wantsToBurn && !fDeviceMenu->HasDevices()) {
				BAlert* alert = new BAlert("Nessun masterizzatore",
					"Nessun masterizzatore ottico rilevato. Collegane uno, "
					"premi \"Aggiorna\" e riprova, oppure usa \"Genera "
					"immagine standalone\".", "OK");
				alert->Go();
				break;
			}

			fOperationRunning = true;
			fGenerateImageButton->SetEnabled(false);
			fBurnButton->SetEnabled(false);
			fAbortButton->SetEnabled(true);
			fProgressPanel->Reset();

			int32 command = wantsToBurn ? MSG_START_BURN : MSG_START_PROVISION;

			BMessage startMessage(command);
			for (size_t i = 0; i < paths.size(); i++)
				startMessage.AddString(FIELD_SOURCE_PATH, paths[i]);
			startMessage.AddInt64(FIELD_CAPACITY_BYTES,
				(int64)fMediaPresetMenu->SelectedCapacityBytes());
			if (wantsToBurn) {
				startMessage.AddString(FIELD_DEVICE_ADDRESS,
					fDeviceMenu->SelectedDeviceAddress());
			}

			BMessenger(fEngine).SendMessage(&startMessage);
			break;
		}

		case MSG_START_DISC_COPY_CLICKED:
		{
			if (fOperationRunning)
				break;

			if (!fDiscCopyPanel->SourceMenu()->HasDevices()) {
				BAlert* alert = new BAlert("Nessun disco sorgente",
					"Nessun masterizzatore ottico rilevato. Collegane uno, "
					"premi \"Aggiorna\" nel tab \"Masterizza File\" e "
					"riprova.", "OK");
				alert->Go();
				break;
			}

			bool copyToDisc =
				fDiscCopyPanel->SelectedDestMode() == DISC_COPY_TO_DISC;
			if (copyToDisc && !fDiscCopyPanel->DestDeviceMenu()->HasDevices()) {
				BAlert* alert = new BAlert("Nessuna destinazione",
					"Nessun masterizzatore di destinazione rilevato per la "
					"copia disco->disco.", "OK");
				alert->Go();
				break;
			}

			fOperationRunning = true;
			fAbortButton->SetEnabled(true);
			fProgressPanel->Reset();

			BMessage startMessage(MSG_START_DISC_COPY);
			startMessage.AddString(FIELD_SOURCE_DEVICE_ADDRESS,
				fDiscCopyPanel->SourceMenu()->SelectedDeviceAddress());
			startMessage.AddBool(FIELD_COPY_TO_DISC, copyToDisc);
			if (copyToDisc) {
				startMessage.AddString(FIELD_DEVICE_ADDRESS,
					fDiscCopyPanel->DestDeviceMenu()->SelectedDeviceAddress());
			}

			BMessenger(fEngine).SendMessage(&startMessage);
			break;
		}

		case MSG_DISC_COPY_DEST_MODE_CHANGED:
			// Letta on-demand da fDiscCopyPanel->SelectedDestMode() quando
			// si avvia la copia; non serve reagire live al cambio.
			break;

		case MSG_ABORT_CLICKED:
			BMessenger(fEngine).SendMessage(MSG_ABORT);
			break;

		case MSG_REFRESH_DEVICES_CLICKED:
			_RefreshBackendCapabilities();
			break;

		case MSG_BACKEND_DETECTED:
		{
			std::vector<OpticalDevice> devices;
			const char* address;
			const char* vendor;
			const char* model;
			int32 i = 0;
			while (message->FindString("device_address", i, &address) == B_OK) {
				message->FindString("device_vendor", i, &vendor);
				message->FindString("device_model", i, &model);

				OpticalDevice device;
				device.address = address;
				device.vendor = vendor;
				device.model = model;
				devices.push_back(device);
				i++;
			}
			fDeviceMenu->SetDevices(devices);
			fDiscCopyPanel->SetDevices(devices);
			break;
		}

		case MSG_PHASE_PROGRESS:
			fProgressPanel->HandleEngineMessage(message);
			break;

		case MSG_PHASE_COMPLETE:
		{
			fProgressPanel->HandleEngineMessage(message);
			fOperationRunning = false;
			fGenerateImageButton->SetEnabled(true);
			fBurnButton->SetEnabled(true);
			fAbortButton->SetEnabled(false);

			BString imagePath;
			if (message->FindString(FIELD_IMAGE_PATH, &imagePath) == B_OK
				&& imagePath.Length() > 0) {
				fLastImagePath = imagePath;
				fSaveImageButton->SetEnabled(true);
			}
			BString mountPoint;
			if (message->FindString(FIELD_MOUNT_POINT, &mountPoint) == B_OK)
				fLastMountPoint = mountPoint;

			if (fQuitAfterAbort) {
				// L'operazione e' finita (anche se e' riuscita invece di
				// essere annullata in tempo: l'utente ha comunque chiesto
				// di uscire) -- ora si puo' pulire e chiudere per davvero.
				_CleanUpTempImage();
				be_app->PostMessage(B_QUIT_REQUESTED);
				Quit();
				return;
			}
			break;
		}

		case MSG_PHASE_ERROR:
			fProgressPanel->HandleEngineMessage(message);
			fOperationRunning = false;
			fGenerateImageButton->SetEnabled(true);
			fBurnButton->SetEnabled(true);
			fAbortButton->SetEnabled(false);

			{
				// Un'immagine parziale/annullata a meta' non e' salvabile
				// (fSaveImageButton resta disabilitato), ma va comunque
				// tracciata per essere eliminata alla chiusura invece di
				// restare come file temporaneo orfano (vedi piano,
				// rischio §7.3).
				BString partialImagePath;
				if (message->FindString(FIELD_IMAGE_PATH, &partialImagePath) == B_OK
					&& partialImagePath.Length() > 0) {
					fLastImagePath = partialImagePath;
				}
				BString mountPoint;
				if (message->FindString(FIELD_MOUNT_POINT, &mountPoint) == B_OK)
					fLastMountPoint = mountPoint;
			}

			if (fQuitAfterAbort) {
				_CleanUpTempImage();
				be_app->PostMessage(B_QUIT_REQUESTED);
				Quit();
				return;
			}
			break;

		case MSG_SAVE_IMAGE_CLICKED:
			if (fLastImagePath.Length() > 0)
				fSaveImagePanel->Show();
			break;

		case B_SAVE_REQUESTED:
		{
			entry_ref dirRef;
			const char* name;
			if (message->FindRef("directory", &dirRef) != B_OK
				|| message->FindString("name", &name) != B_OK
				|| fLastImagePath.Length() == 0) {
				break;
			}

			BPath destPath(&dirRef);
			destPath.Append(name);

			status_t status = B_ERROR;
			BFile source(fLastImagePath.String(), B_READ_ONLY);
			if (source.InitCheck() == B_OK) {
				BFile dest(destPath.Path(),
					B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
				status = dest.InitCheck();
				if (status == B_OK) {
					char buffer[65536];
					ssize_t bytesRead;
					while ((bytesRead = source.Read(buffer, sizeof(buffer))) > 0) {
						ssize_t bytesWritten = dest.Write(buffer, bytesRead);
						if (bytesWritten != bytesRead) {
							status = B_IO_ERROR;
							break;
						}
					}
				}
			}

			BAlert* alert;
			if (status == B_OK) {
				BString text;
				text.SetToFormat("Immagine salvata in:\n%s", destPath.Path());
				alert = new BAlert("Salvataggio completato", text.String(), "OK");
			} else {
				BString text;
				text.SetToFormat("Impossibile salvare l'immagine: %s",
					strerror(status));
				alert = new BAlert("Errore", text.String(), "OK",
					NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			}
			alert->Go();
			break;
		}

		case MSG_SHOW_ABOUT:
		{
			AboutWindow* about = new AboutWindow();
			about->Show();
			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool
MainWindow::QuitRequested()
{
	if (_OperationInProgress()) {
		BAlert* alert = new BAlert("Operazione in corso",
			"Un'operazione di masterizzazione e' in corso. Uscire comunque?",
			"Annulla", "Esci comunque", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		int32 choice = alert->Go();
		if (choice == 0)
			return false;

		// Non si chiude subito: cancellare l'immagine temporanea o
		// chiudere mentre il worker ha ancora un mount/processo esterno
		// attivo rischierebbe di lasciare risorse orfane (vedi piano,
		// rischio §7.4). Si manda l'abort (che ora termina davvero
		// mkfs/cdrecord/readcd in corso, vedi BurnEngine::MSG_ABORT) e si
		// aspetta la conferma MSG_PHASE_ERROR prima di completare la
		// chiusura in MessageReceived.
		fQuitAfterAbort = true;
		BMessenger(fEngine).SendMessage(MSG_ABORT);
		return false;
	}

	_CleanUpTempImage();

	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
