/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef BURN_ENGINE_H
#define BURN_ENGINE_H

#include <vector>

#include <Looper.h>
#include <Messenger.h>
#include <OS.h>
#include <String.h>

#include "BurnEngineDefs.h"

class ExternalProcess;

// Orchestratore a stati delle fasi 2-6 dell'SRS (piu' la lettura disco per la
// copia disco). Gira sul proprio thread (BLooper) e, per ogni comando
// MSG_START_*, spawna un thread worker separato cosi' il Looper resta
// reattivo a MSG_ABORT.
//
// Milestone 5: le fasi 2-4 (scansione, provisioning, popolamento) usano i
// task reali (ScanTask/ImageProvisionTask/ImagePopulateTask). Milestone 6:
// anche la fase di masterizzazione (5) usa BurnTask/BackendCapabilities
// reali (sceglie il masterizzatore scelto in UI; se nessuno e' presente,
// errore chiaro invece di un finto successo). Milestone 8: la copia disco
// usa DiscCopyTask reale (readcd) per la lettura, e riusa _RunBurnPhase per
// l'eventuale scrittura disco->disco. In nessun caso la UI e' stata toccata
// per questi passaggi: usa sempre lo stesso contratto BMessage definito in
// BurnEngineDefs.h.
class BurnEngine : public BLooper {
public:
								BurnEngine();
	virtual						~BurnEngine();

	virtual	void				MessageReceived(BMessage* message);

			void				SetProgressTarget(const BMessenger& target);

private:
			void				_StartWorker(int32 command,
									const std::vector<BString>& sourcePaths,
									uint64 capacityBytes,
									const BString& deviceAddress,
									const BString& sourceDeviceAddress,
									bool copyToDisc);
			void				_SendProgress(BurnPhase phase, float percent,
									const char* message);
			void				_SendComplete(BurnPhase finalPhase,
									const char* message,
									const BString& imagePath = BString());
			void				_SendError(const char* message);

	static	int32				_WorkerEntry(void* data);
			void				_RunWorker(int32 command,
									const std::vector<BString>& sourcePaths,
									uint64 capacityBytes,
									const BString& deviceAddress,
									const BString& sourceDeviceAddress,
									bool copyToDisc);
			void				_RunDiscCopy(const BString& sourceDeviceAddress,
									bool copyToDisc,
									const BString& destDeviceAddress);
			bool				_RunBurnPhase(const BString& imagePath,
									const BString& deviceAddress);
			bool				_PrepareWorkPaths(BString& imagePath,
									BString& mountPoint,
									const char* extension = "bfs");

			BMessenger			fProgressTarget;
			BurnPhase			fPhase;
			volatile bool		fAbortRequested;
			thread_id			fWorkerThread;
			ExternalProcess*	fActiveProcess;
			BString				fCurrentImagePath;
			BString				fCurrentMountPoint;

			class BurnReporter;
			friend class BurnReporter;
};

#endif // BURN_ENGINE_H
