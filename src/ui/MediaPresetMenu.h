/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef MEDIA_PRESET_MENU_H
#define MEDIA_PRESET_MENU_H

#include <MenuField.h>
#include <Messenger.h>

#include "../util/MediaPreset.h"

// Costante di messaggio inviata quando l'utente cambia preset.
enum {
	MSG_MEDIA_PRESET_CHANGED = 'bMPc'
};

// BMenuField con i preset di capacita' supporto (SRS 4.1): CD 700MB,
// DVD-R 4.7GB, DVD-DL 8.5GB, Personalizzato.
class MediaPresetMenu : public BMenuField {
public:
								MediaPresetMenu(const char* name,
									BMessenger notifyTarget);

			MediaPresetKind		SelectedKind() const;
			uint64				SelectedCapacityBytes() const;

private:
			BMessenger			fNotifyTarget;
};

#endif // MEDIA_PRESET_MENU_H
