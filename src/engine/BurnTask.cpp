/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "BurnTask.h"

#include <vector>

#include "../util/ExternalProcess.h"


BurnResult
BurnTask::Run(BurnBackend backend, const BString& imagePath,
	const BString& deviceAddress, bool dummyMode, ProcessLineReporter* reporter,
	ExternalProcess** activeProcessOut)
{
	BurnResult result;
	result.error = B_OK;

	if (backend == BACKEND_NONE) {
		result.error = B_NOT_ALLOWED;
		result.errorMessage = "Nessun backend di masterizzazione disponibile "
			"(cdrecord/xorriso non trovati sul sistema).";
		return result;
	}

	if (deviceAddress.Length() == 0) {
		result.error = B_BAD_VALUE;
		result.errorMessage = "Nessun masterizzatore ottico selezionato.";
		return result;
	}

	// xorriso fornisce una modalita' di compatibilita' con la riga di
	// comando di cdrecord ("-as cdrecord"), usata qui come fallback quando
	// cdrecord stesso non e' disponibile (vedi piano, rischio "copertura
	// DVD senza growisofs").
	std::vector<BString> argv;
	if (backend == BACKEND_XORRISO) {
		argv.push_back("xorriso");
		argv.push_back("-as");
		argv.push_back("cdrecord");
	} else {
		argv.push_back("cdrecord");
	}

	BString devArg("dev=");
	devArg << deviceAddress;

	argv.push_back(devArg);
	argv.push_back("-v");
	argv.push_back("-eject");
	argv.push_back("-dao");
	if (dummyMode)
		argv.push_back("-dummy");
	argv.push_back("-data");
	argv.push_back(imagePath);

	LineReaderContext ctx;
	ctx.reporter = reporter;

	ExternalProcess process;
	if (activeProcessOut != NULL)
		*activeProcessOut = &process;

	ExternalProcessResult procResult = process.Run(argv,
		ExternalProcessLineCallback, &ctx);

	if (activeProcessOut != NULL)
		*activeProcessOut = NULL;

	if (procResult.exitCode != 0) {
		result.error = B_ERROR;
		result.errorMessage.SetToFormat("%s ha restituito il codice %d:\n%s",
			backend == BACKEND_XORRISO ? "xorriso" : "cdrecord",
			procResult.exitCode, procResult.output.String());
	}

	return result;
}
