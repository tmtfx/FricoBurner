/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H

#include <Bitmap.h>
#include <View.h>
#include <Window.h>

// Vista che disegna semplicemente un BBitmap centrato (la mascotte).
class MascotView : public BView {
public:
								MascotView(BBitmap* bitmap);
	virtual						~MascotView();

	virtual	void				Draw(BRect updateRect);
	virtual	void				GetPreferredSize(float* width, float* height);

private:
			BBitmap*			fBitmap;
};


// Finestra "About" con nome applicazione, versione e mascotte piratesca
// (SRS extra richiesto dall'utente).
class AboutWindow : public BWindow {
public:
								AboutWindow();
};

#endif // ABOUT_WINDOW_H
