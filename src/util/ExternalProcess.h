/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef EXTERNAL_PROCESS_H
#define EXTERNAL_PROCESS_H

#include <vector>

#include <String.h>
#include <SupportDefs.h>

#include <sys/types.h>

struct ExternalProcessResult {
	int		exitCode;	// -1 se il processo non e' partito
	BString	output;		// stdout+stderr combinati
};

// Callback invocata ad ogni blocco di output letto dal processo, prima che
// il processo termini: serve a BurnTask per leggere il progresso di
// cdrecord/xorriso mentre scrivono (Milestone 6), invece di aspettare la
// fine del processo come fa il resto del motore.
typedef void (*ExternalProcessOutputCallback)(const char* chunk,
	size_t length, void* cookie);

// Wrapper riusabile per processi esterni (mkfs, mount, unmount, cdrecord,
// xorriso, readcd, dd...). Nessuna shell intermedia: gli argomenti sono
// passati direttamente a execvp, quindi i path con spazi (come la cartella
// di progetto stessa) sono sicuri senza bisogno di escaping manuale.
class ExternalProcess {
public:
								ExternalProcess();

	// argv[0] e' il nome/percorso del programma da eseguire. Se callback
	// non e' NULL, viene invocata per ogni blocco di output non appena
	// disponibile (oltre ad essere comunque accumulato in
	// ExternalProcessResult::output).
			ExternalProcessResult	Run(const std::vector<BString>& argv,
									ExternalProcessOutputCallback callback = NULL,
									void* cookie = NULL);

	// Invia SIGTERM al processo in esecuzione (se presente). Da chiamare
	// da un altro thread rispetto a quello che ha invocato Run().
			void				Cancel();

private:
			vint32				fChildPid;
};

#endif // EXTERNAL_PROCESS_H
