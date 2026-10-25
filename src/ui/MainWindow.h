/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Button.h>
#include <Window.h>

#include "../engine/BurnEngine.h"
#include "AboutWindow.h"
#include "BurnerDeviceMenu.h"
#include "CapacityBarView.h"
#include "DiscCopyPanel.h"
#include "FileListView.h"
#include "MediaPresetMenu.h"
#include "ProgressPanel.h"
#include "SaveImagePanel.h"

class BFilePanel;

enum {
	MSG_ADD_FILES_CLICKED		= 'bAdd',
	MSG_REMOVE_FILES_CLICKED	= 'bRem',
	MSG_GENERATE_IMAGE_CLICKED	= 'bGen',
	MSG_BURN_CLICKED			= 'bBrn',
	MSG_SHOW_ABOUT				= 'bAbo',
	MSG_FILES_SELECTED_FOR_ADD	= 'bFSA',
	MSG_ABORT_CLICKED			= 'bAbC',
	MSG_REFRESH_DEVICES_CLICKED	= 'bRfD',
	MSG_BACKEND_DETECTED		= 'bBkD',
	MSG_SAVE_IMAGE_CLICKED		= 'bSvI'
};

class MainWindow : public BWindow {
public:
								MainWindow();
	virtual						~MainWindow();

	virtual	void				MessageReceived(BMessage* message);
	virtual	bool				QuitRequested();

private:
			void				_UpdateCapacityBar();
			bool				_OperationInProgress() const;
			void				_RefreshBackendCapabilities();
			void				_CleanUpTempImage();

			FileListView*		fFileListView;
			MediaPresetMenu*	fMediaPresetMenu;
			BurnerDeviceMenu*	fDeviceMenu;
			CapacityBarView*	fCapacityBar;
			ProgressPanel*		fProgressPanel;
			DiscCopyPanel*		fDiscCopyPanel;

			BButton*			fAddButton;
			BButton*			fRemoveButton;
			BButton*			fGenerateImageButton;
			BButton*			fBurnButton;
			BButton*			fAbortButton;
			BButton*			fRefreshDevicesButton;
			BButton*			fSaveImageButton;

			BurnEngine*			fEngine;
			BFilePanel*			fAddFilePanel;
			SaveImagePanel*		fSaveImagePanel;
			bool				fOperationRunning;
			bool				fQuitAfterAbort;
			BString				fLastImagePath;
			BString				fLastMountPoint;
};

#endif // MAIN_WINDOW_H
