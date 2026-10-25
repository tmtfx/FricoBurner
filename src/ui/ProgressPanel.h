/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef PROGRESS_PANEL_H
#define PROGRESS_PANEL_H

#include <StatusBar.h>
#include <View.h>

#include "../engine/BurnEngineDefs.h"

// Pannello di stato dell'operazione in corso: etichetta fase + BStatusBar
// nativa per il progresso reale (0-100%). Riceve i messaggi
// MSG_PHASE_PROGRESS/COMPLETE/ERROR dal BurnEngine (o dal suo mock).
class ProgressPanel : public BView {
public:
								ProgressPanel(const char* name);

			void				HandleEngineMessage(BMessage* message);
			void				Reset();

			BStatusBar*			StatusBar() const { return fStatusBar; }

private:
			BStatusBar*			fStatusBar;
};

#endif // PROGRESS_PANEL_H
