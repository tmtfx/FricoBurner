/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "DiscCopyTask.h"

#include <vector>

#include "../util/ExternalProcess.h"


DiscCopyResult
DiscCopyTask::Run(const BString& sourceDeviceAddress,
	const BString& outputImagePath, bool hasReadcd, ProcessLineReporter* reporter,
	ExternalProcess** activeProcessOut)
{
	DiscCopyResult result;
	result.error = B_OK;

	if (!hasReadcd) {
		result.error = B_NOT_ALLOWED;
		result.errorMessage = "Tool 'readcd' non disponibile sul sistema.";
		return result;
	}

	if (sourceDeviceAddress.Length() == 0) {
		result.error = B_BAD_VALUE;
		result.errorMessage = "Nessun disco sorgente selezionato.";
		return result;
	}

	// Sintassi verificata manualmente (Milestone 0): "readcd dev=x,y,z
	// f=output.iso".
	std::vector<BString> argv;
	argv.push_back("readcd");

	BString devArg("dev=");
	devArg << sourceDeviceAddress;
	argv.push_back(devArg);

	BString fileArg("f=");
	fileArg << outputImagePath;
	argv.push_back(fileArg);

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
		result.errorMessage.SetToFormat("readcd ha restituito il codice %d:\n%s",
			procResult.exitCode, procResult.output.String());
	}

	return result;
}
