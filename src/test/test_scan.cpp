/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

// Test CLI standalone per ScanTask + SizeEstimator (Milestone 3). Non
// dipende da nessuna classe UI: puo' girare da terminale per validare il
// calcolo di dimensione prima di agganciarlo al BurnEngine (Milestone 5).
//
// Uso: test_scan <path> [path...]

#include <cstdio>
#include <vector>

#include <String.h>

#include "../engine/ScanTask.h"
#include "../util/MediaPreset.h"
#include "../util/SizeEstimator.h"


int
main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "uso: %s <path> [path...]\n", argv[0]);
		return 1;
	}

	std::vector<BString> paths;
	for (int i = 1; i < argc; i++)
		paths.push_back(BString(argv[i]));

	ScanTask task;
	ScanResult result = task.Scan(paths);

	printf("File scansionati:       %u\n", result.fileCount);
	printf("Directory scansionate:  %u\n", result.dirCount);
	printf("Dati (solo file):       %llu byte (%.2f MB)\n",
		(unsigned long long)result.dataBytes,
		result.dataBytes / (1024.0 * 1024.0));
	printf("Attributi estesi:       %llu byte (%.2f MB)\n",
		(unsigned long long)result.attrBytes,
		result.attrBytes / (1024.0 * 1024.0));

	uint64 combined = result.dataBytes + result.attrBytes;
	printf("Totale dati+attributi:  %llu byte (%.2f MB)\n\n",
		(unsigned long long)combined, combined / (1024.0 * 1024.0));

	printf("Stima immagine BFS (DimDati * 1.08, DimDati + 16MB, il maggiore):\n");
	for (int32 i = 0; i < MEDIA_PRESET_COUNT; i++) {
		if (kMediaPresets[i].kind == MEDIA_PRESET_CUSTOM)
			continue;

		SizeEstimate est = SizeEstimator::Estimate(combined,
			kMediaPresets[i].capacityBytes);

		printf("  %-20s -> %10.2f MB%s\n", kMediaPresets[i].label,
			est.totalBytes / (1024.0 * 1024.0),
			est.exceedsCapacity ? "   [ECCEDE LA CAPACITA' DEL SUPPORTO]" : "");
	}

	return 0;
}
