#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/transazioni.h"
#include "../include/conti.h"
#include "../include/utils.h"

Transazione *transazione_aggiungi(StatoBanca *banca, Conto *conto,
                                   TipoTransazione tipo, double importo,
                                   const char *descrizione,
                                   const char *iban_controparte) {
    Transazione *t = (Transazione *)malloc(sizeof(Transazione));
    if (!t) return NULL;

    memset(t, 0, sizeof(Transazione));
    t->id      = banca->prossimo_id_transazione++;
    t->tipo    = tipo;
    t->importo = importo;

    strncpy(t->descrizione, descrizione ? descrizione : "", 127);
    strncpy(t->iban_controparte, iban_controparte ? iban_controparte : "", 34);
    data_ora_corrente(t->data);

    /* Inserisce in testa alla lista */
    t->next = conto->transazioni;
    conto->transazioni = t;

    return t;
}

int deposita(StatoBanca *banca, const char *iban,
             double importo, const char *descrizione) {
    if (importo <= 0.0) return 0;

    Conto *c = conto_cerca_iban(banca, iban);
    if (!c) return 0;

    c->saldo += importo;
    transazione_aggiungi(banca, c, TX_DEPOSITO, importo, descrizione, "");
    return 1;
}

int preleva(StatoBanca *banca, const char *iban,
            double importo, const char *descrizione) {
    if (importo <= 0.0) return 0;

    Conto *c = conto_cerca_iban(banca, iban);
    if (!c) return 0;

    if (c->saldo < importo) return 0;

    c->saldo -= importo;
    transazione_aggiungi(banca, c, TX_PRELIEVO, importo, descrizione, "");
    return 1;
}

int bonifico(StatoBanca *banca,
             const char *iban_mittente,  const char *iban_destinatario,
             double importo, const char *descrizione) {
    if (importo <= 0.0) return -1;

    Conto *mittente = conto_cerca_iban(banca, iban_mittente);
    if (!mittente) return -1;

    if (mittente->saldo < importo) return -2;

    Conto *destinatario = conto_cerca_iban(banca, iban_destinatario);
    if (!destinatario) return -3;

    mittente->saldo -= importo;
    transazione_aggiungi(banca, mittente,    TX_BONIFICO_OUT, importo,
                         descrizione, iban_destinatario);

    destinatario->saldo += importo;
    transazione_aggiungi(banca, destinatario, TX_BONIFICO_IN,  importo,
                         descrizione, iban_mittente);

    return 1;
}

void transazioni_libera(Transazione *testa) {
    while (testa) {
        Transazione *next = testa->next;
        free(testa);
        testa = next;
    }
}

void transazione_to_json(const Transazione *t, char *out, int outsize) {
    snprintf(out, outsize,
        "{"
        "\"id\":%d,"
        "\"tipo\":\"%s\","
        "\"importo\":%.2f,"
        "\"descrizione\":\"%s\","
        "\"data\":\"%s\","
        "\"iban_controparte\":\"%s\""
        "}",
        t->id,
        tipo_transazione_str(t->tipo),
        t->importo,
        t->descrizione,
        t->data,
        t->iban_controparte);
}

void transazioni_to_json(const Transazione *testa, char *out, int outsize) {
    int written = 0;
    int first   = 1;

    written += snprintf(out + written, outsize - written, "[");

    for (const Transazione *t = testa;
         t && written < outsize - 2; t = t->next) {
        if (!first) written += snprintf(out + written, outsize - written, ",");
        first = 0;

        char tmp[512];
        transazione_to_json(t, tmp, sizeof(tmp));
        written += snprintf(out + written, outsize - written, "%s", tmp);
    }

    snprintf(out + written, outsize - written, "]");
}

const char *tipo_transazione_str(TipoTransazione tipo) {
    switch (tipo) {
        case TX_DEPOSITO:     return "deposito";
        case TX_PRELIEVO:     return "prelievo";
        case TX_BONIFICO_OUT: return "bonifico_uscita";
        case TX_BONIFICO_IN:  return "bonifico_entrata";
        default:              return "sconosciuto";
    }
}
