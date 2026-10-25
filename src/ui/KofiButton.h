/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef KOFI_BUTTON_H
#define KOFI_BUTTON_H

#include <Bitmap.h>
#include <String.h>
#include <View.h>

// Pulsante "Buy me a coffee" di Ko-fi: disegna il bitmap passato e apre
// l'URL indicato (tramite BUrl) al click.
class KofiButton : public BView {
public:
								KofiButton(BBitmap* bitmap, const char* url);
	virtual						~KofiButton();

	virtual	void				Draw(BRect updateRect);
	virtual	void				GetPreferredSize(float* width, float* height);
	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);

private:
			BBitmap*			fBitmap;
			BString				fUrl;
};

#endif // KOFI_BUTTON_H
