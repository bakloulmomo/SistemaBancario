#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "../include/utils.h"
#include "../include/tipi.h"

/* ---- Generatori ---- */

void genera_iban(int id_conto, char *iban_out) {
    /* Formato: IT60 + ABI(5) + CAB(5) + CC(12 cifre zero-padded) */
    snprintf(iban_out, 35, "IT60%s%s%012d", BANCA_ABI, BANCA_CAB, id_conto);
}

void genera_token(char *token_out) {
    /* Token da 64 char hex (256 bit di entropia pseudocasuale) */
    for (int i = 0; i < 64; i++) {
        int r = rand() % 16;
        token_out[i] = "0123456789abcdef"[r];
    }
    token_out[64] = '\0';
}

void data_ora_corrente(char *out) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(out, 20, "%Y-%m-%d %H:%M:%S", tm_info);
}

void data_corrente(char *out) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(out, 12, "%Y-%m-%d", tm_info);
}

/* ---- Gestione sessioni ---- */

int sessione_crea(StatoBanca *banca, int id_utente, char *token_out) {
    /* Rimuovi sessioni scadute prima di aggiungere */
    sessioni_pulisci(banca);

    if (banca->n_sessioni >= MAX_SESSIONI)
        return 0;

    genera_token(token_out);

    Sessione *s = &banca->sessioni[banca->n_sessioni++];
    strncpy(s->token, token_out, 64);
    s->token[64] = '\0';
    s->id_utente = id_utente;
    s->scadenza  = (long)time(NULL) + SESSION_DURATA_SEC;

    return 1;
}

int sessione_verifica(StatoBanca *banca, const char *token) {
    long adesso = (long)time(NULL);
    for (int i = 0; i < banca->n_sessioni; i++) {
        Sessione *s = &banca->sessioni[i];
        if (strcmp(s->token, token) == 0) {
            if (s->scadenza < adesso) {
                /* Sessione scaduta: rimuovi e ritorna errore */
                sessione_rimuovi(banca, token);
                return -1;
            }
            /* Rinnova scadenza ad ogni uso */
            s->scadenza = adesso + SESSION_DURATA_SEC;
            return s->id_utente;
        }
    }
    return -1;
}

void sessione_rimuovi(StatoBanca *banca, const char *token) {
    for (int i = 0; i < banca->n_sessioni; i++) {
        if (strcmp(banca->sessioni[i].token, token) == 0) {
            /* Sposta l'ultimo al posto di questo (swap & shrink) */
            banca->sessioni[i] = banca->sessioni[--banca->n_sessioni];
            return;
        }
    }
}

void sessioni_pulisci(StatoBanca *banca) {
    long adesso = (long)time(NULL);
    for (int i = banca->n_sessioni - 1; i >= 0; i--) {
        if (banca->sessioni[i].scadenza < adesso) {
            banca->sessioni[i] = banca->sessioni[--banca->n_sessioni];
        }
    }
}

/* ---- Coda notifiche (FIFO) ---- */

void coda_enqueue(CodaNotifiche *coda, int id_utente, const char *msg) {
    NodoCoda *nodo = (NodoCoda *)malloc(sizeof(NodoCoda));
    if (!nodo) return;

    strncpy(nodo->messaggio, msg, 255);
    nodo->messaggio[255] = '\0';
    nodo->id_utente = id_utente;
    nodo->next = NULL;

    if (coda->coda)
        coda->coda->next = nodo;
    else
        coda->testa = nodo;

    coda->coda = nodo;
    coda->n++;
}

NodoCoda *coda_dequeue(CodaNotifiche *coda) {
    if (!coda->testa) return NULL;

    NodoCoda *nodo = coda->testa;
    coda->testa = nodo->next;
    if (!coda->testa) coda->coda = NULL;
    coda->n--;
    return nodo;
}

void coda_libera(CodaNotifiche *coda) {
    NodoCoda *curr = coda->testa;
    while (curr) {
        NodoCoda *next = curr->next;
        free(curr);
        curr = next;
    }
    coda->testa = coda->coda = NULL;
    coda->n = 0;
}

/* ---- JSON builder ---- */

void json_ok(const char *data_json, char *out, int outsize) {
    if (data_json && data_json[0] != '\0')
        snprintf(out, outsize, "{\"status\":\"ok\",\"data\":%s}", data_json);
    else
        snprintf(out, outsize, "{\"status\":\"ok\"}");
}

void json_errore(const char *messaggio, char *out, int outsize) {
    snprintf(out, outsize, "{\"status\":\"error\",\"message\":\"%s\"}", messaggio);
}

/* ---- JSON parser (input da Node.js) ---- */

/*
 * Cerca "chiave":"valore" nel JSON piatto.
 * Gestisce solo valori stringa (con virgolette).
 * Ritorna 1 se trovato, 0 altrimenti.
 */
int json_get_str(const char *json, const char *chiave,
                 char *val_out, int val_size) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", chiave);

    const char *pos = strstr(json, pattern);
    if (!pos) return 0;

    /* Avanza oltre il nome chiave */
    pos += strlen(pattern);

    /* Cerca ':' */
    while (*pos && *pos != ':') pos++;
    if (!*pos) return 0;
    pos++; /* salta ':' */

    /* Salta spazi */
    while (*pos && isspace((unsigned char)*pos)) pos++;

    /* Verifica che il valore sia una stringa */
    if (*pos != '"') return 0;
    pos++; /* salta '"' di apertura */

    int i = 0;
    while (*pos && *pos != '"' && i < val_size - 1) {
        /* Gestione escape JSON base */
        if (*pos == '\\' && *(pos + 1)) {
            pos++;
            switch (*pos) {
                case 'n': val_out[i++] = '\n'; break;
                case 't': val_out[i++] = '\t'; break;
                case '"': val_out[i++] = '"';  break;
                case '\\':val_out[i++] = '\\'; break;
                default:  val_out[i++] = *pos; break;
            }
        } else {
            val_out[i++] = *pos;
        }
        pos++;
    }
    val_out[i] = '\0';
    return 1;
}

/*
 * Cerca "chiave":numero nel JSON piatto.
 * Ritorna 1 se trovato, 0 altrimenti.
 */
int json_get_num(const char *json, const char *chiave, double *val_out) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", chiave);

    const char *pos = strstr(json, pattern);
    if (!pos) return 0;

    pos += strlen(pattern);

    while (*pos && *pos != ':') pos++;
    if (!*pos) return 0;
    pos++;

    while (*pos && isspace((unsigned char)*pos)) pos++;

    if (*pos == '"') return 0; /* è una stringa, non numero */

    return sscanf(pos, "%lf", val_out) == 1 ? 1 : 0;
}

/* ---- Utilità stringhe ---- */

void str_trim(char *s) {
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
    char *start = s;
    while (*start && isspace((unsigned char)*start))
        start++;
    if (start != s)
        memmove(s, start, strlen(start) + 1);
}

void csv_escape(const char *in, char *out, int outsize) {
    int j = 0;
    for (int i = 0; in[i] && j < outsize - 2; i++) {
        if (in[i] == ',') {
            out[j++] = '\\';
            out[j++] = ',';
        } else if (in[i] == '\n') {
            out[j++] = '\\';
            out[j++] = 'n';
        } else {
            out[j++] = in[i];
        }
    }
    out[j] = '\0';
}

void csv_unescape(const char *in, char *out, int outsize) {
    int j = 0;
    for (int i = 0; in[i] && j < outsize - 1; i++) {
        if (in[i] == '\\' && in[i + 1] == ',') {
            out[j++] = ',';
            i++;
        } else if (in[i] == '\\' && in[i + 1] == 'n') {
            out[j++] = '\n';
            i++;
        } else {
            out[j++] = in[i];
        }
    }
    out[j] = '\0';
}

/* ---- Ricerca case-insensitive ---- */

int str_contains_ci(const char *haystack, const char *needle) {
    if (!needle || !needle[0]) return 1;
    int nlen = (int)strlen(needle);
    for (int i = 0; haystack[i]; i++) {
        int j;
        for (j = 0; j < nlen && haystack[i+j]; j++) {
            if (tolower((unsigned char)haystack[i+j]) != tolower((unsigned char)needle[j]))
                break;
        }
        if (j == nlen) return 1;
    }
    return 0;
}

/* Genera username da nome+cognome: "abcd_wxyz_NNN" (prime 4 lettere, 3 cifre random) */
void genera_username(const char *nome, const char *cognome, int suffisso, char *out, int outsize) {
    char p1[5] = {0}, p2[5] = {0};
    int i;
    for (i = 0; i < 4 && nome[i]; i++)
        p1[i] = (char)tolower((unsigned char)nome[i]);
    for (i = 0; i < 4 && cognome[i]; i++)
        p2[i] = (char)tolower((unsigned char)cognome[i]);
    snprintf(out, outsize, "%s_%s_%03d", p1, p2, suffisso % 1000);
}

/* Rimuove tutte le sessioni di un dato utente */
void sessioni_rimuovi_utente(StatoBanca *banca, int id_utente) {
    for (int i = banca->n_sessioni - 1; i >= 0; i--) {
        if (banca->sessioni[i].id_utente == id_utente)
            banca->sessioni[i] = banca->sessioni[--banca->n_sessioni];
    }
}


void statistiche_json(StatoBanca *banca, int id_utente,
                      char *out, int outsize) {
    double totale_depositi  = 0.0;
    double totale_prelievi  = 0.0;
    double totale_bonif_out = 0.0;
    double totale_bonif_in  = 0.0;
    double saldo_totale     = 0.0;
    int    n_conti_attivi   = 0;
    int    n_transazioni    = 0;

    for (int i = 0; i < banca->n_conti; i++) {
        Conto *c = &banca->conti[i];
        if (c->id_utente != id_utente || !c->attivo) continue;

        n_conti_attivi++;
        saldo_totale += c->saldo;

        for (Transazione *t = c->transazioni; t; t = t->next) {
            n_transazioni++;
            switch (t->tipo) {
                case TX_DEPOSITO:     totale_depositi  += t->importo; break;
                case TX_PRELIEVO:     totale_prelievi  += t->importo; break;
                case TX_BONIFICO_OUT: totale_bonif_out += t->importo; break;
                case TX_BONIFICO_IN:  totale_bonif_in  += t->importo; break;
            }
        }
    }

    snprintf(out, outsize,
        "{"
        "\"n_conti\":%d,"
        "\"saldo_totale\":%.2f,"
        "\"n_transazioni\":%d,"
        "\"totale_depositi\":%.2f,"
        "\"totale_prelievi\":%.2f,"
        "\"totale_bonifici_uscita\":%.2f,"
        "\"totale_bonifici_entrata\":%.2f"
        "}",
        n_conti_attivi, saldo_totale, n_transazioni,
        totale_depositi, totale_prelievi,
        totale_bonif_out, totale_bonif_in);
}
