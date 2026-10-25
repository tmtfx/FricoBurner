/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef TEMP_IMAGE_REGISTRY_H
#define TEMP_IMAGE_REGISTRY_H

// Pulizia di immagini/mountpoint orfani (SRS, piano §7 rischio "cleanup
// dopo crash"): se FricoBurner viene terminato in modo anomalo (crash,
// kill -9) mentre un'immagine e' montata, il file immagine e la cartella
// di mount restano nella cartella di lavoro condivisa. Dato che ogni
// sessione usa nomi univoci "image-<pid>.<ext>" / "mount-<pid>" (vedi
// BurnEngine::_PrepareWorkPaths), un'immagine e' riconoscibile come orfana
// semplicemente controllando se il pid nel nome corrisponde a un processo
// ancora vivo: non serve un registro separato su disco.
namespace TempImageRegistry {

// Da chiamare una volta all'avvio dell'applicazione (prima di aprire la
// finestra principale): smonta (se necessario) e cancella ogni immagine o
// mountpoint il cui processo proprietario non esiste piu'.
void CleanOrphans();

}

#endif // TEMP_IMAGE_REGISTRY_H
