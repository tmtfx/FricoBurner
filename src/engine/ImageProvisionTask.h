/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IMAGE_PROVISION_TASK_H
#define IMAGE_PROVISION_TASK_H

#include <String.h>
#include <SupportDefs.h>

struct ProvisionResult {
	status_t	error;			// B_OK se successo
	BString		errorMessage;	// valido solo se error != B_OK
};

// Fase 3 SRS: crea il file immagine vuoto della dimensione richiesta e lo
// formatta BFS. Nessuna dipendenza UI, testabile da CLI
// (src/test/test_image_engine.cpp).
class ImageProvisionTask {
public:
			ProvisionResult		Run(const BString& imagePath,
									uint64 sizeBytes,
									const BString& volumeName);
};

#endif // IMAGE_PROVISION_TASK_H
