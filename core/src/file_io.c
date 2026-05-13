#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file_io.h"
#include "../include/utenti.h"
#include "../include/conti.h"
#include "../include/transazioni.h"
#include "../include/utils.h"

// Salva dati nel file in /data

// UTENTI: salva, carica
// CONTI: salva, carica
// TRANSAZIONI: salva, carica
// NOTIFICHE: carica, salva
// TUTTI I DATI: salva, carica

// CARICA = mette in RAM, tutto all'avvio
// utenti
int salva_utenti(const StatoBanca *banca) {
    FILE *f = fopen(PATH_UTENTI, "w");
    if (!f) return 0;

    // header
    fprintf(f, "id,username,password_hex,nome,cognome,attivo\n");

    // per ogni utente in banca,
    for (int i = 0; i < banca->n_utenti; i++) {
        const Utente *u = &banca->utenti[i];
        // solo per le stringhe scelte dall'utente, controlliamo ed aggiustamo le virgole
        char username_esc[64], nome_esc[128], cognome_esc[128];
        csv_escape(u->username, username_esc, sizeof(username_esc));
        csv_escape(u->nome,     nome_esc,     sizeof(nome_esc));
        csv_escape(u->cognome,  cognome_esc,  sizeof(cognome_esc));
        // usiamo infine stringa->esc

        // scriviamo infine tutto sul file CSV utenti
        fprintf(f, "%d,%s,%s,%s,%s,%d\n",
                u->id, username_esc, u->password_hex,
                nome_esc, cognome_esc, u->attivo);
    }

    fclose(f);
    return 1;
}

// per caricare gli studenti nell'array, all'inizio del programma
int carica_utenti(StatoBanca *banca) {
    FILE *f = fopen(PATH_UTENTI, "r");
    if (!f) return 0;
    // un utente alla volta, quando appena viene creato, viene slavato,
    // e poi caricato

    char riga[1024];
    fgets(riga, sizeof(riga), f); // legge prima riga (header)

    // si legge una riga alla volta
    while (fgets(riga, sizeof(riga), f)) {
        str_trim(riga); // toglie i \n a fine riga
        if (!riga[0]) continue; // dopo il trim, se la riga e vuota, si evita e continuamo con il loop

        // sempre se dobbiamo, riallochiamo
        if (banca->n_utenti >= banca->cap_utenti) {
            banca->cap_utenti *= 2;
            banca->utenti = (Utente *)realloc(banca->utenti,
                                banca->cap_utenti * sizeof(Utente));
        }

        // carica l'ultimo elemento dell'array
        Utente *u = &banca->utenti[banca->n_utenti];
        // pulisce lo spazio in memoria suo
        memset(u, 0, sizeof(Utente));

        char nome_esc[128], cognome_esc[128], username_esc[256];
        int attivo;

        // parsing manuale con strtok
        char *tok;
        char  buf[1024];
        // tolgo la prima riga, dell'header
        strncpy(buf, riga, sizeof(buf) - 1);

        // strtok = divido tutta la stringa CSV dell'utente con un ',' a parti
        tok = strtok(buf, ","); if (!tok) continue; u->id = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(username_esc, tok, 63);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(u->password_hex, tok, 95);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(nome_esc, tok, 127);
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(cognome_esc, tok, 127);
        tok = strtok(NULL, ","); if (!tok) continue; attivo = atoi(tok);

        // riporto le nuovo modifiche per le stringhe scelte dall'utente
        csv_unescape(nome_esc,    u->nome,    sizeof(u->nome));
        csv_unescape(cognome_esc, u->cognome, sizeof(u->cognome));
        csv_unescape(username_esc, u->username, sizeof(u->username));
        // lo ritorniamo attivo
        u->attivo = attivo;

        // incremento l'id per il prossimo utente, +1 da questo utente
        if (u->id >= banca->prossimo_id_utente)
            banca->prossimo_id_utente = u->id +1;

        banca->n_utenti++;
    }

    fclose(f);
    return 1;
}

// Conti
int salva_conti(const StatoBanca *banca) {
    FILE *f = fopen(PATH_CONTI, "w");
    if (!f) return 0;

    // header
    fprintf(f, "id,iban,id_utente,saldo,attivo\n");

    // per ogni utente, salviamo il suo conto
    for (int i = 0; i < banca->n_conti; i++) {
        const Conto *c = &banca->conti[i];
        fprintf(f, "%d,%s,%d,%d,%d\n",
                c->id, c->iban, c->id_utente, c->saldo, c->attivo);
    }

    fclose(f);
    return 1;
}

// carica conto, un utente alla volta
int carica_conti(StatoBanca *banca) {
    FILE *f = fopen(PATH_CONTI, "r");
    if (!f) return 1;
    // header
    char riga[512];
    fgets(riga, sizeof(riga), f);

    // per ogni riga del csv, la mandiamo in RAM
    while (fgets(riga, sizeof(riga), f)) {
        str_trim(riga);
        // se la riga e vuota, facciamo senza la riga
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
        // per il file csv, lo dividiamo uno ad uno, poi copiamo i parametri in c
        tok = strtok(buf, ","); if (!tok) continue; c->id = atoi(tok); // atoi per id
        tok = strtok(NULL, ","); if (!tok) continue; strncpy(c->iban, tok, 34);
        tok = strtok(NULL, ","); if (!tok) continue; c->id_utente = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; c->saldo = atoi(tok);
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

    fprintf(f, "id,id_conto,tipo,importo,iban_controparte\n");

    for (int i = 0; i < banca->n_conti; i++) {
        const Conto *c = &banca->conti[i];
        for (const Transazione *t = c->transazioni; t; t = t->next) {
            fprintf(f, "%d,%d,%d,%d,%s\n",
                    t->id, c->id, t->tipo, t->importo,
                    t->iban_controparte);
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
        int    importo;
        char   iban_cp[35];

        char buf[1024];
        strncpy(buf, riga, sizeof(buf) - 1);

        char *tok;
        tok = strtok(buf, ",");  if (!tok) continue; id = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; id_conto = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; tipo = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; importo = atoi(tok);
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
        strncpy(t->iban_controparte, iban_cp, 34);

        t->next = c->transazioni;
        c->transazioni = t;

        if (id >= banca->prossimo_id_transazione)
            banca->prossimo_id_transazione = id + 1;
    }

    fclose(f);
    return 1;
}

// carica tutti i dati
int carica_dati(StatoBanca *banca) {
    if (!carica_utenti(banca))      return 0;
    if (!carica_conti(banca))       return 0;
    if (!carica_transazioni(banca)) return 0;
    return 1;
}

int salva_dati(const StatoBanca *banca) {
    if (!salva_utenti(banca))      return 0;
    if (!salva_conti(banca))       return 0;
    if (!salva_transazioni(banca)) return 0;
    return 1;
}
