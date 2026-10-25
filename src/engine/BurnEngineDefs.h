/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef BURN_ENGINE_DEFS_H
#define BURN_ENGINE_DEFS_H

// Comandi BMessage inviati dalla UI al BurnEngine. MSG_START_PROVISION,
// nonostante il nome, avvia l'intera sequenza scansione+immagine+copia
// (si ferma prima della masterizzazione: e' il comando di "Genera immagine
// standalone"); MSG_START_BURN fa la stessa sequenza e poi masterizza.
// Entrambi richiedono FIELD_SOURCE_PATH (uno o piu') e FIELD_CAPACITY_BYTES
// nel BMessage.
enum {
	MSG_START_SCAN			= 'bSSc',
	MSG_START_PROVISION		= 'bSPr',
	MSG_START_POPULATE		= 'bSPo',
	MSG_START_BURN			= 'bSBu',
	MSG_START_DISC_COPY		= 'bSDc',
	MSG_ABORT				= 'bAbt',

	// Messaggi di stato inviati dal BurnEngine alla UI.
	MSG_PHASE_PROGRESS		= 'bPPr',
	MSG_PHASE_COMPLETE		= 'bPCo',
	MSG_PHASE_ERROR			= 'bPEr'
};

// Fasi del workflow di masterizzazione (SRS sezione 2) + lettura disco sorgente
// per la copia disco (SRS titolo / scope esteso).
enum BurnPhase {
	PHASE_IDLE,
	PHASE_SCANNING,
	PHASE_PROVISIONING,
	PHASE_POPULATING,
	PHASE_BURNING,
	PHASE_READING_DISC,
	PHASE_DONE,
	PHASE_ERROR,
	PHASE_ABORTING
};

// Nomi dei campi BMessage: contratto stabile tra UI e motore. La UI (Milestone 2)
// e il motore reale (Milestone 5+) devono usare sempre questi stessi nomi/tipi,
// cosi passare dal motore mock a quello reale non richiede modifiche alla UI.
#define FIELD_PHASE				"phase"				// int32 (BurnPhase)
#define FIELD_PROGRESS_PERCENT		"progress_percent"	// float, 0.0 - 100.0
#define FIELD_MESSAGE				"message"			// string, stato leggibile
#define FIELD_SOURCE_PATH			"source_path"		// string, ripetuto per ogni entry
#define FIELD_CAPACITY_BYTES		"capacity_bytes"	// int64, capacita' supporto scelto
#define FIELD_IMAGE_PATH			"image_path"		// string, path immagine generata (in MSG_PHASE_COMPLETE o MSG_PHASE_ERROR se gia' creata)
#define FIELD_MOUNT_POINT			"mount_point"		// string, cartella di mount usata (per la pulizia alla chiusura)
#define FIELD_DEVICE_ADDRESS		"device_address"	// string "bus,id,lun", scelto dall'utente per MSG_START_BURN
#define FIELD_SOURCE_DEVICE_ADDRESS	"source_device_address"	// string "bus,id,lun", disco sorgente per MSG_START_DISC_COPY
#define FIELD_COPY_TO_DISC			"copy_to_disc"		// bool: true = disco->disco (usa anche FIELD_DEVICE_ADDRESS come destinazione), false = disco->immagine

#endif // BURN_ENGINE_DEFS_H
