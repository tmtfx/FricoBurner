/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IMAGE_POPULATE_TASK_H
#define IMAGE_POPULATE_TASK_H

#include <vector>

#include <String.h>
#include <SupportDefs.h>

struct PopulateResult {
	status_t	error;			// B_OK se successo
	BString		errorMessage;	// valido solo se error != B_OK
};

// Fase 4 SRS: monta l'immagine BFS, copia ricorsivamente i file/dir
// sorgente (gli attributi estesi si preservano nativamente perche' la
// destinazione e' un volume BFS reale), smonta sempre -- anche in caso di
// errore durante la copia, per non lasciare il volume montato (vedi
// rischio "cleanup dopo crash" nel piano). Nessuna dipendenza UI, testabile
// da CLI (src/test/test_image_engine.cpp).
class ImagePopulateTask {
public:
			PopulateResult		Run(const BString& imagePath,
									const BString& mountPoint,
									const std::vector<BString>& sourcePaths,
									const volatile bool* abortFlag = NULL);
};

#endif // IMAGE_POPULATE_TASK_H
