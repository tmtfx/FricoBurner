/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "AboutWindow.h"
#include "KofiButton.h"

#include <Application.h>
#include <Entry.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <Roster.h>
#include <StringView.h>
#include <TranslationUtils.h>


static const char* kKofiUrl = "https://ko-fi.com/U6U8R7FFN";


// Risolve il path di un'immagine in resources/ a partire dalla posizione
// dell'eseguibile: .../FricoBurner/bin/FricoBurner -> .../FricoBurner/
// resources/<name>. Funziona finche' l'app gira dalla cartella di sviluppo;
// andra' rivisto quando l'app verra' impacchettata (risorse .rdef, vedi
// struttura di progetto nel piano).
static BPath
ResourcePath(const char* name)
{
	app_info info;
	be_app->GetAppInfo(&info);

	BEntry entry(&info.ref);
	BPath exePath;
	entry.GetPath(&exePath);

	BPath binDir;
	exePath.GetParent(&binDir);

	BPath projectRoot;
	binDir.GetParent(&projectRoot);

	BPath resourcePath(projectRoot);
	resourcePath.Append("resources");
	resourcePath.Append(name);
	return resourcePath;
}


MascotView::MascotView(BBitmap* bitmap)
	:
	BView("mascot", B_WILL_DRAW),
	fBitmap(bitmap)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));	//SetViewColor(255, 255, 255);
}


MascotView::~MascotView()
{
	delete fBitmap;
}


void
MascotView::GetPreferredSize(float* width, float* height)
{
	if (fBitmap != NULL) {
		BRect bounds = fBitmap->Bounds();
		if (width != NULL)
			*width = bounds.Width();
		if (height != NULL)
			*height = bounds.Height();
	} else {
		if (width != NULL)
			*width = 120;
		if (height != NULL)
			*height = 90;
	}
}


void
MascotView::Draw(BRect updateRect)
{
	if (fBitmap != NULL) {
		// L'immagine ha un canale alpha; senza compositing alpha le aree
		// trasparenti verrebbero disegnate con il nero memorizzato nel PNG
		// invece dello sfondo bianco della view.
		SetDrawingMode(B_OP_ALPHA);
		SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		DrawBitmap(fBitmap, BPoint(0, 0));
		SetDrawingMode(B_OP_COPY);
	}
}


AboutWindow::AboutWindow()
	:
	BWindow(BRect(150, 150, 460, 380), "About FricoBurner",
		B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	BBitmap* mascotBitmap = BTranslationUtils::GetBitmap(
		ResourcePath("mascot_pirate.png").Path());

	MascotView* mascot = new MascotView(mascotBitmap);

	BStringView* title = new BStringView("title", "FricoBurner");
	title->SetFont(be_bold_font);

	BStringView* tagline = new BStringView("tagline",
		"Copia, incide e masterizza CD/DVD preservando gli attributi BFS");

	BStringView* version = new BStringView("version", "Versione 0.1 (sviluppo)");

	BBitmap* kofiBitmap = BTranslationUtils::GetBitmap(
		ResourcePath("kofi_button.webp").Path());
	KofiButton* kofiButton = new KofiButton(kofiBitmap, kKofiUrl);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 8)
		.SetInsets(16, 16, 16, 16)
		.AddGroup(B_HORIZONTAL, 8)
			.Add(mascot)
			.AddGroup(B_VERTICAL, 4)
				.Add(title)
				.Add(tagline)
				.Add(version)
				.AddGlue()
			.End()
		.End()
		.AddGroup(B_HORIZONTAL, 0)
			.Add(kofiButton)
			.AddGlue()
		.End()
		.AddGlue();
}
