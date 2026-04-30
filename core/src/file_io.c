#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/file_io.h"
#include "../include/utenti.h"
#include "../include/conti.h"
#include "../include/transazioni.h"
#include "../include/utils.h"

// utenti 

int salva_utenti(const StatoBanca *banca) {
    FILE *f = fopen(PATH_UTENTI, "w");
    if (!f) return 0;

    fprintf(f, "id,username,password_hex,nome,cognome,email,telefono,"
               "data_nascita,data_registrazione,attivo\n");

    for (int i = 0; i < banca->n_utenti; i++) {
        const Utente *u = &banca->utenti[i];
        char nome_esc[128], cognome_esc[128], email_esc[256];
        csv_escape(u->nome,    nome_esc,    sizeof(nome_esc));
        csv_escape(u->cognome, cognome_esc, sizeof(cognome_esc));
        csv_escape(u->email,   email_esc,   sizeof(email_esc));

        fprintf(f, "%d,%s,%s,%s,%s,%s,%s,%s,%s,%d\n",
                u->id, u->username, u->password_hex,
                nome_esc, cognome_esc, email_esc,
                u->telefono, u->data_nascita,
                u->data_registrazione, u->attivo);
    }

    fclose(f);
    return 1;
}

int carica_utenti(StatoBanca *banca) {
    FILE *f = fopen(PATH_UTENTI, "r");
    if (!f) return 1; 

    char riga[1024];
    fgets(riga, sizeof(riga), f); 

    while (fgets(riga, sizeof(riga), f)) {
        str_trim(riga);
        if (!riga[0]) continue;

        if (banca->n_utenti >= banca->cap_utenti) {
            banca->cap_utenti *= 2;
            banca->utenti = (Utente *)realloc(banca->utenti,
                                banca->cap_utenti * sizeof(Utente));
        }

        Utente *u = &banca->utenti[banca->n_utenti];
        memset(u, 0, sizeof(Utente));

        char nome_esc[128], cognome_esc[128], email_esc[256];
        int attivo;

        // parsing manuale con strtok 
        char *tok;
        char  buf[1024];
        strncpy(buf, riga, sizeof(buf) - 1);

        tok = strtok(buf, ","); if (!tok) continue; u->id = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(u->username, tok, 63);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(u->password_hex, tok, 95);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(nome_esc, tok, 127);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(cognome_esc, tok, 127);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(email_esc, tok, 255);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(u->telefono, tok, 19);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(u->data_nascita, tok, 11);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(u->data_registrazione, tok, 19);
        tok = strtok(NULL, ","); if (!tok) continue; attivo = atoi(tok);

        csv_unescape(nome_esc,    u->nome,    sizeof(u->nome));
        csv_unescape(cognome_esc, u->cognome, sizeof(u->cognome));
        csv_unescape(email_esc,   u->email,   sizeof(u->email));
        u->attivo = attivo;

        if (u->id >= banca->prossimo_id_utente)
            banca->prossimo_id_utente = u->id + 1;

        banca->n_utenti++;
    }

    fclose(f);
    return 1;
}

// Conti/

int salva_conti(const StatoBanca *banca) {
    FILE *f = fopen(PATH_CONTI, "w");
    if (!f) return 0;

    fprintf(f, "id,iban,id_utente,saldo,tipo,data_apertura,attivo\n");

    for (int i = 0; i < banca->n_conti; i++) {
        const Conto *c = &banca->conti[i];
        fprintf(f, "%d,%s,%d,%.2f,%d,%s,%d\n",
                c->id, c->iban, c->id_utente, c->saldo,
                c->tipo, c->data_apertura, c->attivo);
    }

    fclose(f);
    return 1;
}

int carica_conti(StatoBanca *banca) {
    FILE *f = fopen(PATH_CONTI, "r");
    if (!f) return 1;

    char riga[512];
    fgets(riga, sizeof(riga), f);

    while (fgets(riga, sizeof(riga), f)) {
        str_trim(riga);
        if (!riga[0]) continue;

        if (banca->n_conti >= banca->cap_conti) {
            banca->cap_conti *= 2;
            banca->conti = (Conto *)realloc(banca->conti,
                               banca->cap_conti * sizeof(Conto));
        }

        Conto *c = &banca->conti[banca->n_conti];
        memset(c, 0, sizeof(Conto));
        c->transazioni = NULL;

        char buf[512];
        strncpy(buf, riga, sizeof(buf) - 1);

        char *tok;
        tok = strtok(buf, ","); if (!tok) continue; c->id = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(c->iban, tok, 34);
        tok = strtok(NULL, ","); if (!tok) continue; c->id_utente = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; c->saldo = atof(tok);
        tok = strtok(NULL, ","); if (!tok) continue; c->tipo = (TipoConto)atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(c->data_apertura, tok, 19);
        tok = strtok(NULL, ","); if (!tok) continue; c->attivo = atoi(tok);

        if (c->id >= banca->prossimo_id_conto)
            banca->prossimo_id_conto = c->id + 1;

        banca->n_conti++;
    }

    fclose(f);
    return 1;
}

// transazioni

int salva_transazioni(const StatoBanca *banca) {
    FILE *f = fopen(PATH_TRANSAZIONI, "w");
    if (!f) return 0;

    fprintf(f, "id,id_conto,tipo,importo,descrizione,data,iban_controparte\n");

    for (int i = 0; i < banca->n_conti; i++) {
        const Conto *c = &banca->conti[i];
        for (const Transazione *t = c->transazioni; t; t = t->next) {
            char desc_esc[256];
            csv_escape(t->descrizione, desc_esc, sizeof(desc_esc));

            fprintf(f, "%d,%d,%d,%.2f,%s,%s,%s\n",
                    t->id, c->id, t->tipo, t->importo,
                    desc_esc, t->data, t->iban_controparte);
        }
    }

    fclose(f);
    return 1;
}

int carica_transazioni(StatoBanca *banca) {
    FILE *f = fopen(PATH_TRANSAZIONI, "r");
    if (!f) return 1;

    char riga[1024];
    fgets(riga, sizeof(riga), f);

    while (fgets(riga, sizeof(riga), f)) {
        str_trim(riga);
        if (!riga[0]) continue;

        int    id, id_conto, tipo;
        double importo;
        char   desc_esc[256], data[20], iban_cp[35];

        char buf[1024];
        strncpy(buf, riga, sizeof(buf) - 1);

        char *tok;
        tok = strtok(buf, ",");  if (!tok) continue; id = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; id_conto = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; tipo = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; importo = atof(tok);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(desc_esc, tok, 255);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(data, tok, 19);
        tok = strtok(NULL, ","); iban_cp[0] = '\0';
        if (tok) strncpy(iban_cp, tok, 34);


        Conto *c = NULL;
        for (int i = 0; i < banca->n_conti; i++) {
            if (banca->conti[i].id == id_conto) {
                c = &banca->conti[i];
                break;
            }
        }
        if (!c) continue;

        Transazione *t = (Transazione *)malloc(sizeof(Transazione));
        if (!t) continue;
        memset(t, 0, sizeof(Transazione));

        t->id     = id;
        t->tipo   = (TipoTransazione)tipo;
        t->importo = importo;
        csv_unescape(desc_esc, t->descrizione, sizeof(t->descrizione));
        strncpy(t->data, data, 19);
        strncpy(t->iban_controparte, iban_cp, 34);

        t->next = c->transazioni;
        c->transazioni = t;

        if (id >= banca->prossimo_id_transazione)
            banca->prossimo_id_transazione = id + 1;
    }

    fclose(f);
    return 1;
}

// sessioni

int salva_sessioni(const StatoBanca *banca) {
    FILE *f = fopen(PATH_SESSIONI, "w");
    if (!f) return 0;

    fprintf(f, "token,id_utente,scadenza\n");

    long adesso = (long)time(NULL);
    for (int i = 0; i < banca->n_sessioni; i++) {
        const Sessione *s = &banca->sessioni[i];
        if (s->scadenza > adesso) /* salva solo non scadute */
            fprintf(f, "%s,%d,%ld\n", s->token, s->id_utente, s->scadenza);
    }

    fclose(f);
    return 1;
}

int carica_sessioni(StatoBanca *banca) {
    FILE *f = fopen(PATH_SESSIONI, "r");
    if (!f) return 1;

    char riga[256];
    fgets(riga, sizeof(riga), f); 

    long adesso = (long)time(NULL);

    while (fgets(riga, sizeof(riga), f)) {
        str_trim(riga);
        if (!riga[0]) continue;
        if (banca->n_sessioni >= MAX_SESSIONI) break;

        Sessione *s = &banca->sessioni[banca->n_sessioni];
        memset(s, 0, sizeof(Sessione));

        char buf[256];
        strncpy(buf, riga, sizeof(buf) - 1);

        char *tok;
        tok = strtok(buf, ","); if (!tok) continue; strncpy(s->token, tok, 64);
        tok = strtok(NULL, ","); if (!tok) continue; s->id_utente = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; s->scadenza = atol(tok);

        if (s->scadenza > adesso) 
            banca->n_sessioni++;
    }

    fclose(f);
    return 1;
}

// notifiche 

int salva_notifiche(const StatoBanca *banca) {
    FILE *f = fopen(PATH_NOTIFICHE, "w");
    if (!f) return 0;

    fprintf(f, "id_utente,messaggio,timestamp\n");

    for (int i = 0; i < banca->n_notifiche; i++) {
        const Notifica *n = &banca->notifiche_arr[i];
        char msg_esc[512];
        csv_escape(n->messaggio, msg_esc, sizeof(msg_esc));
        fprintf(f, "%d,%s,%s\n", n->id_utente, msg_esc, n->timestamp);
    }

    fclose(f);
    return 1;
}

int carica_notifiche(StatoBanca *banca) {
    FILE *f = fopen(PATH_NOTIFICHE, "r");
    if (!f) return 1;

    char riga[512];
    fgets(riga, sizeof(riga), f);

    while (fgets(riga, sizeof(riga), f)) {
        str_trim(riga);
        if (!riga[0]) continue;

        if (banca->n_notifiche >= banca->cap_notifiche) {
            banca->cap_notifiche = banca->cap_notifiche ? banca->cap_notifiche * 2 : 64;
            banca->notifiche_arr = (Notifica *)realloc(banca->notifiche_arr,
                                       banca->cap_notifiche * sizeof(Notifica));
        }

        Notifica *n = &banca->notifiche_arr[banca->n_notifiche];
        memset(n, 0, sizeof(Notifica));

        char buf[512];
        strncpy(buf, riga, sizeof(buf) - 1);

        char *tok;
        tok = strtok(buf, ","); if (!tok) continue; n->id_utente = atoi(tok);
        char msg_esc[512] = {0};
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(msg_esc, tok, 511);
        tok = strtok(NULL, ","); if (tok) strncpy(n->timestamp, tok, 19);

        csv_unescape(msg_esc, n->messaggio, sizeof(n->messaggio));
        banca->n_notifiche++;
    }

    fclose(f);
    return 1;
}

// funzioni

int carica_dati(StatoBanca *banca) {
    if (!carica_utenti(banca))      return 0;
    if (!carica_conti(banca))       return 0;
    if (!carica_transazioni(banca)) return 0;
    carica_sessioni(banca);
    carica_notifiche(banca);
    return 1;
}

int salva_dati(const StatoBanca *banca) {
    if (!salva_utenti(banca))      return 0;
    if (!salva_conti(banca))       return 0;
    if (!salva_transazioni(banca)) return 0;
    salva_sessioni(banca);
    salva_notifiche(banca);
    return 1;
}
