# Specifica di Architettura e Progetto Software: BFS DVD Burner per Haiku OS

## 1. Introduzione e Obiettivo del Progetto
Il progetto si propone di realizzare un'applicazione nativa per il sistema operativo **Haiku OS** in grado di copiare dischi, incidere immagini .iso e masterizzare file e directory mantenendo in maniera completamente trasparente gli **attributi estesi del Be File System (BFS)** (es. icone personalizzate, attributi di tipo dati, metadati Tracker/BeOS). 

Poiché i file system standard dei supporti ottici (ISO9660, UDF) non supportano nativamente la struttura degli attributi BFS senza estensioni proprietarie o complesse conversioni, l'applicazione sfrutterà un approccio basato su **immagini disco BFS trasparenti**.

---

## 2. Architettura Concettuale e Workflow masterizzazione file e directory

Anziché modificare la gestione del filesystem a basso livello, l'applicazione automatizza un flusso di lavoro a 5 fasi basato sulle funzionalità native del sistema:

```
[Selezione File/Dir] -> [Calcolo Dimensione + Overhead BFS] -> [Creazione/Formattazione Immagine .bfs]
                                                                        |
[Masterizzazione Disc] <- [Smontaggio Volume] <- [Copia File con Attributi]
```

### Flusso Logico Operativo:
1. **Analisi:** Selezione dei file e delle directory da includere nella masterizzazione.
2. **Dimensionamento Dinamico:** Scansione ricorsiva della struttura dei file, calcolo del peso logico dei dati e stima dell'overhead strutturale di BFS.
3. **Provisioning Immagine:** Creazione di un file contenitore vuoto con estensione `.bfs` e relativa formattazione BFS tramite lo strumento di sistema `mkbfs`.
4. **Popolamento e Trasparenza:** Montaggio temporaneo (`mount`) dell'immagine come volume BFS virtuale nel filesystem di Haiku e copia ricorsiva dei dati. Poiché la destinazione è un volume BFS attivo, la conservazione degli attributi estesi avviene in modo nativo e trasparente.
5. **Finalizzazione e Scrittura:** Smontaggio del volume virtuale (`unmount`) e passaggio dell'immagine binaria finale al backend di masterizzazione ottica (es. `growisofs` / `cdrecord`).
6. **Salvataggio (opzionale) immagine:** Al termine della masterizzazione sarà possibile salvare l'immagine bfs temporanea dove vuole l'utente (se lo desidera), in ogni caso l'immagine temporanea verrà eliminata alla chiusura della relativa finestra

---

## 3. Calcolo Dinamico della Dimensione dell'Immagine

Per evitare lo spreco di tempo di scrittura e spazio su disco, l'immagine verrà allocata dinamicamente sulla base della quantità effettivi di dati selezionati dall'utente.

### Algoritmo di Stima dello Spazio:

$$\text{Dimensione Finale} = \text{Dimensione Dati} + \text{Overhead BFS} + \text{Margine di Sicurezza}$$

* **Dimensione Dati ($\text{Dim}_{\text{data}}$):** Somma ricorsiva delle dimensioni in byte dei file, delle directory e dei loro attributi estesi.
* **Overhead BFS ($\text{Overhead}_{\text{bfs}}$):** Spazio riservato alla struttura interna di BFS:
  * Superblock e Bitmap delle allocazioni dei blocchi.
  * Inode per file e directory.
  * Alberi B+ (B-Plus Tree) per l'indicizzazione rapida della struttura e degli attributi.
  * Journaling logico di sistema (spazio minimo riservato al registro eventi delle transazioni).
* **Formula di Calcolo Suggerita:**
  $$\text{Dimensione Totale} = \max\Big((\text{Dim}_{\text{data}} \times 1.08), \text{Dim}_{\text{data}} + 16\text{ MB}\Big)$$
  *(In ogni caso la dimensione totale non può superare la capacità fisica del supporto ottico selezionato).*

---

## 4. Requisiti Funzionali

### 4.1 Interfaccia Utente (UI/UX)
* **Design Nativo:** Sviluppata rigorosamente tramite la libreria grafica nativa `BInterfaceKit` (`BWindow`, `BView`, `BListView`, ecc.) per integrarsi con il look-and-feel del Tracker e del Lookout di Haiku.
* **Drag-and-Drop:** Supporto al trascinamento diretto di file e cartelle dalle finestre del Tracker nell'interfaccia dell'applicazione.
* **Barra dello Stato di Riempimento:** Barra visiva dinamica con i limiti preimpostati dei supporti target (es. CD 700MB, DVD-R 4.7GB, DVD-DL 8.5GB, Personalizzato).
* **Generatore di Immagini Standalone:** Possibilità per l'utente di fermarsi al punto di creazione del file `.bfs` per esportarlo (senza obbligo di masterizzazione fisica).

### 4.2 Backend di Sistema
* Scrittura di file immagine binari tramite chiamate di sistema efficienti (`ftruncate` / allocazione diretta di blocchi).
* Esecuzione e gestione in background dei comandi di sistema per formattazione e montaggio (`mkbfs`, `mount`).
* Integrazione con lo stack di sottosistema di masterizzazione di Haiku per la gestione dei dispositivi ottici hardware (`/dev/disk/atapi/...` o utility `cdrecord`/`growisofs`).

---

## 5. Requisiti Tecnologici e Dipendenze

* **Linguaggio di Programmazione:** C++ (Standard nativo per lo sviluppo su Haiku).
* **API Frameworks:**
  * `Storage Kit` (`BDirectory`, `BEntry`, `BPath`, `BNode`, `BNodeInfo` per l'ispezione ed il calcolo degli attributi estesi).
  * `Application Kit` e `Interface Kit` (`BApplication`, `BWindow`, `BView`, `BMessage`).
* **Strumenti CLI del Sistema Operativo:**
  * `mkbfs` per la formattazione dei volumi virtuali.
  * `mount` / `unmount` per l'aggancio del file immagine nel filesystem di Haiku.
  * `growisofs` o backend equivalenza per la masterizzazione dei file `.bfs` raw su supporto DVD/CD.

---

## 6. Prospettive ed Evoluzioni Future
* **Verifica Integrità:** Modulo per la ri-lettura e verifica del checksum SHA256/MD5 dell'immagine masterizzata rispetto al contenuto sorgente.