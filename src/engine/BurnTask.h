/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef BURN_TASK_H
#define BURN_TASK_H

#include <String.h>
#include <SupportDefs.h>

#include "BackendCapabilities.h"
#include "../util/ExternalProcessLineReader.h"

struct BurnResult {
	status_t	error;			// B_OK se successo
	BString		errorMessage;	// valido solo se error != B_OK
};

// Fase 5 SRS: passa l'immagine binaria al backend di masterizzazione
// (cdrecord o, in sua assenza, xorriso in modalita' compatibilita'
// cdrecord). Nessuna dipendenza UI, testabile da CLI
// (src/test/test_backend.cpp) anche senza un masterizzatore reale grazie
// alla modalita' "dummy" di cdrecord/xorriso (scrive tutto il percorso di
// codice senza accendere il laser). Il progresso (ProcessLineReporter) usa
// la stessa interfaccia di DiscCopyTask (Milestone 8).
class BurnTask {
public:
				BurnResult			Run(BurnBackend backend,
										const BString& imagePath,
										const BString& deviceAddress,
										bool dummyMode,
										ProcessLineReporter* reporter = NULL,
										ExternalProcess** activeProcessOut = NULL);
};

#endif // BURN_TASK_H
