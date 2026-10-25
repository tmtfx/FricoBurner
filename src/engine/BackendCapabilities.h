/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef BACKEND_CAPABILITIES_H
#define BACKEND_CAPABILITIES_H

#include <vector>

#include <String.h>
#include <SupportDefs.h>

// Un masterizzatore ottico rilevato tramite "cdrecord -scanbus".
struct OpticalDevice {
	BString	address;	// "bus,id,lun", da passare a cdrecord/xorriso come dev=
	BString	vendor;
	BString	model;
};

enum BurnBackend {
	BACKEND_NONE,		// nessun tool di masterizzazione disponibile
	BACKEND_CDRECORD,
	BACKEND_XORRISO
};

// Stato del toolchain di sistema rilevato a runtime (vedi piano, rischio
// "divergenza toolchain tra installazioni Haiku"): i nomi dei binari non
// vanno mai assunti presenti, si verificano qui una volta sola.
struct BackendStatus {
	bool	hasMkfs;
	bool	hasMount;
	bool	hasUnmount;
	bool	hasCdrecord;
	bool	hasXorriso;
	bool	hasGrowisofs;
	bool	hasReadcd;
	bool	hasDd;

	std::vector<OpticalDevice>	opticalDevices;

	BurnBackend	PreferredBurnBackend() const
	{
		if (hasCdrecord)
			return BACKEND_CDRECORD;
		if (hasXorriso)
			return BACKEND_XORRISO;
		return BACKEND_NONE;
	}
};

// Rilevamento capacita' di sistema (Milestone 6). Nessuna dipendenza UI,
// testabile da CLI (src/test/test_backend.cpp).
namespace BackendCapabilities {

BackendStatus Detect();

}

#endif // BACKEND_CAPABILITIES_H
