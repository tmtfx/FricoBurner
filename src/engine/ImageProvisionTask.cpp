/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "ImageProvisionTask.h"

#include <vector>

#include <File.h>

#include <string.h>

#include "../util/ExternalProcess.h"


ProvisionResult
ImageProvisionTask::Run(const BString& imagePath, uint64 sizeBytes,
	const BString& volumeName)
{
	ProvisionResult result;
	result.error = B_OK;

	BFile file(imagePath.String(), B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
	status_t status = file.InitCheck();
	if (status != B_OK) {
		result.error = status;
		result.errorMessage.SetToFormat(
			"Impossibile creare il file immagine '%s': %s",
			imagePath.String(), strerror(status));
		return result;
	}

	status = file.SetSize((off_t)sizeBytes);
	if (status != B_OK) {
		result.error = status;
		result.errorMessage.SetToFormat(
			"Impossibile dimensionare il file immagine a %llu byte: %s",
			(unsigned long long)sizeBytes, strerror(status));
		return result;
	}
	file.Unset();

	// Sintassi verificata manualmente (Milestone 0/4): "mkbfs" non esiste su
	// questa installazione, si usa "mkfs -t bfs". "-q" evita il prompt di
	// conferma interattivo, che altrimenti bloccherebbe il processo.
	std::vector<BString> argv;
	argv.push_back("mkfs");
	argv.push_back("-t");
	argv.push_back("bfs");
	argv.push_back("-q");
	argv.push_back("-o");
	argv.push_back("block_size 2048");
	argv.push_back(imagePath);
	argv.push_back(volumeName);

	ExternalProcess process;
	ExternalProcessResult procResult = process.Run(argv);

	if (procResult.exitCode != 0) {
		result.error = B_ERROR;
		result.errorMessage.SetToFormat(
			"mkfs ha restituito il codice %d:\n%s",
			procResult.exitCode, procResult.output.String());
	}

	return result;
}
