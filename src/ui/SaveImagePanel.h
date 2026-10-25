/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef SAVE_IMAGE_PANEL_H
#define SAVE_IMAGE_PANEL_H

#include <FilePanel.h>
#include <Messenger.h>

// Wrapper su BFilePanel (B_SAVE_PANEL) per il salvataggio dell'immagine
// .bfs temporanea in una destinazione scelta dall'utente (SRS 2.6).
// E' un dialogo nativo Haiku: funzionante gia' in questa milestone, non
// e' un mock (la logica di cosa fare col path scelto arriva in Milestone 7).
class SaveImagePanel {
public:
								SaveImagePanel(BMessenger notifyTarget);
	virtual						~SaveImagePanel();

			void				Show();

private:
			BFilePanel*			fFilePanel;
};

#endif // SAVE_IMAGE_PANEL_H
