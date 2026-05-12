#ifndef FILE_IO_H
#define FILE_IO_H

#include "tipi.h"

#define PATH_UTENTI       "data/utenti.csv"
#define PATH_CONTI        "data/conti.csv"
#define PATH_TRANSAZIONI  "data/transazioni.csv"
// Carica tutti i dati da CSV all'avvio.
// Ritorna 1 se ok, 0 se errore critico.
int carica_dati(StatoBanca *banca);

// Salva tutti i dati su CSV.
// Ritorna 1 se ok, 0 se errore scrittura.
int salva_dati(const StatoBanca *banca);

// Singoli salvataggi (per operazioni atomiche)
int salva_utenti(const StatoBanca *banca);
int salva_conti(const StatoBanca *banca);
int salva_transazioni(const StatoBanca *banca);

// Singoli caricamenti
int carica_utenti(StatoBanca *banca);
int carica_conti(StatoBanca *banca);
int carica_transazioni(StatoBanca *banca);

#endif // FILE_IO_H
