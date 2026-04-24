#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/conti.h"
#include "../include/utils.h"
#include "../include/transazioni.h"

#define CAP_INIZIALE 16

void conti_init(StatoBanca *banca) {
    banca->conti     = (Conto *)malloc(CAP_INIZIALE * sizeof(Conto));
    banca->n_conti   = 0;
    banca->cap_conti = CAP_INIZIALE;
}

void conti_libera(StatoBanca *banca) {
    for (int i = 0; i < banca->n_conti; i++)
        transazioni_libera(banca->conti[i].transazioni);
    free(banca->conti);
    banca->conti     = NULL;
    banca->n_conti   = 0;
    banca->cap_conti = 0;
}

int conto_apri(StatoBanca *banca, int id_utente, TipoConto tipo) {
    if (banca->n_conti >= banca->cap_conti) {
        banca->cap_conti *= 2;
        banca->conti = (Conto *)realloc(banca->conti,
                           banca->cap_conti * sizeof(Conto));
        if (!banca->conti) return -1;
    }

    Conto *c = &banca->conti[banca->n_conti];
    memset(c, 0, sizeof(Conto));

    c->id         = banca->prossimo_id_conto++;
    c->id_utente  = id_utente;
    c->saldo      = 0.0;
    c->tipo       = tipo;
    c->attivo     = 1;
    c->transazioni = NULL;

    genera_iban(c->id, c->iban);
    data_ora_corrente(c->data_apertura);

    banca->n_conti++;
    return c->id;
}

int conto_chiudi(StatoBanca *banca, int id_utente, const char *iban) {
    Conto *c = conto_cerca_iban(banca, iban);
    if (!c) return 0;
    if (c->id_utente != id_utente) return 0;
    if (c->saldo > 0.0) return 0;
    c->attivo = 0;
    return 1;
}

Conto *conto_cerca_iban(StatoBanca *banca, const char *iban) {
    for (int i = 0; i < banca->n_conti; i++) {
        if (banca->conti[i].attivo &&
            strcmp(banca->conti[i].iban, iban) == 0)
            return &banca->conti[i];
    }
    return NULL;
}

void conto_to_json(const Conto *c, char *out, int outsize) {
    snprintf(out, outsize,
        "{"
        "\"id\":%d,"
        "\"iban\":\"%s\","
        "\"id_utente\":%d,"
        "\"saldo\":%.2f,"
        "\"tipo\":\"%s\","
        "\"data_apertura\":\"%s\""
        "}",
        c->id, c->iban, c->id_utente, c->saldo,
        c->tipo == CONTO_CORRENTE ? "corrente" : "risparmio",
        c->data_apertura);
}

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
