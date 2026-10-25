/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "ScanTask.h"

#include <sys/stat.h>

#include <Directory.h>
#include <Node.h>
#include <Path.h>
#include <fs_attr.h>


ScanTask::ScanTask(ScanProgressReporter* reporter)
	:
	fReporter(reporter)
{
}


ScanResult
ScanTask::Scan(const std::vector<BString>& entryPaths)
{
	ScanResult result = { 0, 0, 0, 0 };

	for (size_t i = 0; i < entryPaths.size(); i++) {
		BEntry entry(entryPaths[i].String(), true);
		if (entry.Exists())
			_ScanEntry(entry, result);
	}

	return result;
}


uint64
ScanTask::_AttributeBytes(BNode& node) const
{
	uint64 total = 0;
	char attrName[B_ATTR_NAME_LENGTH];

	node.RewindAttrs();
	while (node.GetNextAttrName(attrName) == B_OK) {
		attr_info info;
		if (node.GetAttrInfo(attrName, &info) == B_OK)
			total += (uint64)info.size;
	}

	return total;
}


void
ScanTask::_ScanEntry(const BEntry& constEntry, ScanResult& result)
{
	BEntry entry(constEntry);

	struct stat st;
	if (entry.GetStat(&st) != B_OK)
		return;

	BNode node(&entry);
	if (node.InitCheck() == B_OK)
		result.attrBytes += _AttributeBytes(node);

	if (S_ISDIR(st.st_mode)) {
		result.dirCount++;

		BDirectory dir(&entry);
		BEntry child;
		while (dir.GetNextEntry(&child) == B_OK)
			_ScanEntry(child, result);
	} else {
		result.fileCount++;
		result.dataBytes += (uint64)st.st_size;

		if (fReporter != NULL) {
			BPath path;
			entry.GetPath(&path);
			fReporter->OnEntryScanned(path.Path(), result.fileCount);
		}
	}
}
