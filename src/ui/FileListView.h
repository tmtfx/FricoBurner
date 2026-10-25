/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef FILE_LIST_VIEW_H
#define FILE_LIST_VIEW_H

#include <vector>

#include <ListView.h>
#include <Messenger.h>
#include <Path.h>
#include <String.h>

enum {
	MSG_FILE_LIST_CHANGED = 'bFLc'
};

// BStringItem custom che memorizza anche il path completo e se l'entry e'
// una directory (serve a ScanTask, Milestone 3). Qui in Milestone 2 serve
// solo a far vedere nella lista cosa e' stato trascinato.
class FileEntryItem : public BListItem {
public:
								FileEntryItem(const BPath& path, bool isDirectory);

	virtual	void				DrawItem(BView* owner, BRect frame, bool complete);
	virtual	void				Update(BView* owner, const BFont* font);

			const BPath&		Path() const { return fPath; }
			bool				IsDirectory() const { return fIsDirectory; }

private:
			BPath				fPath;
			bool				fIsDirectory;
			BString				fLabel;
};


// BListView che accetta il trascinamento di file/cartelle dalle finestre del
// Tracker (B_SIMPLE_DATA con "refs") e popola la lista con FileEntryItem.
class FileListView : public BListView {
public:
								FileListView(const char* name,
									BMessenger changeTarget);

	virtual	void				MessageReceived(BMessage* message);

			void				AddEntry(const BPath& path, bool isDirectory);
			void				RemoveSelectedEntries();
			uint64				TotalApparentSize() const;
			std::vector<BString>	AllPaths() const;

private:
			void				_NotifyChanged();

			BMessenger			fChangeTarget;
};

#endif // FILE_LIST_VIEW_H
