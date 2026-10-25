/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef CAPACITY_BAR_VIEW_H
#define CAPACITY_BAR_VIEW_H

#include <View.h>

// Barra di riempimento rispetto alla capacita' del supporto target
// (SRS 4.1 "Barra dello Stato di Riempimento"). A differenza di una
// BStatusBar generica, qui il 100% rappresenta la capacita' fisica del
// preset scelto, non il progresso di un'operazione.
class CapacityBarView : public BView {
public:
								CapacityBarView(const char* name);

	virtual	void				Draw(BRect updateRect);
	virtual	void				GetPreferredSize(float* width, float* height);

			void				SetValues(uint64 usedBytes, uint64 capacityBytes);

private:
			uint64				fUsedBytes;
			uint64				fCapacityBytes;
};

#endif // CAPACITY_BAR_VIEW_H
