/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef EXTERNAL_PROCESS_LINE_READER_H
#define EXTERNAL_PROCESS_LINE_READER_H

#include <string>

#include "ExternalProcess.h"

// Interfaccia per ricevere l'output di un ExternalProcess riga per riga,
// anche quando il processo riscrive la stessa riga con '\r' (tipico di
// cdrecord/xorriso/readcd per mostrare il progresso in tempo reale).
// Riusata da BurnTask (Milestone 6) e DiscCopyTask (Milestone 8) per non
// duplicare la stessa logica di bufferizzazione.
class ProcessLineReporter {
public:
	virtual				~ProcessLineReporter() {}
	virtual	void		OnProcessLine(const char* line) = 0;
};

// Contesto da passare come "cookie" a ExternalProcess::Run() insieme a
// ExternalProcessLineCallback.
struct LineReaderContext {
	ProcessLineReporter*	reporter;
	std::string				buffer;
};

void ExternalProcessLineCallback(const char* chunk, size_t length, void* cookie);

#endif // EXTERNAL_PROCESS_LINE_READER_H
