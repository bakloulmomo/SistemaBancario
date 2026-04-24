#ifndef CONTI_H
#define CONTI_H

#include "tipi.h"

/* Inizializza array dinamico conti */
void conti_init(StatoBanca *banca);

/* Libera array conti e tutte le liste di transazioni */
void conti_libera(StatoBanca *banca);

/* Apre nuovo conto per utente. Ritorna id conto o -1. */
int conto_apri(StatoBanca *banca, int id_utente, TipoConto tipo);

/* Chiude conto (soft delete). Ritorna 1 ok / 0 se saldo > 0 o non trovato. */
int conto_chiudi(StatoBanca *banca, int id_utente, const char *iban);

/* Cerca conto per IBAN. Ritorna puntatore o NULL. */
Conto *conto_cerca_iban(StatoBanca *banca, const char *iban);

/* Serializza conto come JSON object (senza lista transazioni). */
void conto_to_json(const Conto *c, char *out, int outsize);

/* Serializza tutti i conti attivi di un utente come JSON array. */
void conti_utente_to_json(StatoBanca *banca, int id_utente,
                           char *out, int outsize);

#endif /* CONTI_H */
