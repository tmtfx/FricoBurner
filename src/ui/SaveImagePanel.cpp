/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "SaveImagePanel.h"


SaveImagePanel::SaveImagePanel(BMessenger notifyTarget)
{
	fFilePanel = new BFilePanel(B_SAVE_PANEL, &notifyTarget, NULL, 0,
		false, NULL, NULL, true, true);
	fFilePanel->SetSaveText("immagine.bfs");
}


SaveImagePanel::~SaveImagePanel()
{
	delete fFilePanel;
}


void
SaveImagePanel::Show()
{
	fFilePanel->Show();
}
