/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

// Test CLI standalone per ImageProvisionTask + ImagePopulateTask
// (Milestone 4): crea un'immagine BFS vuota, la formatta, la monta, copia
// le sorgenti indicate preservando gli attributi estesi, smonta. Nessuna
// dipendenza da classi UI.
//
// Uso: test_image_engine <image-path> <size-bytes> <mountpoint> <source...>

#include <cstdio>
#include <cstdlib>
#include <vector>

#include <String.h>

#include "../engine/ImagePopulateTask.h"
#include "../engine/ImageProvisionTask.h"


int
main(int argc, char** argv)
{
	if (argc < 5) {
		fprintf(stderr,
			"uso: %s <image-path> <size-bytes> <mountpoint> <source...>\n",
			argv[0]);
		return 1;
	}

	BString imagePath(argv[1]);
	uint64 sizeBytes = strtoull(argv[2], NULL, 10);
	BString mountPoint(argv[3]);

	std::vector<BString> sources;
	for (int i = 4; i < argc; i++)
		sources.push_back(BString(argv[i]));

	printf("Fase 3: creazione e formattazione immagine '%s' (%llu byte)...\n",
		imagePath.String(), (unsigned long long)sizeBytes);

	ImageProvisionTask provisionTask;
	ProvisionResult provisionResult = provisionTask.Run(imagePath, sizeBytes,
		"FricoBurnerTest");
	if (provisionResult.error != B_OK) {
		fprintf(stderr, "ERRORE: %s\n", provisionResult.errorMessage.String());
		return 1;
	}
	printf("OK.\n\n");

	printf("Fase 4: mount, copia, unmount su '%s'...\n", mountPoint.String());

	ImagePopulateTask populateTask;
	PopulateResult populateResult = populateTask.Run(imagePath, mountPoint,
		sources);
	if (populateResult.error != B_OK) {
		fprintf(stderr, "ERRORE: %s\n", populateResult.errorMessage.String());
		return 1;
	}
	printf("OK.\n");

	return 0;
}
