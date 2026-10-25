/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef DISC_COPY_TASK_H
#define DISC_COPY_TASK_H

#include <String.h>
#include <SupportDefs.h>

#include "../util/ExternalProcessLineReader.h"

struct DiscCopyResult {
	status_t	error;			// B_OK se successo
	BString		errorMessage;	// valido solo se error != B_OK
};

// Copia disco (scope esteso SRS, Milestone 8): legge un disco sorgente già
// masterizzato byte per byte in un file immagine tramite "readcd". A
// differenza del flusso file/dir->masterizzazione, qui la sorgente ha già
// un proprio filesystem (ISO9660/UDF), quindi non si passa per
// l'incapsulamento BFS: e' una lettura raw sequenziale.
//
// Nota di scope: l'eventuale fallback a "dd" quando readcd fallisce su un
// device specifico (previsto dal piano) richiederebbe di mappare
// l'indirizzo SCSI bus,id,lun di cdrecord/readcd al nodo /dev/disk/...
// corrispondente, cosa non implementata qui: senza un masterizzatore reale
// su cui verificarla, l'ho lasciata come evoluzione futura documentata
// piuttosto che codice non testabile.
class DiscCopyTask {
public:
			DiscCopyResult		Run(const BString& sourceDeviceAddress,
									const BString& outputImagePath,
									bool hasReadcd,
									ProcessLineReporter* reporter = NULL,
									ExternalProcess** activeProcessOut = NULL);
};

#endif // DISC_COPY_TASK_H
