/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef DISC_COPY_PANEL_H
#define DISC_COPY_PANEL_H

#include <Button.h>
#include <MenuField.h>
#include <View.h>

#include "BurnerDeviceMenu.h"

// Costanti di messaggio per i controlli del pannello.
enum {
	MSG_DISC_COPY_DEST_MODE_CHANGED	= 'bDCd',
	MSG_START_DISC_COPY_CLICKED		= 'bDCs'
};

enum DiscCopyDestMode {
	DISC_COPY_TO_IMAGE = 0,
	DISC_COPY_TO_DISC
};

// Tab "Copia Disco": selettore device sorgente + destinazione (device fisico
// per disco->disco, oppure "Salva come immagine" per disco->immagine).
// Milestone 8: sorgente e destinazione sono BurnerDeviceMenu reali,
// popolate da BackendCapabilities come nel tab "Masterizza File".
class DiscCopyPanel : public BView {
public:
								DiscCopyPanel(const char* name);

			void				SetDevices(
									const std::vector<OpticalDevice>& devices);

			BButton*			StartButton() const { return fStartButton; }
			BurnerDeviceMenu*	SourceMenu() const { return fSourceMenu; }
			DiscCopyDestMode	SelectedDestMode() const;
			BurnerDeviceMenu*	DestDeviceMenu() const { return fDestDeviceMenu; }

private:
			BurnerDeviceMenu*	fSourceMenu;
			BMenuField*			fDestModeMenu;
			BurnerDeviceMenu*	fDestDeviceMenu;
			BButton*			fStartButton;
};

#endif // DISC_COPY_PANEL_H
