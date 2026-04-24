#ifndef UTILS_H
#define UTILS_H

#include "tipi.h"

/* ---- Generatori ---- */

/* Genera un IBAN italiano nel formato IT60 + ABI + CAB + CC(12 cifre) */
void genera_iban(int id_conto, char *iban_out);

/* Genera un token di sessione casuale (64 hex char) */
void genera_token(char *token_out);

/* Data/ora corrente come stringa "YYYY-MM-DD HH:MM:SS" */
void data_ora_corrente(char *out);

/* Solo data corrente "YYYY-MM-DD" */
void data_corrente(char *out);

/* ---- Gestione sessioni ---- */

/* Aggiunge sessione, ritorna 1 ok / 0 se slot esauriti */
int sessione_crea(StatoBanca *banca, int id_utente, char *token_out);

/* Cerca sessione valida per token. Ritorna id_utente o -1 */
int sessione_verifica(StatoBanca *banca, const char *token);

/* Rimuove sessione per token */
void sessione_rimuovi(StatoBanca *banca, const char *token);

/* Rimuove sessioni scadute */
void sessioni_pulisci(StatoBanca *banca);

/* ---- Coda notifiche ---- */

void coda_enqueue(CodaNotifiche *coda, int id_utente, const char *msg);
NodoCoda *coda_dequeue(CodaNotifiche *coda);
void coda_libera(CodaNotifiche *coda);

/* ---- JSON builder (output verso Node.js) ---- */

/* Risposta successo: {"status":"ok","data":{...}} */
void json_ok(const char *data_json, char *out, int outsize);

/* Risposta errore: {"status":"error","message":"..."} */
void json_errore(const char *messaggio, char *out, int outsize);

/* ---- JSON parser (input da Node.js) ---- */

/* Estrae valore stringa da JSON piatto: "chiave":"valore" */
int json_get_str(const char *json, const char *chiave,
                 char *val_out, int val_size);

/* Estrae valore numerico da JSON piatto: "chiave":123.45 */
int json_get_num(const char *json, const char *chiave, double *val_out);

/* ---- Utilità stringhe ---- */

/* Rimuove spazi iniziali/finali in-place */
void str_trim(char *s);

/* Escape delle virgole nei campi CSV (sostituisce , con \\,) */
void csv_escape(const char *in, char *out, int outsize);

/* Unescape dei campi CSV */
void csv_unescape(const char *in, char *out, int outsize);

/* ---- Statistiche ---- */

/* Calcola statistiche e scrive JSON nel buffer */
void statistiche_json(StatoBanca *banca, int id_utente,
                      char *out, int outsize);

/* ---- Stampa barre ASCII (||||) proporzionali ---- */
void stampa_barre(double valore, double max_val, int larghezza, char *out);

#endif /* UTILS_H */
