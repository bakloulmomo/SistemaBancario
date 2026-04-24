#ifndef TRANSAZIONI_H
#define TRANSAZIONI_H

#include "tipi.h"

/* Aggiunge nodo transazione in testa alla lista del conto. */
Transazione *transazione_aggiungi(StatoBanca *banca, Conto *conto,
                                   TipoTransazione tipo, double importo,
                                   const char *descrizione,
                                   const char *iban_controparte);

/* Deposita importo sul conto. Ritorna 1 ok / 0 errore. */
int deposita(StatoBanca *banca, const char *iban,
             double importo, const char *descrizione);

/* Preleva importo dal conto. Ritorna 1 ok / 0 se fondi insufficienti. */
int preleva(StatoBanca *banca, const char *iban,
            double importo, const char *descrizione);

/*
 * Bonifico tra due IBAN.
 * Ritorna: 1 ok / -1 mittente non trovato / -2 fondi insufficienti
 *          / -3 destinatario non trovato
 */
int bonifico(StatoBanca *banca,
             const char *iban_mittente,  const char *iban_destinatario,
             double importo, const char *descrizione);

/* Libera l'intera lista collegata di transazioni di un conto. */
void transazioni_libera(Transazione *testa);

/* Serializza lista transazioni di un conto come JSON array. */
void transazioni_to_json(const Transazione *testa, char *out, int outsize);

/* Serializza una singola transazione come JSON object. */
void transazione_to_json(const Transazione *t, char *out, int outsize);

/* Nomi stringa per TipoTransazione */
const char *tipo_transazione_str(TipoTransazione tipo);

#endif /* TRANSAZIONI_H */
