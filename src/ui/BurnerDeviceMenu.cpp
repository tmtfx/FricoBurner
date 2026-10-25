/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "BurnerDeviceMenu.h"

#include <Menu.h>
#include <MenuItem.h>
#include <PopUpMenu.h>


BurnerDeviceMenu::BurnerDeviceMenu(const char* name, const char* label,
	const char* emptyLabel)
	:
	BMenuField(name, label, new BPopUpMenu("burner-device-popup")),
	fHasDevices(false),
	fEmptyLabel(emptyLabel)
{
	SetDevices(std::vector<OpticalDevice>());
}


void
BurnerDeviceMenu::SetDevices(const std::vector<OpticalDevice>& devices)
{
	BMenu* menu = Menu();
	for (int32 i = menu->CountItems() - 1; i >= 0; i--)
		delete menu->RemoveItem(i);

	fHasDevices = !devices.empty();

	if (!fHasDevices) {
		BMenuItem* placeholder = new BMenuItem(fEmptyLabel.String(), NULL);
		placeholder->SetEnabled(false);
		menu->AddItem(placeholder);
		placeholder->SetMarked(true);
		return;
	}

	for (size_t i = 0; i < devices.size(); i++) {
		BString label;
		label.SetToFormat("%s %s (dev=%s)", devices[i].vendor.String(),
			devices[i].model.String(), devices[i].address.String());

		BMessage* itemMessage = new BMessage('bDva');
		itemMessage->AddString("address", devices[i].address);

		BMenuItem* item = new BMenuItem(label.String(), itemMessage);
		menu->AddItem(item);
		if (i == 0)
			item->SetMarked(true);
	}
}


BString
BurnerDeviceMenu::SelectedDeviceAddress() const
{
	if (!fHasDevices)
		return BString();

	BMenuItem* marked = Menu()->FindMarked();
	if (marked == NULL || marked->Message() == NULL)
		return BString();

	BString address;
	marked->Message()->FindString("address", &address);
	return address;
}
