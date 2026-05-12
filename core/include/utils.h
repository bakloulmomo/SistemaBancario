#ifndef UTILS_H
#define UTILS_H

#include "tipi.h"

// ---- Generatori ----

// Genera un IBAN italiano nel formato IT60 + ABI + CAB + CC(12 cifre)
void genera_iban(int id_conto, char *iban_out);

// ---- JSON builder (output verso Node.js) ----

// Risposta successo: {"status":"ok","data":{...}}
void json_ok(const char *data_json, char *out, int outsize);

// Risposta errore: {"status":"error","message":"..."}
void json_errore(const char *messaggio, char *out, int outsize);

// ---- JSON parser (input da Node.js) ----

// Estrae valore stringa da JSON piatto: "chiave":"valore"
int json_get_str(const char *json, const char *chiave,
                 char *val_out, int val_size);

// Estrae valore numerico da JSON piatto: "chiave":123.45
int json_get_num(const char *json, const char *chiave, double *val_out);

// ---- Utilità stringhe ----

// Rimuove spazi iniziali/finali in-place
void str_trim(char *s);

// Ricerca case-insensitive di needle in haystack. Ritorna 1 se trovato.
int str_contains_ci(const char *haystack, const char *needle);

// Escape delle virgole nei campi CSV (sostituisce , con \\,)
void csv_escape(const char *in, char *out, int outsize);

// Unescape dei campi CSV
void csv_unescape(const char *in, char *out, int outsize);

#endif // UTILS_H
