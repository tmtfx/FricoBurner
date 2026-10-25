/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef SCAN_TASK_H
#define SCAN_TASK_H

#include <vector>

#include <Entry.h>
#include <String.h>
#include <SupportDefs.h>

class BNode;

// Risultato della scansione ricorsiva (Fase 2 SRS).
struct ScanResult {
	uint64	dataBytes;		// somma dimensione file (no directory, no attributi)
	uint64	attrBytes;		// somma dimensione di tutti gli attributi estesi letti
	uint32	fileCount;
	uint32	dirCount;
};

// Interfaccia di progresso disaccoppiata dalla UI: ScanTask non dipende da
// BMessage/BLooper, cosi' e' testabile da CLI (vedi src/test/test_scan.cpp)
// prima di essere agganciata al BurnEngine (Milestone 5).
class ScanProgressReporter {
public:
	virtual				~ScanProgressReporter() {}
	virtual	void		OnEntryScanned(const char* path, uint32 filesSoFar) = 0;
};

class ScanTask {
public:
								ScanTask(ScanProgressReporter* reporter = NULL);

			ScanResult			Scan(const std::vector<BString>& entryPaths);

private:
			void				_ScanEntry(const BEntry& entry, ScanResult& result);
			uint64				_AttributeBytes(BNode& node) const;

			ScanProgressReporter*	fReporter;
};

#endif // SCAN_TASK_H
