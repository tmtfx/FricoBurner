/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "DiscCopyPanel.h"

#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuItem.h>
#include <PopUpMenu.h>


DiscCopyPanel::DiscCopyPanel(const char* name)
	:
	BView(name, 0)
{
	fSourceMenu = new BurnerDeviceMenu("source-device", "Disco sorgente:",
		"Nessun masterizzatore rilevato");

	BPopUpMenu* destModePopup = new BPopUpMenu("dest-mode-popup");

	BMessage* toImageMessage = new BMessage(MSG_DISC_COPY_DEST_MODE_CHANGED);
	toImageMessage->AddInt32("mode", DISC_COPY_TO_IMAGE);
	BMenuItem* toImageItem = new BMenuItem("Salva come immagine...",
		toImageMessage);
	destModePopup->AddItem(toImageItem);
	toImageItem->SetMarked(true);

	BMessage* toDiscMessage = new BMessage(MSG_DISC_COPY_DEST_MODE_CHANGED);
	toDiscMessage->AddInt32("mode", DISC_COPY_TO_DISC);
	destModePopup->AddItem(new BMenuItem(
		"Stesso masterizzatore (disco->disco)", toDiscMessage));

	BMenuField* destModeMenu = new BMenuField("dest-mode", "Destinazione:",
		destModePopup);
	fDestModeMenu = destModeMenu;

	fDestDeviceMenu = new BurnerDeviceMenu("dest-device",
		"Masterizzatore di destinazione:", "Nessun masterizzatore rilevato");

	fStartButton = new BButton("start-disc-copy", "Avvia copia disco",
		new BMessage(MSG_START_DISC_COPY_CLICKED));

	BLayoutBuilder::Group<>(this, B_VERTICAL, 8)
		.SetInsets(12, 12, 12, 12)
		.Add(fSourceMenu)
		.Add(fDestModeMenu)
		.Add(fDestDeviceMenu)
		.AddGlue()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fStartButton)
		.End();
}


void
DiscCopyPanel::SetDevices(const std::vector<OpticalDevice>& devices)
{
	fSourceMenu->SetDevices(devices);
	fDestDeviceMenu->SetDevices(devices);
}


DiscCopyDestMode
DiscCopyPanel::SelectedDestMode() const
{
	BMenuItem* marked = fDestModeMenu->Menu()->FindMarked();
	if (marked == NULL || marked->Message() == NULL)
		return DISC_COPY_TO_IMAGE;

	int32 mode = DISC_COPY_TO_IMAGE;
	marked->Message()->FindInt32("mode", &mode);
	return (DiscCopyDestMode)mode;
}
