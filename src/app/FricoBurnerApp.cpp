/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "FricoBurnerApp.h"

#include "MainWindow.h"
#include "../util/TempImageRegistry.h"

static const char* kAppSignature = "application/x-vnd.Frico-FricoBurner";


FricoBurnerApp::FricoBurnerApp()
	:
	BApplication(kAppSignature)
{
	// Rimuove immagini/mountpoint orfani lasciati da una precedente istanza
	// terminata in modo anomalo, prima di aprire la finestra (Milestone 9).
	TempImageRegistry::CleanOrphans();

	fMainWindow = new MainWindow();
	fMainWindow->Show();
}
