/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "ImagePopulateTask.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <Node.h>
#include <OS.h>
#include <Path.h>
#include <fs_attr.h>

#include "../util/ExternalProcess.h"


static void
CopyAttributes(BNode& source, BNode& dest)
{
	char attrName[B_ATTR_NAME_LENGTH];
	source.RewindAttrs();
	while (source.GetNextAttrName(attrName) == B_OK) {
		attr_info info;
		if (source.GetAttrInfo(attrName, &info) != B_OK)
			continue;

		void* buffer = malloc((size_t)info.size);
		if (buffer == NULL)
			continue;

		ssize_t bytesRead = source.ReadAttr(attrName, info.type, 0, buffer,
			(size_t)info.size);
		if (bytesRead == info.size)
			dest.WriteAttr(attrName, info.type, 0, buffer, (size_t)info.size);

		free(buffer);
	}
}


// B_CANCELED: usato come segnale interno "annullato dall'utente", distinto
// da un vero errore di copia -- ImagePopulateTask::Run lo riconosce per
// mostrare il messaggio giusto invece di un codice/strerror confuso.
static const status_t kCanceled = B_CANCELED;


static bool
IsAborted(const volatile bool* abortFlag)
{
	return abortFlag != NULL && *abortFlag;
}


// Isolata dalla ricorsione di CopyRecursive apposta: un buffer da 64KB come
// variabile locale di una funzione ricorsiva moltiplica per la profondita'
// di annidamento delle cartelle, esaurendo in fretta lo stack (limitato) del
// thread worker. Tenendolo in una funzione non ricorsiva, ogni copia di file
// costa 64KB una volta sola, non una volta per livello di annidamento.
static status_t
CopyFileContents(BFile& sourceFile, BFile& destFile,
	const volatile bool* abortFlag)
{
	char buffer[65536];
	ssize_t bytesRead;
	while ((bytesRead = sourceFile.Read(buffer, sizeof(buffer))) > 0) {
		if (IsAborted(abortFlag))
			return kCanceled;

		ssize_t bytesWritten = destFile.Write(buffer, bytesRead);
		if (bytesWritten != bytesRead)
			return B_IO_ERROR;
	}

	return B_OK;
}


static status_t
CopyRecursive(BEntry& source, BDirectory& destParent,
	const volatile bool* abortFlag)
{
	if (IsAborted(abortFlag))
		return kCanceled;

	char name[B_FILE_NAME_LENGTH];
	status_t status = source.GetName(name);
	if (status != B_OK)
		return status;

	struct stat st;
	status = source.GetStat(&st);
	if (status != B_OK)
		return status;

	if (S_ISDIR(st.st_mode)) {
		BDirectory newDir;
		status = destParent.CreateDirectory(name, &newDir);
		if (status != B_OK)
			return status;

		BNode sourceNode(&source);
		CopyAttributes(sourceNode, newDir);

		BDirectory sourceDir(&source);
		BEntry child;
		while (sourceDir.GetNextEntry(&child) == B_OK) {
			status = CopyRecursive(child, newDir, abortFlag);
			if (status != B_OK)
				return status;
		}
	} else {
		BFile sourceFile(&source, B_READ_ONLY);
		status = sourceFile.InitCheck();
		if (status != B_OK)
			return status;

		BFile destFile;
		status = destParent.CreateFile(name, &destFile);
		if (status != B_OK)
			return status;

		status = CopyFileContents(sourceFile, destFile, abortFlag);
		if (status != B_OK)
			return status;

		CopyAttributes(sourceFile, destFile);
	}

	return B_OK;
}


static status_t
UnmountVolume(const BString& mountPoint)
{
	// Anche quando tutti gli handle (BDirectory/BEntry/BFile) sono stati
	// chiusi e sono usciti di scope prima di arrivare qui, si e' osservato
	// in test che "unmount" puo' fallire con "device busy" per una breve
	// finestra di tempo prima che il kernel registri effettivamente il nodo
	// come libero. Un paio di tentativi con una piccola pausa risolvono la
	// race senza bisogno di una vera sincronizzazione col VFS.
	const int32 kMaxAttempts = 5;
	ExternalProcessResult result;

	for (int32 attempt = 0; attempt < kMaxAttempts; attempt++) {
		std::vector<BString> argv;
		argv.push_back("unmount");
		argv.push_back(mountPoint);

		ExternalProcess process;
		result = process.Run(argv);
		if (result.exitCode == 0)
			return B_OK;

		snooze(100000); // 100ms
	}

	return B_ERROR;
}


PopulateResult
ImagePopulateTask::Run(const BString& imagePath, const BString& mountPoint,
	const std::vector<BString>& sourcePaths, const volatile bool* abortFlag)
{
	PopulateResult result;
	result.error = B_OK;

	if (mkdir(mountPoint.String(), 0755) != 0 && errno != EEXIST) {
		result.error = errno;
		result.errorMessage.SetToFormat(
			"Impossibile creare il mountpoint '%s': %s",
			mountPoint.String(), strerror(errno));
		return result;
	}

	std::vector<BString> mountArgv;
	mountArgv.push_back("mount");
	mountArgv.push_back("-t");
	mountArgv.push_back("bfs");
	mountArgv.push_back(imagePath);
	mountArgv.push_back(mountPoint);

	ExternalProcess mountProcess;
	ExternalProcessResult mountResult = mountProcess.Run(mountArgv);
	if (mountResult.exitCode != 0) {
		result.error = B_ERROR;
		result.errorMessage.SetToFormat("mount ha fallito (codice %d):\n%s",
			mountResult.exitCode, mountResult.output.String());
		return result;
	}

	status_t status;

	// Scope dedicato: BDirectory/BEntry/BFile tengono aperto un nodo sul
	// volume montato finche' non vengono distrutti. Se restano vivi oltre
	// questo punto, il successivo unmount fallisce con "device busy"
	// (capitato in test: lo stesso processo che ha appena copiato i file
	// teneva ancora destDir aperta). Vanno quindi fatti uscire di scope
	// esplicitamente prima di smontare.
	{
		BDirectory destDir(mountPoint.String());
		status = destDir.InitCheck();
		if (status != B_OK) {
			result.error = status;
			result.errorMessage.SetToFormat(
				"Impossibile aprire il mountpoint '%s': %s",
				mountPoint.String(), strerror(status));
			UnmountVolume(mountPoint);
			return result;
		}

		for (size_t i = 0; i < sourcePaths.size(); i++) {
			BEntry entry(sourcePaths[i].String(), true);
			if (!entry.Exists())
				continue;

			status = CopyRecursive(entry, destDir, abortFlag);
			if (status != B_OK) {
				result.error = status;
				if (status == kCanceled) {
					result.errorMessage = "Operazione annullata dall'utente.";
				} else {
					result.errorMessage.SetToFormat(
						"Copia fallita per '%s': %s",
						sourcePaths[i].String(), strerror(status));
				}
				break;
			}
		}
	}

	// Smonta sempre, anche se la copia e' fallita a meta': non va mai
	// lasciato un volume montato su un errore (vedi piano, rischio §7.3).
	status_t unmountStatus = UnmountVolume(mountPoint);
	if (result.error == B_OK && unmountStatus != B_OK) {
		result.error = unmountStatus;
		result.errorMessage = "unmount ha fallito dopo la copia.";
	}

	return result;
}
