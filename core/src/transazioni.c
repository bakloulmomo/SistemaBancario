#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/transazioni.h"
#include "../include/conti.h"
#include "../include/utils.h"

// Transizioni gestiti tramite Lista linkata
// crea Nodo transizione 
Transazione *transazione_aggiungi(StatoBanca *banca, Conto *conto,
                                   TipoTransazione tipo, double importo,
                                   const char *descrizione,
                                   const char *iban_controparte) {
    Transazione *t = (Transazione *)malloc(sizeof(Transazione));
    if (!t) return NULL;

    // stesso ragionamento per l'array, per azzerare i campi del nodo, in caso di errori
    memset(t, 0, sizeof(Transazione));
    t->id      = banca->prossimo_id_transazione++;
    t->tipo    = tipo;
    t->importo = importo;

    strncpy(t->descrizione, descrizione ? descrizione : "", 127);
    strncpy(t->iban_controparte, iban_controparte ? iban_controparte : "", 34);
    data_ora_corrente(t->data);

    // inserisce in lista
    t->next = conto->transazioni;
    conto->transazioni = t;

    return t;
}

// deposita soldi all'account inizialmente 
int deposita(StatoBanca *banca, const char *iban,
             double importo, const char *descrizione) {
    if (importo <= 0.0) return 0;


    Conto *c = conto_cerca_iban(banca, iban);
    if (!c) return 0;

    c->saldo += importo;
    // la aggiunge nello storico delle transazioni
    transazione_aggiungi(banca, c, TX_DEPOSITO, importo, descrizione, "");
    return 1;
}

// preleva soldi dall'account
int preleva(StatoBanca *banca, const char *iban,
            double importo, const char *descrizione) {
    if (importo <= 0.0) return 0;

    Conto *c = conto_cerca_iban(banca, iban);
    if (!c) return 0;

    // soldi insufficenti
    if (c->saldo < importo) return 0;

    c->saldo -= importo;
    // salva transazione
    transazione_aggiungi(banca, c, TX_PRELIEVO, importo, descrizione, "");
    return 1;
}

// per cmd, -1 = importo <= 0; -2 = soldi insufficenti; -3 IBAN RX non trovato
// servono per dire all'utente (tramite JSON) il tipo di errore nel codice

// processo bonifico
int bonifico(StatoBanca *banca,
             const char *iban_mittente,  const char *iban_destinatario,
             double importo, const char *descrizione) {
    if (importo <= 0.0) return -1;

    // trovo IBAN mittente
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

// nel cleanup, libera memoria lista
void transazioni_libera(Transazione *testa) {
    while (testa) {
        Transazione *next = testa->next;
        free(testa);
        testa = next;
    }
}

// manda la transizione json all'altra funzione
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

// questa funzione, per mandare tutto al server in un colpo
void transazioni_to_json(const Transazione *testa, char *out, int outsize) {
    int written = 0; // quanti byte sono gia scritti nel buffer out
    int first   = 1; // flag per la virgola (per JSON)

    written += snprintf(out + written, outsize - written, "[");
    //inizializza JSON con [

    for (const Transazione *t = testa; t && written < outsize - 2; t = t->next) {
        if (!first) written += snprintf(out + written, outsize - written, ","); // se first = 0 aggiunge una virgola
        first = 0;

        char tmp[512];
        transazione_to_json(t, tmp, sizeof(tmp)); // aggiunge transizione in tmp
        written += snprintf(out + written, outsize - written, "%s", tmp); // scrive infine tutta la transizione in out
    }

    snprintf(out + written, outsize - written, "]");
    //finisce JSON con ]
}


// converte numero enum in stringa leggibile per il client
// tutto il JSON (o quasi) serve all'utente per mostrargli l'errore
const char *tipo_transazione_str(TipoTransazione tipo) {
    switch (tipo) {
        case TX_DEPOSITO:     return "deposito";
        case TX_PRELIEVO:     return "prelievo";
        case TX_BONIFICO_OUT: return "bonifico_uscita";
        case TX_BONIFICO_IN:  return "bonifico_entrata";
        default:              return "sconosciuto";
    }
}
