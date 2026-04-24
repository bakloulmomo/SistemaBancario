Introduzione generale ai progetti
Questi progetti hanno l'obiettivo di realizzare un sistema software completo, dalla progettazione iniziale fino al testing e alla documentazione finale. Il percorso è più importante del risultato: imparare a suddividere un problema complesso in moduli, a gestire la memoria manualmente e a lavorare in gruppo su una base di codice condivisa sono competenze che rimangono utili ben oltre il singolo progetto.
Seguite il cronoprogramma settimana per settimana. L'obiettivo non è bruciare le tappe per arrivare prima al codice funzionante, ma costruire ogni parte in modo solido prima di passare alla successiva. La prima settimana di progettazione — in cui si definiscono le struct, gli header e il repository — è la più importante: le decisioni prese lì condizionano tutto il resto.
Non è necessario completare ogni funzionalità prevista. Un progetto parziale ma ben compreso, ben documentato e con una relazione onesta vale più di un progetto apparentemente completo ma scritto senza cognizione di causa. Se per mancanza di tempo o difficoltà tecniche alcune funzionalità non vengono implementate, non è un problema in sé — a patto che nella relazione, nella sezione Risultati e obiettivi raggiunti, venga spiegato chiaramente cosa manca, perché non è stato completato e cosa sarebbe servito per farlo. La valutazione tiene conto del percorso, non solo del punto di arrivo.
Rimane fondamentale strutturare il codice in funzioni commentando ogni funzione ed ogni parte del codice che si ritiene non sia chiara alla prima lettura.
In sintesi
Usate gli strumenti a vostra disposizione — IA inclusa — ma restate sempre voi i responsabili di quello che consegnate. Capire è l'unica cosa che non si può delegare.

Gruppi da 3

ID
Progetto scelto
Team leader
Compagno 1
Compagno 2
Progetto originale scelto
1
 Progetto originale
Bakloul
Nair
El Maroufi
Progetto Originale
2
 DungeonC
Bernini
Cuoghi
Guerreschi
Compilare se non si scegliere una delle 3 proposte
3
 GestBib
Vittorio
Omose Osagiede
Cutroneo
Compilare se non si scegliere una delle 3 proposte
4
 Progetto originale
Daoudi
Garmouma
Raghi
Pacman
5
 DungeonC
Malaguti
Bellini M.
Nadalini
Compilare se non si scegliere una delle 3 proposte
6
 NightRes
Bellini A.
Sava
Orlando
Compilare se non si scegliere una delle 3 proposte
7
 DungeonC
Ogliani
Rivaroli
Luciano
Lonighi
Compilare se non si scegliere una delle 3 proposte
8
 DungeonC
Ouizzif
Glya
Bushaj
Compilare se non si scegliere una delle 3 proposte
9
 GestBib
Procentese
El Amrabti
Miotto
Compilare se non si scegliere una delle 3 proposte
10
 GestBib
Singh
Scarselli
Rossi
Compilare se non si scegliere una delle 3 proposte
11
 NightRes
Trotto
Rossanda
Prenga
Compilare se non si scegliere una delle 3 proposte




GestBib — Sistema di Gestione Biblioteca
Descrizione del progetto
GestBib è un gestionale da riga di comando scritto in C che simula il sistema informatico di una biblioteca pubblica. Permette di gestire il catalogo dei libri, gli utenti iscritti e i prestiti attivi, con persistenza dei dati su file CSV e notifiche per le scadenze.
Strutture dati utilizzate
Il progetto copre tutti i requisiti richiesti dal programma:
Lista collegata — ogni utente ha un campo *prestiti che punta alla testa di una lista dinamica dei suoi prestiti attivi. Ogni nodo della lista contiene un puntatore a struct Prestito e un puntatore *next al nodo successivo.
Coda (FIFO) — le notifiche di scadenza vengono accodate in una struttura CodaNotifiche. Ogni volta che il sistema rileva un prestito scaduto, inserisce una notifica in coda; il menu principale la estrae e la mostra all'utente all'avvio.
Array dinamico con malloc/realloc — il catalogo dei libri e l'elenco degli utenti sono array di puntatori a struct, ridimensionati dinamicamente.
Struct annidate — struct Prestito contiene un puntatore a struct Libro e uno a struct Utente.
Funzionalità da implementare
Il sistema deve offrire le seguenti funzioni, organizzate in un menu interattivo:
Gestione libri — aggiunta, modifica, eliminazione e ricerca per titolo, autore o genere. La ricerca deve scorrere la lista e usare strstr per la corrispondenza parziale.
Gestione utenti — registrazione, cancellazione, visualizzazione dello storico prestiti. Ogni utente ha un ID univoco generato automaticamente.
Gestione prestiti — creazione di un nuovo prestito (con controllo copie disponibili), restituzione, lista dei prestiti scaduti. La data di scadenza si calcola come data odierna + 30 giorni usando le funzioni <time.h>.
Statistiche — libro più prestato, utente con più prestiti, tasso di restituzione, generi più richiesti. Queste vengono calcolate scorrendo le strutture e stampate con una semplice visualizzazione testuale a barre (|||||).
Persistenza — al termine di ogni sessione, tutti i dati vengono salvati su quattro file CSV separati; all'avvio vengono ricaricati con parsing manuale riga per riga (usando fgets, sscanf e strtok).
Struttura dei file sorgente
gestbib/
├── include/
│   ├── libri.h
│   ├── utenti.h
│   ├── prestiti.h
│   ├── file_io.h
│   └── utils.h
├── src/
│   ├── main.c          ← menu e ciclo principale
│   ├── libri.c         ← Create, Read, Update, Delete (CRUD) del catalogo
│   ├── utenti.c        ← gestione anagrafica
│   ├── prestiti.c      ← logica prestiti e scadenze
│   ├── file_io.c       ← lettura/scrittura CSV
│   └── utils.c         ← ordinamento, statistiche, stampa
└── data/
    ├── libri.csv
    ├── utenti.csv
    ├── prestiti.csv
    └── storico.csv

Divisione dei ruoli
Studente A — Strutture dati e catalogo Si occupa di definire tutte le struct (nei file .h), implementare libri.c e utenti.c con le relative funzioni CRUD, e gestire l'allocazione/deallocazione della memoria. Scrive la sezione tecnica della relazione (scelte implementative, diagramma delle strutture).
Studente B — Logica prestiti e notifiche Implementa prestiti.c, incluse la coda delle notifiche, il calcolo delle date con <time.h> e la funzione che cerca i prestiti scaduti. Si occupa anche del testing sistematico con casi limite (utente senza prestiti, libro con zero copie, date al limite). Documenta i test nella relazione.
Studente C — I/O, interfaccia e statistiche Implementa file_io.c (parsing CSV, escape delle virgole nei campi) e utils.c (ordinamento con bubblesort o selection sort su array di puntatori, funzioni di stampa formattata, calcolo statistiche). Gestisce anche main.c. Redige la relazione finale e la presentazione.
Tutti e tre partecipano alla progettazione iniziale delle struct (da fare insieme come prima attività) e alla revisione finale del codice.
Struttura della relazione
La relazione deve essere un documento tecnico di 10–15 pagine, organizzata come segue:
1. Copertina — titolo, nomi, classe, anno scolastico, data di consegna.
2. Indice
3. Introduzione (1 pagina) — descrizione del problema affrontato, motivazione della scelta del progetto.
4. Analisi dei requisiti (1 pagine) — requisiti funzionali (cosa fa il sistema) e non funzionali (performance attese, limiti, gestione degli errori). Si può usare una tabella con ID requisito, descrizione e priorità (alta/media/bassa).
5. Progettazione (2–4 pagine) — diagramma delle strutture dati con rappresentazione grafica della lista collegata e della coda, schema dei file sorgente e delle dipendenze, descrizione degli algoritmi principali (ricerca, ordinamento, parsing CSV).
6. Implementazione (2–4 pagine) — frammenti di codice commentati per le parti più significative, con spiegazione delle scelte fatte. Non va incollato tutto il codice: si selezionano i passi più interessanti (es. la funzione di inserimento in lista, il parsing di una riga CSV, la restituzione di un libro).
7. Testing (massimo 1 pagina) — tabella dei casi di test con input, output atteso e output ottenuto. Include almeno: inserimento libro con dati mancanti, prestito con zero copie disponibili, restituzione di un prestito già restituito, caricamento file corrotto.
8. Risultati e obiettivi raggiunti (massimo 1 pagina) — elenco puntato degli obiettivi del progetto con indicazione se ciascuno è stato raggiunto, parzialmente raggiunto o non raggiunto, con spiegazione.
9. Divisione del lavoro (1 pagina) — tabella con le attività, il responsabile e le ore stimate. Include una breve riflessione personale di ciascuno studente (3–5 righe a testa).
10. Conclusioni (1 pagina) — difficoltà incontrate, cosa si è imparato, possibili estensioni future.
11. Appendici — link di Github con tutto il codice prodotto ed esempi di file CSV.

Cronoprogramma consigliato (5 settimane)
Settimana
Attività
1
Progettazione: definizione struct, header files, suddivisione compiti, impostazione repository
2
Implementazione moduli A e B: CRUD libri/utenti, scheletro prestiti
3
Completamento: file I/O, menu, statistiche, integrazione moduli
4
Testing, debug, gestione errori, pulizia del codice
5
Relazione, commenti al codice, presentazione finale

Estensioni facoltative 
Se il gruppo termina in anticipo, può aggiungere una o più delle seguenti funzionalità: ricerca con albero binario di ricerca (BST) ordinato per titolo; grafo delle co-letture (due utenti sono collegati se hanno preso in prestito lo stesso libro); esportazione di un report in HTML; interfaccia con colori ANSI nel terminale.

NightRes — Sistema di Prenotazione Tavoli per Discoteca
Descrizione del progetto
NightRes è un gestionale da riga di comando scritto in C che simula il sistema informatico di una discoteca. Permette di gestire i tavoli disponibili, i clienti registrati e le prenotazioni attive, con persistenza dei dati su file CSV e notifiche per i turni della serata.
Strutture dati utilizzate
Lista collegata — ogni cliente ha un campo *prenotazioni che punta alla testa di una lista dinamica delle sue prenotazioni passate e attive. Ogni nodo contiene un puntatore a struct Prenotazione e un puntatore *next al nodo successivo.
Coda (FIFO) — le richieste di tavoli in eccesso (overbooking / lista d'attesa) vengono accodate in una struttura CodaAttesa. Ogni volta che tutti i tavoli di una fascia sono occupati, il sistema accoda il cliente; all'inizio del turno estrae i clienti in coda e li notifica se si libera un posto.
Array dinamico con malloc/realloc — il catalogo dei tavoli e l'elenco dei clienti sono array di puntatori a struct, ridimensionati dinamicamente al caricamento e all'aggiunta di nuovi elementi.
Struct annidate — struct Prenotazione contiene un puntatore a struct Tavolo e uno a struct Cliente, oltre ai campi relativi a orario, caparra e stato.
Funzionalità da implementare
Il sistema deve offrire le seguenti funzioni, organizzate in un menu interattivo:
Gestione tavoli — aggiunta, modifica, eliminazione e ricerca per zona (VIP, dancefloor, lounge, esterno), capienza massima e prezzo minimo di consumazione. La ricerca scorre l'array e usa confronti numerici e strstr per le zone.
Gestione clienti — registrazione, cancellazione, visualizzazione dello storico prenotazioni. Ogni cliente ha un ID univoco generato automaticamente e un livello fedeltà (standard, gold, VIP).
Gestione prenotazioni — creazione di una nuova prenotazione con controllo disponibilità per fascia oraria (apertura, prime ore, late night), cancellazione con calcolo della penale, lista delle prenotazioni per la serata corrente. L'orario di scadenza della prenotazione (no-show) si calcola come orario di inizio turno + 30 minuti usando <time.h>.
Lista d'attesa — se un tavolo non è disponibile, il cliente viene inserito in coda FIFO; se un tavolo si libera (cancellazione), il primo in lista d'attesa riceve automaticamente la prenotazione.
Statistiche — tavolo più prenotato, cliente con più serate, tasso di no-show, zona più richiesta, incasso medio per serata. Visualizzate con barre testuali (|||||) proporzionali al valore massimo.
Persistenza — al termine di ogni sessione tutti i dati vengono salvati su quattro file CSV separati; all'avvio vengono ricaricati con parsing manuale riga per riga usando fgets, fscanf e strtok.
Struttura dei file sorgente
nightres/
├── include/
│   ├── tavoli.h
│   ├── clienti.h
│   ├── prenotazioni.h
│   ├── file_io.h
│   └── utils.h
├── src/
│   ├── main.c           ← menu e ciclo principale
│   ├── tavoli.c         ← CRUD del catalogo tavoli
│   ├── clienti.c        ← gestione anagrafica clienti
│   ├── prenotazioni.c   ← logica prenotazioni, attesa e no-show
│   ├── file_io.c        ← lettura/scrittura CSV
│   └── utils.c          ← ordinamento, statistiche, stampa
└── data/
    ├── tavoli.csv
    ├── clienti.csv
    ├── prenotazioni.csv
    └── storico.csv
Divisione dei ruoli
Studente A — Strutture dati e catalogo tavoli Definisce tutte le struct nei file .h, implementa tavoli.c e clienti.c con le relative funzioni CRUD, gestisce malloc/free per array dinamici e lista collegata. Redige la sezione tecnica della relazione (scelte implementative, diagramma delle strutture).
Studente B — Logica prenotazioni e lista d'attesa Implementa prenotazioni.c: coda FIFO per la lista d'attesa, calcolo degli slot orari con <time.h>, funzione di rilevamento no-show, gestione delle penali di cancellazione. Si occupa del testing sistematico con casi limite. Documenta i test nella relazione.
Studente C — I/O, interfaccia e statistiche Implementa file_io.c (parsing CSV, escape delle virgole nei campi testuali come nomi con virgola) e utils.c (ordinamento con bubblesort su array di puntatori, stampa formattata, calcolo statistiche). Gestisce main.c. Redige la relazione finale e la presentazione.
Tutti e tre partecipano alla progettazione iniziale delle struct e alla revisione finale del codice.
Struttura della relazione
La relazione deve essere un documento tecnico di 10–15 pagine, organizzata come segue:
Copertina — titolo, nomi, classe, anno scolastico, data di consegna.
Indice
Introduzione (1 pagina) — descrizione del problema, motivazione della scelta del progetto.
Analisi dei requisiti (1 pagina) — requisiti funzionali e non funzionali in tabella con ID, descrizione e priorità (alta/media/bassa).
Progettazione (2–4 pagine) — diagramma delle struct con lista collegata e coda, schema dei file sorgente e dipendenze, descrizione degli algoritmi principali (gestione slot orari, coda d'attesa, parsing CSV).
Implementazione (2–4 pagine) — frammenti di codice commentati per le parti più significative: inserimento in coda, gestione no-show con difftime, parsing di una riga CSV con campi facoltativi.
Testing (massimo 1 pagina) — tabella con input, output atteso e output ottenuto. Include almeno: prenotazione con tavolo pieno (→ lista d'attesa), cancellazione con penale al di sotto della soglia, no-show automatico allo scadere del tempo, caricamento di un CSV con riga malformata.
Risultati e obiettivi raggiunti (massimo 1 pagina) — elenco degli obiettivi con stato (raggiunto / parzialmente / non raggiunto) e spiegazione.
Divisione del lavoro (1 pagina) — tabella attività/responsabile/ore stimate e riflessione personale di ciascuno studente (3–5 righe).
Conclusioni (1 pagina) — difficoltà, apprendimenti, estensioni future.
Appendici — link GitHub con tutto il codice ed esempi di file CSV.
Cronoprogramma consigliato (5 settimane)
Settimana
Attività
1
Progettazione: definizione struct, header files, suddivisione compiti, impostazione repository
2
Implementazione moduli A e B: CRUD tavoli/clienti, scheletro prenotazioni e coda
3
Completamento: file I/O, menu, statistiche, integrazione moduli
4
Testing, debug, gestione errori, pulizia del codice
5
Relazione, commenti al codice, presentazione finale


Estensioni facoltative
Se il gruppo termina in anticipo, può aggiungere una o più delle seguenti funzionalità: ricerca dei tavoli disponibili tramite albero binario di ricerca (BST) ordinato per capienza; grafo delle co-prenotazioni (due clienti sono collegati se hanno condiviso la stessa serata); esportazione del foglio serata in HTML (lista tavoli con stato e orario); interfaccia con colori ANSI nel terminale per distinguere visivamente le zone della discoteca (VIP in giallo, dancefloor in rosso, lounge in blu).


DungeonC — Gioco testuale a stanze
Descrizione del progetto
DungeonC è un gioco di esplorazione testuale in cui il giocatore guida un eroe attraverso un dungeon composto da stanze collegate tra loro, raccoglie oggetti, combatte mostri e cerca di raggiungere il boss finale. Tutta la mappa è implementata come un grafo orientato in cui ogni nodo è una struct Stanza con quattro puntatori alle stanze adiacenti (nord, sud, est, ovest).
Meccaniche di gioco
Il giocatore interagisce tramite comandi testuali digitati da tastiera. Il parser legge la stringa con fgets e confronta le parole chiave con strcmp. I comandi base sono:
vai nord/sud/est/ovest — sposta l'eroe nella stanza adiacente, se esiste il collegamento. Se la stanza è bloccata (es. richiede una chiave), viene stampato un messaggio di avviso.
guarda — stampa la descrizione della stanza corrente, elenca gli oggetti presenti e segnala se c'è un mostro.
prendi [oggetto] — aggiunge l'oggetto in cima alla pila dell'inventario (push). Il numero massimo di oggetti trasportabili è un parametro costante (es. 8).
usa [oggetto] — applica l'effetto dell'oggetto in cima alla pila o cercato per nome: una pozione ripristina HP, una chiave sblocca una porta, una torcia rivela stanze nascoste.
attacca — avvia il combattimento a turni con il mostro nella stanza corrente. I danni vengono calcolati con rand() % attacco + 1 e modificati dalla difesa del bersaglio.
inventario — stampa la pila degli oggetti posseduti con i rispettivi effetti.
salva / carica — scrive/legge lo stato completo della partita su file binario.
mappa — mostra una rappresentazione ASCII delle stanze già visitate.
Strutture dati utilizzate
Grafo con puntatori — la mappa è costruita a runtime con malloc. Ogni struct Stanza contiene quattro puntatori *nord, *sud, *est, *ovest. Collegare due stanze significa assegnare i puntatori in entrambe le direzioni. Il grafo viene percorso con un puntatore *stanza_corrente nell'Eroe che si sposta ad ogni comando di movimento. Questo è il cuore strutturale del progetto.
Pila (stack) per l'inventario — la struct Pila ha un array di puntatori a struct Oggetto e un indice top. Le operazioni push e pop sono O(1). Quando l'inventario è pieno, push restituisce un errore senza aggiungere l'oggetto.
Lista collegata per gli oggetti nelle stanze — ogni stanza ha un campo *oggetti che punta alla testa di una lista. Quando il giocatore raccoglie un oggetto, il nodo viene rimosso dalla lista con gestione corretta dei puntatori (caso testa, caso nodo intermedio).
Enum per i tipi — TipoOggetto (POZIONE, ARMA, ARMATURA, CHIAVE, TORCIA) e TipoMostro (SCHELETRO, GOBLIN, DRAGO, BOSS) rendono il codice leggibile e permettono di usare lo switch per la logica specifica di ogni tipo.
File binario per il salvataggio — lo stato del gioco (HP, XP, oro, inventario, stanze visitate) viene serializzato con fwrite su un file .sav. Al caricamento, fread ricostruisce le strutture. 
File di testo per configurazioni — caratteristiche del mostro finale, nome e caratteristiche oggetti…
Struttura dei file sorgente
dungeon/
├── include/
│   ├── tipi.h          ← tutte le struct e gli enum
│   ├── mappa.h
│   ├── eroe.h
│   ├── combattimento.h
│   └── salvataggio.h
├── src/
│   ├── main.c          ← game loop e parser comandi
│   ├── mappa.c         ← costruzione grafo, stampa ASCII
│   ├── eroe.c          ← movimento, inventario (pila), livelli
│   ├── combattimento.c ← turni, dadi, morte, ricompense
│   └── salvataggio.c   ← fwrite/fread stato partita
└── save/
    └── partita.sav

Il game loop
Il ciclo principale in main.c potrebbe essere simile a questo:
while (eroe->hp > 0 && !partita_vinta) {
    stampa_stato(eroe);          // HP, stanza corrente
    printf("> ");
    fgets(input, MAX_INPUT, stdin);
    TipoComando cmd = parse(input, argomento);
    esegui_comando(cmd, argomento, eroe, &partita_vinta);
}

La funzione parse restituisce un valore di un enum TipoComando, e esegui_comando usa uno switch per chiamare la funzione corretta. 
Divisione dei ruoli
Studente A — Mappa e strutture Costruisce il grafo delle stanze in mappa.c: alloca le stanze con malloc, assegna i puntatori di collegamento, popola ogni stanza con oggetti (lista collegata) e mostri. Implementa la stampa ASCII della mappa delle stanze visitate. Definisce tutti i tipi in tipi.h. Scrive la sezione della relazione sulle strutture dati.
Studente B — Eroe e combattimento Implementa eroe.c con la pila dell'inventario (push/pop/stampa), il movimento tra stanze (aggiornamento *stanza_corrente), il sistema di XP e livelli (con soglie in un array di costanti), la gestione degli oggetti usabili. Implementa combattimento.c con i turni, il calcolo dei danni con rand(), la fuga e le ricompense. Bilancia i valori di HP e danni per rendere il gioco equilibrato. Documenta i test di gioco.
Studente C — Game loop, parser e salvataggio Scrive main.c con il game loop e la funzione parse. Implementa salvataggio.c con serializzazione/deserializzazione binaria dello stato. Redige la relazione finale e prepara la presentazione con una demo live del gioco.
Struttura della relazione
La relazione deve essere un documento tecnico di 10–15 pagine, organizzata come segue:
Copertina — titolo, nomi, classe, anno scolastico, data di consegna.
Indice
Introduzione (1 pagina) — descrizione del progetto, motivazione della scelta del tema videoludico e panoramica delle meccaniche implementate.
Analisi dei requisiti (1 pagina) — requisiti funzionali e non funzionali in tabella con ID, descrizione e priorità (alta/media/bassa). Esempi: gestione del movimento nel grafo (alta), sistema di salvataggio binario (alta), stampa ASCII della mappa (media), bilanciamento danni/HP (bassa).
Progettazione (2–4 pagine) — diagramma del grafo delle stanze con rappresentazione dei puntatori nord/sud/est/ovest, schema della pila dell'inventario con push/pop, schema della lista collegata degli oggetti per stanza, diagramma del game loop e del flusso dei comandi, descrizione degli algoritmi principali (parser, attraversamento del grafo, serializzazione binaria).
Implementazione (2–4 pagine) — frammenti di codice commentati per le parti più significative, con spiegazione delle scelte fatte. Non va incollato tutto il codice: si selezionano i passi più interessanti, come la funzione di collegamento bidirezionale tra stanze, il push/pop della pila inventario, la funzione parse con lo switch sui comandi, un turno di combattimento con rand(), la serializzazione con fwrite.
Testing (massimo 1 pagina) — tabella dei casi di test con input, output atteso e output ottenuto. Include almeno: movimento verso una stanza bloccata senza chiave, raccolta oggetto con inventario pieno (push fallito), uso di una pozione con HP già al massimo, salvataggio e caricamento con verifica dello stato ricostruito, combattimento contro il boss con esito morte dell'eroe.
Risultati e obiettivi raggiunti (massimo 1 pagina) — elenco degli obiettivi del progetto con indicazione se ciascuno è stato raggiunto, parzialmente raggiunto o non raggiunto, con spiegazione. Includere una valutazione soggettiva della giocabilità e del bilanciamento.
Divisione del lavoro (1 pagina) — tabella con le attività, il responsabile e le ore stimate. Include una breve riflessione personale di ciascuno studente (3–5 righe a testa) su cosa ha imparato e quali difficoltà ha incontrato.
Conclusioni (1 pagina) — difficoltà tecniche incontrate (in particolare nella gestione dei puntatori del grafo e nella serializzazione binaria), cosa si è imparato, possibili estensioni future come mappa generata proceduralmente, interfaccia con colori ANSI, o salvataggio in formato JSON testuale.
Appendici — link GitHub con tutto il codice prodotto, mappa del dungeon disegnata a mano o in ASCII, e un esempio di file .sav con spiegazione dei campi serializzati.
Cronoprogramma (5 settimane)
Settimana
Attività
1
Design: disegno della mappa, definizione struct in tipi.h, assegnazione ruoli
2
Implementazione mappa (grafo) + movimento eroe base + parser comandi
3
Inventario (pila), combattimento, oggetti usabili, sistema XP
4
Salvataggio, mappa ASCII, gestione errori, testing e bilanciamento
5
Pulizia codice, commenti, relazione, presentazione con demo


Estensioni facoltative
Per chi vuole approfondire: generazione procedurale del dungeon con un algoritmo ricorsivo (DFS con backtracking); sistema di quest con una coda di obiettivi; più finali alternativi in base alle scelte del giocatore; implementazione di un albero delle abilità dell'eroe (BST ordinato per livello richiesto).
