/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef BURNER_DEVICE_MENU_H
#define BURNER_DEVICE_MENU_H

#include <vector>

#include <MenuField.h>
#include <String.h>

#include "../engine/BackendCapabilities.h"

// BMenuField popolato con i masterizzatori ottici rilevati davvero sul
// sistema (BackendCapabilities::Detect(), Milestone 6), cosi' l'utente
// puo' scegliere quale usare se ce ne sono piu' di uno, invece di
// affidarsi silenziosamente al primo trovato. Riusata sia per il
// masterizzatore nel tab "Masterizza File" sia per sorgente/destinazione
// nel tab "Copia Disco" (Milestone 8), con etichette diverse.
class BurnerDeviceMenu : public BMenuField {
public:
								BurnerDeviceMenu(const char* name,
									const char* label = "Masterizzatore:",
									const char* emptyLabel =
										"Nessun masterizzatore rilevato");

			void				SetDevices(
									const std::vector<OpticalDevice>& devices);
			bool				HasDevices() const { return fHasDevices; }
			BString				SelectedDeviceAddress() const;

private:
			bool				fHasDevices;
			BString				fEmptyLabel;
};

#endif // BURNER_DEVICE_MENU_H
