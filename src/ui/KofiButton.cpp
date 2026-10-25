/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#include "KofiButton.h"

#include <Cursor.h>
#include <Url.h>
#include <Window.h>


KofiButton::KofiButton(BBitmap* bitmap, const char* url)
	:
	BView("kofi_button", B_WILL_DRAW | B_FRAME_EVENTS),
	fBitmap(bitmap),
	fUrl(url)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}


KofiButton::~KofiButton()
{
	delete fBitmap;
}


void
KofiButton::GetPreferredSize(float* width, float* height)
{
	if (fBitmap != NULL) {
		BRect bounds = fBitmap->Bounds();
		if (width != NULL)
			*width = bounds.Width();
		if (height != NULL)
			*height = bounds.Height();
	} else {
		if (width != NULL)
			*width = 0;
		if (height != NULL)
			*height = 0;
	}
}


void
KofiButton::Draw(BRect updateRect)
{
	if (fBitmap != NULL) {
		SetDrawingMode(B_OP_ALPHA);
		SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		DrawBitmap(fBitmap, BPoint(0, 0));
		SetDrawingMode(B_OP_COPY);
	}
}


void
KofiButton::MouseDown(BPoint where)
{
	BUrl url(fUrl.String(), true);
	url.OpenWithPreferredApplication(true);
}


void
KofiButton::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	if (transit == B_ENTERED_VIEW) {
		BCursor linkCursor(B_CURSOR_ID_FOLLOW_LINK);
		SetViewCursor(&linkCursor);
	} else if (transit == B_EXITED_VIEW) {
		BCursor defaultCursor(B_CURSOR_ID_SYSTEM_DEFAULT);
		SetViewCursor(&defaultCursor);
	}

	BView::MouseMoved(where, transit, dragMessage);
}
