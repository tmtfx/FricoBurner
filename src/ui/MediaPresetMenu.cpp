/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "MediaPresetMenu.h"

#include <Menu.h>
#include <MenuItem.h>
#include <Message.h>
#include <Messenger.h>
#include <PopUpMenu.h>


MediaPresetMenu::MediaPresetMenu(const char* name, BMessenger notifyTarget)
	:
	BMenuField(name, "Supporto:", new BPopUpMenu("media-preset-popup")),
	fNotifyTarget(notifyTarget)
{
	BMenu* menu = Menu();
	for (int32 i = 0; i < MEDIA_PRESET_COUNT; i++) {
		BMessage* itemMessage = new BMessage(MSG_MEDIA_PRESET_CHANGED);
		itemMessage->AddInt32("kind", kMediaPresets[i].kind);

		BMenuItem* item = new BMenuItem(kMediaPresets[i].label, itemMessage);
		menu->AddItem(item);
		if (i == 0)
			item->SetMarked(true);
	}
	menu->SetTargetForItems(fNotifyTarget);
}


MediaPresetKind
MediaPresetMenu::SelectedKind() const
{
	BMenuItem* marked = Menu()->FindMarked();
	if (marked == NULL)
		return MEDIA_PRESET_CD;

	int32 kind = MEDIA_PRESET_CD;
	marked->Message()->FindInt32("kind", &kind);
	return (MediaPresetKind)kind;
}


uint64
MediaPresetMenu::SelectedCapacityBytes() const
{
	MediaPresetKind kind = SelectedKind();
	for (int32 i = 0; i < MEDIA_PRESET_COUNT; i++) {
		if (kMediaPresets[i].kind == kind)
			return kMediaPresets[i].capacityBytes;
	}
	return 0;
}
