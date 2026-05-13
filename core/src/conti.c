#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/conti.h"
#include "../include/utils.h"
#include "../include/transazioni.h"

#define CAP_INIZIALE 16

// Conti gestiti tramite array, dim modificabile

// Inizializza conto
void conti_init(StatoBanca *banca) {
    banca->conti     = (Conto *)malloc(CAP_INIZIALE * sizeof(Conto));
    banca->n_conti   = 0;
    banca->cap_conti = CAP_INIZIALE;
}

// libera: 1. liste di transazione di tutti i conti degli utenti. 2. libera array di conti
void conti_libera(StatoBanca *banca) {
    for (int i = 0; i < banca->n_conti; i++)
        transazioni_libera(banca->conti[i].transazioni);
    free(banca->conti);
    banca->conti     = NULL;
    banca->n_conti   = 0;
    banca->cap_conti = 0;
}

// crea conto dell'utente
int conto_apri(StatoBanca *banca, int id_utente) {
    // se numero di conti supera o = la memoria allocata per i conti, si rialloca
    if (banca->n_conti >= banca->cap_conti) {
        banca->cap_conti *= 2;
        banca->conti = (Conto *)realloc(banca->conti,
                           banca->cap_conti * sizeof(Conto));
        if (!banca->conti) return -1;
    }

    // creo conto
    Conto *c = &banca->conti[banca->n_conti];
    memset(c, 0, sizeof(Conto));

    c->id         = banca->prossimo_id_conto++;
    c->id_utente  = id_utente;
    c->saldo      = 0;
    c->attivo     = 1;
    c->transazioni = NULL;

    // genera IBAN per l'account (ogni account = 1 conto, quindi 1 IBAN)
    genera_iban(c->id, c->iban);

    banca->n_conti++;
    // torniamo id per le altre funzioni
    return c->id;
}

// cerca IBAN
Conto *conto_cerca_iban(StatoBanca *banca, const char *iban) {
    for (int i = 0; i < banca->n_conti; i++) {
        if (banca->conti[i].attivo &&
            strcmp(banca->conti[i].iban, iban) == 0)
            return &banca->conti[i];
    }
    return NULL;
}

// stesso ragionamento per transazioni, prima un conto alla volta, poi mandiamo tutto il buffer out
void conto_to_json(const Conto *c, char *out, int outsize) {
    snprintf(out, outsize,
        "{"
        "\"id\":%d,"
        "\"iban\":\"%s\","
        "\"id_utente\":%d,"
        "\"saldo\":%d"
        "}",
        c->id, c->iban, c->id_utente, c->saldo);
}

// stessa procedura 1. [ 2. aggiungiamo conto 1 alla volta 3. aggiungiamo virgole (first = 1) 4. ]
void conti_utente_to_json(StatoBanca *banca, int id_utente,
                           char *out, int outsize) {
    int written = 0;
    int first   = 1;

    written += snprintf(out + written, outsize - written, "[");

    for (int i = 0; i < banca->n_conti && written < outsize - 2; i++) {
        Conto *c = &banca->conti[i];
        if (c->id_utente != id_utente || !c->attivo) continue;

        if (!first) written += snprintf(out + written, outsize - written, ",");
        first = 0;

        char tmp[512];
        conto_to_json(c, tmp, sizeof(tmp));
        written += snprintf(out + written, outsize - written, "%s", tmp);
    }

    snprintf(out + written, outsize - written, "]");
}
