/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

// Test CLI standalone per BackendCapabilities + BurnTask (Milestone 6).
// Senza argomenti: stampa solo le capacita' rilevate (tool disponibili,
// masterizzatori ottici trovati). Con argomenti <image> <device>: prova
// anche una masterizzazione in modalita' -dummy (nessun laser, utile per
// validare la catena di comando anche senza un masterizzatore reale, o per
// verificare la gestione dell'errore con un indirizzo device inesistente).
//
// Uso: test_backend [<image-path> <device-address>]

#include <cstdio>

#include "../engine/BackendCapabilities.h"
#include "../engine/BurnTask.h"


static void
PrintTool(const char* name, bool present)
{
	printf("  %-12s %s\n", name, present ? "trovato" : "ASSENTE");
}


class PrintingReporter : public ProcessLineReporter {
public:
	virtual void OnProcessLine(const char* line)
	{
		printf("    | %s\n", line);
	}
};


int
main(int argc, char** argv)
{
	printf("Rilevamento capacita' di sistema...\n\n");

	BackendStatus status = BackendCapabilities::Detect();

	printf("Tool CLI:\n");
	PrintTool("mkfs", status.hasMkfs);
	PrintTool("mount", status.hasMount);
	PrintTool("unmount", status.hasUnmount);
	PrintTool("cdrecord", status.hasCdrecord);
	PrintTool("xorriso", status.hasXorriso);
	PrintTool("growisofs", status.hasGrowisofs);
	PrintTool("readcd", status.hasReadcd);
	PrintTool("dd", status.hasDd);

	printf("\nBackend di masterizzazione preferito: ");
	switch (status.PreferredBurnBackend()) {
		case BACKEND_CDRECORD: printf("cdrecord\n"); break;
		case BACKEND_XORRISO: printf("xorriso (modalita' compatibilita')\n"); break;
		default: printf("NESSUNO\n"); break;
	}

	printf("\nMasterizzatori ottici rilevati: %zu\n",
		status.opticalDevices.size());
	for (size_t i = 0; i < status.opticalDevices.size(); i++) {
		const OpticalDevice& dev = status.opticalDevices[i];
		printf("  dev=%s  '%s' '%s'\n", dev.address.String(),
			dev.vendor.String(), dev.model.String());
	}
	if (status.opticalDevices.empty())
		printf("  (nessuno: masterizzazione reale non disponibile su questa macchina)\n");

	if (argc >= 3) {
		BString imagePath(argv[1]);
		BString deviceAddress(argv[2]);

		printf("\n--- Test BurnTask in modalita' -dummy ---\n");
		printf("immagine: %s\ndevice:   %s\n\n", imagePath.String(),
			deviceAddress.String());

		PrintingReporter reporter;
		BurnTask burnTask;
		BurnResult result = burnTask.Run(status.PreferredBurnBackend(),
			imagePath, deviceAddress, true, &reporter);

		if (result.error != B_OK) {
			fprintf(stderr, "\nERRORE (atteso se il device non esiste davvero): %s\n",
				result.errorMessage.String());
			return 1;
		}
		printf("\nOK.\n");
	}

	return 0;
}
