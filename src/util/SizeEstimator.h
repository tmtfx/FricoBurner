/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef SIZE_ESTIMATOR_H
#define SIZE_ESTIMATOR_H

#include <SupportDefs.h>

// Risultato della stima (SRS sezione 3).
struct SizeEstimate {
	uint64	dataBytes;			// dati + attributi estesi, somma grezza in ingresso
	uint64	totalBytes;			// dati + overhead BFS stimato
	bool	exceedsCapacity;	// true se totalBytes supera la capacita' del supporto
};

namespace SizeEstimator {

// Formula SRS sezione 3:
//   DimensioneTotale = max(DimDati * 1.08, DimDati + 16MB)
// capacityBytes = 0 significa "nessun limite da verificare" (es. preset
// Personalizzato non ancora impostato).
SizeEstimate Estimate(uint64 dataBytes, uint64 capacityBytes);

}

#endif // SIZE_ESTIMATOR_H
