/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "ProgressPanel.h"

#include <LayoutBuilder.h>
#include <Message.h>
#include <String.h>
#include <StringView.h>


ProgressPanel::ProgressPanel(const char* name)
	:
	BView(name, 0)
{
	// Label lasciata vuota apposta: il testo di stato arriva sempre via
	// Update()/SetText(), altrimenti BStatusBar concatena Label e Text
	// senza spazio (es. "Pronto.Operazione completata...").
	fStatusBar = new BStatusBar("progress");
	fStatusBar->SetMaxValue(100.0f);
	fStatusBar->SetText("Pronto.");

	BLayoutBuilder::Group<>(this, B_VERTICAL, 4)
		.SetInsets(8, 4, 8, 8)
		.Add(fStatusBar)
		.End();
}


void
ProgressPanel::Reset()
{
	fStatusBar->Reset();
	fStatusBar->SetText("Pronto.");
}


void
ProgressPanel::HandleEngineMessage(BMessage* message)
{
	BString text;
	message->FindString(FIELD_MESSAGE, &text);

	switch (message->what) {
		case MSG_PHASE_PROGRESS:
		{
			float percent = 0;
			message->FindFloat(FIELD_PROGRESS_PERCENT, &percent);
			float delta = percent - fStatusBar->CurrentValue();
			fStatusBar->Update(delta, text.String());
			break;
		}

		case MSG_PHASE_COMPLETE:
		{
			float delta = 100.0f - fStatusBar->CurrentValue();
			fStatusBar->Update(delta, text.String());
			break;
		}

		case MSG_PHASE_ERROR:
		{
			BString errorText("Errore: ");
			errorText << text;
			fStatusBar->Reset();
			fStatusBar->SetText(errorText.String());
			break;
		}

		default:
			break;
	}
}
