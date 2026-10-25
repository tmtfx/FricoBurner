/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "FileListView.h"

#include <Catalog.h>
#include <Directory.h>
#include <Entry.h>
#include <Message.h>
#include <Node.h>
#include <View.h>
#include <Window.h>

#include <sys/stat.h>


FileEntryItem::FileEntryItem(const BPath& path, bool isDirectory)
	:
	BListItem(),
	fPath(path),
	fIsDirectory(isDirectory)
{
	fLabel = fPath.Path();
}


void
FileEntryItem::Update(BView* owner, const BFont* font)
{
	BListItem::Update(owner, font);
}


void
FileEntryItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	if (IsSelected() || complete) {
		rgb_color color = IsSelected() ? ui_color(B_LIST_SELECTED_BACKGROUND_COLOR)
			: owner->ViewColor();
		owner->SetHighColor(color);
		owner->FillRect(frame);
	}

	rgb_color textColor = IsSelected() ? ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR)
		: ui_color(B_LIST_ITEM_TEXT_COLOR);
	owner->SetHighColor(textColor);
	owner->SetLowColor(owner->ViewColor());

	font_height metrics;
	owner->GetFontHeight(&metrics);
	float baseline = frame.top + metrics.ascent
		+ (frame.Height() - (metrics.ascent + metrics.descent)) / 2;

	BString prefix = fIsDirectory ? "[dir] " : "[file] ";
	owner->DrawString(prefix, BPoint(frame.left + 4, baseline));
	float prefixWidth = owner->StringWidth(prefix);
	owner->DrawString(fLabel, BPoint(frame.left + 4 + prefixWidth, baseline));
}


FileListView::FileListView(const char* name, BMessenger changeTarget)
	:
	BListView(name, B_MULTIPLE_SELECTION_LIST),
	fChangeTarget(changeTarget)
{
}


void
FileListView::AddEntry(const BPath& path, bool isDirectory)
{
	AddItem(new FileEntryItem(path, isDirectory));
	_NotifyChanged();
}


void
FileListView::RemoveSelectedEntries()
{
	for (int32 i = CountItems() - 1; i >= 0; i--) {
		if (ItemAt(i)->IsSelected()) {
			delete RemoveItem(i);
		}
	}
	_NotifyChanged();
}


std::vector<BString>
FileListView::AllPaths() const
{
	std::vector<BString> paths;
	for (int32 i = 0; i < CountItems(); i++) {
		FileEntryItem* item = static_cast<FileEntryItem*>(ItemAt(i));
		paths.push_back(BString(item->Path().Path()));
	}
	return paths;
}


void
FileListView::_NotifyChanged()
{
	if (fChangeTarget.IsValid())
		fChangeTarget.SendMessage(MSG_FILE_LIST_CHANGED);
}


static uint64
RecursiveApparentSize(const BEntry& entry)
{
	uint64 size = 0;
	struct stat st;
	if (entry.GetStat(&st) != B_OK)
		return 0;

	if (S_ISDIR(st.st_mode)) {
		BDirectory dir(&entry);
		BEntry child;
		while (dir.GetNextEntry(&child) == B_OK)
			size += RecursiveApparentSize(child);
	} else {
		size += (uint64)st.st_size;
	}
	return size;
}


uint64
FileListView::TotalApparentSize() const
{
	uint64 total = 0;
	for (int32 i = 0; i < CountItems(); i++) {
		FileEntryItem* item = static_cast<FileEntryItem*>(ItemAt(i));
		BEntry entry(item->Path().Path());
		total += RecursiveApparentSize(entry);
	}
	return total;
}


void
FileListView::MessageReceived(BMessage* message)
{
	if (message->WasDropped() && message->what == B_SIMPLE_DATA) {
		entry_ref ref;
		int32 i = 0;
		bool addedAny = false;
		while (message->FindRef("refs", i++, &ref) == B_OK) {
			BEntry entry(&ref, true);
			if (!entry.Exists())
				continue;

			BPath path;
			entry.GetPath(&path);

			bool isDirectory = entry.IsDirectory();
			AddEntry(path, isDirectory);
			addedAny = true;
		}
		if (addedAny)
			Invalidate();
		return;
	}

	BListView::MessageReceived(message);
}
