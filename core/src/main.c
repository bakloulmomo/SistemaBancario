#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/tipi.h"
#include "../include/utenti.h"
#include "../include/conti.h"
#include "../include/transazioni.h"
#include "../include/file_io.h"
#include "../include/utils.h"

/*
 * Protocollo: Node.js invia una riga JSON su stdin, il core risponde
 * con una riga JSON su stdout e poi termina.
 * Formato input: {"cmd":"<comando>", ...parametri...}
 * Formato output: {"status":"ok","data":{...}} oppure {"status":"error","message":"..."}
 *
 * Comandi supportati:
 *   registra       — username, password, nome, cognome, email, telefono, data_nascita
 *   login          — username, password
 *   logout         — token
 *   profilo        — token
 *   aggiorna_profilo — token, nome, cognome, email, telefono
 *   cambia_password  — token, vecchia_password, nuova_password
 *   apri_conto     — token, tipo  (0=corrente, 1=risparmio)
 *   chiudi_conto   — token, iban
 *   lista_conti    — token
 *   estratto_conto — token, iban
 *   deposita       — token, iban, importo, descrizione
 *   preleva        — token, iban, importo, descrizione
 *   bonifico       — token, iban_mittente, iban_destinatario, importo, descrizione
 *   statistiche    — token
 */

static StatoBanca banca;

/* Verifica token e ritorna id_utente, oppure stampa errore e ritorna -1 */
static int verifica_sessione(const char *json, char *token_out) {
    if (!json_get_str(json, "token", token_out, 65)) {
        char out[256];
        json_errore("token mancante", out, sizeof(out));
        puts(out);
        return -1;
    }
    int id = sessione_verifica(&banca, token_out);
    if (id < 0) {
        char out[256];
        json_errore("sessione non valida o scaduta", out, sizeof(out));
        puts(out);
        return -1;
    }
    return id;
}

/* ---- Handler per ogni comando ---- */

static void cmd_registra(const char *json) {
    char username[64], password[64], nome[64], cognome[64];
    char email[128], telefono[20], data_nascita[12];
    char out[MAX_JSON_OUT];

    if (!json_get_str(json, "username",     username,     sizeof(username))    ||
        !json_get_str(json, "password",     password,     sizeof(password))    ||
        !json_get_str(json, "nome",         nome,         sizeof(nome))        ||
        !json_get_str(json, "cognome",      cognome,      sizeof(cognome))     ||
        !json_get_str(json, "email",        email,        sizeof(email))       ||
        !json_get_str(json, "telefono",     telefono,     sizeof(telefono))    ||
        !json_get_str(json, "data_nascita", data_nascita, sizeof(data_nascita))) {
        json_errore("parametri mancanti", out, sizeof(out));
        puts(out);
        return;
    }

    int id = utente_aggiungi(&banca, username, password, nome, cognome,
                              email, telefono, data_nascita);
    if (id < 0) {
        json_errore("username già esistente", out, sizeof(out));
        puts(out);
        return;
    }

    salva_utenti(&banca);

    Utente *u = utente_cerca_id(&banca, id);
    char utente_json[1024];
    utente_to_json(u, utente_json, sizeof(utente_json));
    json_ok(utente_json, out, sizeof(out));
    puts(out);
}

static void cmd_login(const char *json) {
    char username[64], password[64];
    char token[65];
    char out[MAX_JSON_OUT];

    if (!json_get_str(json, "username", username, sizeof(username)) ||
        !json_get_str(json, "password", password, sizeof(password))) {
        json_errore("parametri mancanti", out, sizeof(out));
        puts(out);
        return;
    }

    int id = utente_login(&banca, username, password, token);
    if (id < 0) {
        json_errore("credenziali non valide", out, sizeof(out));
        puts(out);
        return;
    }

    Utente *u = utente_cerca_id(&banca, id);
    char utente_json[1024];
    utente_to_json(u, utente_json, sizeof(utente_json));

    /* Aggiunge il token alla risposta */
    char data_json[2048];
    snprintf(data_json, sizeof(data_json),
             "{\"token\":\"%s\",\"utente\":%s}", token, utente_json);

    json_ok(data_json, out, sizeof(out));
    salva_sessioni(&banca);
    puts(out);
}

static void cmd_logout(const char *json) {
    char token[65];
    char out[256];

    if (!json_get_str(json, "token", token, sizeof(token))) {
        json_errore("token mancante", out, sizeof(out));
        puts(out);
        return;
    }

    utente_logout(&banca, token);
    salva_sessioni(&banca);
    json_ok("", out, sizeof(out));
    puts(out);
}

static void cmd_profilo(const char *json) {
    char token[65];
    char out[MAX_JSON_OUT];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    Utente *u = utente_cerca_id(&banca, id);
    if (!u) {
        json_errore("utente non trovato", out, sizeof(out));
        puts(out);
        return;
    }

    char utente_json[1024];
    utente_to_json(u, utente_json, sizeof(utente_json));
    json_ok(utente_json, out, sizeof(out));
    puts(out);
}

static void cmd_aggiorna_profilo(const char *json) {
    char token[65], nome[64], cognome[64], email[128], telefono[20];
    char out[512];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    json_get_str(json, "nome",     nome,     sizeof(nome));
    json_get_str(json, "cognome",  cognome,  sizeof(cognome));
    json_get_str(json, "email",    email,    sizeof(email));
    json_get_str(json, "telefono", telefono, sizeof(telefono));

    if (!utente_aggiorna_profilo(&banca, id, nome, cognome, email, telefono)) {
        json_errore("aggiornamento profilo fallito", out, sizeof(out));
        puts(out);
        return;
    }

    salva_utenti(&banca);
    json_ok("", out, sizeof(out));
    puts(out);
}

static void cmd_cambia_password(const char *json) {
    char token[65], vecchia[64], nuova[64];
    char out[256];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    if (!json_get_str(json, "vecchia_password", vecchia, sizeof(vecchia)) ||
        !json_get_str(json, "nuova_password",   nuova,   sizeof(nuova))) {
        json_errore("parametri mancanti", out, sizeof(out));
        puts(out);
        return;
    }

    if (!utente_cambia_password(&banca, id, vecchia, nuova)) {
        json_errore("password attuale errata", out, sizeof(out));
        puts(out);
        return;
    }

    salva_utenti(&banca);
    json_ok("", out, sizeof(out));
    puts(out);
}

static void cmd_apri_conto(const char *json) {
    char token[65];
    double tipo_num = 0.0;
    char out[MAX_JSON_OUT];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    json_get_num(json, "tipo", &tipo_num);
    TipoConto tipo = (tipo_num == 1.0) ? CONTO_RISPARMIO : CONTO_CORRENTE;

    int id_conto = conto_apri(&banca, id, tipo);
    if (id_conto < 0) {
        json_errore("apertura conto fallita", out, sizeof(out));
        puts(out);
        return;
    }

    salva_conti(&banca);

    Conto *c = NULL;
    for (int i = 0; i < banca.n_conti; i++) {
        if (banca.conti[i].id == id_conto) { c = &banca.conti[i]; break; }
    }

    char conto_json[512];
    conto_to_json(c, conto_json, sizeof(conto_json));
    json_ok(conto_json, out, sizeof(out));
    puts(out);
}

static void cmd_chiudi_conto(const char *json) {
    char token[65], iban[35];
    char out[256];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    if (!json_get_str(json, "iban", iban, sizeof(iban))) {
        json_errore("iban mancante", out, sizeof(out));
        puts(out);
        return;
    }

    if (!conto_chiudi(&banca, id, iban)) {
        json_errore("chiusura non consentita (saldo > 0 o conto non trovato)", out, sizeof(out));
        puts(out);
        return;
    }

    salva_conti(&banca);
    json_ok("", out, sizeof(out));
    puts(out);
}

static void cmd_lista_conti(const char *json) {
    char token[65];
    char out[MAX_JSON_OUT];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    char conti_json[MAX_JSON_OUT - 64];
    conti_utente_to_json(&banca, id, conti_json, sizeof(conti_json));
    json_ok(conti_json, out, sizeof(out));
    puts(out);
}

static void cmd_estratto_conto(const char *json) {
    char token[65], iban[35];
    char out[MAX_JSON_OUT];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    if (!json_get_str(json, "iban", iban, sizeof(iban))) {
        json_errore("iban mancante", out, sizeof(out));
        puts(out);
        return;
    }

    Conto *c = conto_cerca_iban(&banca, iban);
    if (!c || c->id_utente != id) {
        json_errore("conto non trovato", out, sizeof(out));
        puts(out);
        return;
    }

    char conto_json[512];
    conto_to_json(c, conto_json, sizeof(conto_json));

    char tx_json[MAX_JSON_OUT - 1024];
    transazioni_to_json(c->transazioni, tx_json, sizeof(tx_json));

    char data_json[MAX_JSON_OUT - 256];
    snprintf(data_json, sizeof(data_json),
             "{\"conto\":%s,\"transazioni\":%s}", conto_json, tx_json);

    json_ok(data_json, out, sizeof(out));
    puts(out);
}

static void cmd_deposita(const char *json) {
    char token[65], iban[35], descrizione[128];
    double importo = 0.0;
    char out[512];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    if (!json_get_str(json, "iban", iban, sizeof(iban)) ||
        !json_get_num(json, "importo", &importo)) {
        json_errore("parametri mancanti", out, sizeof(out));
        puts(out);
        return;
    }
    json_get_str(json, "descrizione", descrizione, sizeof(descrizione));

    /* Verifica che il conto appartenga all'utente */
    Conto *c = conto_cerca_iban(&banca, iban);
    if (!c || c->id_utente != id) {
        json_errore("conto non trovato", out, sizeof(out));
        puts(out);
        return;
    }

    if (!deposita(&banca, iban, importo, descrizione)) {
        json_errore("deposito non consentito", out, sizeof(out));
        puts(out);
        return;
    }

    salva_dati(&banca);

    char saldo_json[64];
    snprintf(saldo_json, sizeof(saldo_json), "{\"saldo\":%.2f}", c->saldo);
    json_ok(saldo_json, out, sizeof(out));
    puts(out);
}

static void cmd_preleva(const char *json) {
    char token[65], iban[35], descrizione[128];
    double importo = 0.0;
    char out[512];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    if (!json_get_str(json, "iban", iban, sizeof(iban)) ||
        !json_get_num(json, "importo", &importo)) {
        json_errore("parametri mancanti", out, sizeof(out));
        puts(out);
        return;
    }
    json_get_str(json, "descrizione", descrizione, sizeof(descrizione));

    Conto *c = conto_cerca_iban(&banca, iban);
    if (!c || c->id_utente != id) {
        json_errore("conto non trovato", out, sizeof(out));
        puts(out);
        return;
    }

    if (!preleva(&banca, iban, importo, descrizione)) {
        json_errore("fondi insufficienti", out, sizeof(out));
        puts(out);
        return;
    }

    salva_dati(&banca);

    char saldo_json[64];
    snprintf(saldo_json, sizeof(saldo_json), "{\"saldo\":%.2f}", c->saldo);
    json_ok(saldo_json, out, sizeof(out));
    puts(out);
}

static void cmd_bonifico(const char *json) {
    char token[65], iban_m[35], iban_d[35], descrizione[128];
    double importo = 0.0;
    char out[512];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    if (!json_get_str(json, "iban_mittente",     iban_m,    sizeof(iban_m))   ||
        !json_get_str(json, "iban_destinatario", iban_d,    sizeof(iban_d))   ||
        !json_get_num(json, "importo",           &importo)) {
        json_errore("parametri mancanti", out, sizeof(out));
        puts(out);
        return;
    }
    json_get_str(json, "descrizione", descrizione, sizeof(descrizione));

    Conto *mittente = conto_cerca_iban(&banca, iban_m);
    if (!mittente || mittente->id_utente != id) {
        json_errore("conto mittente non trovato", out, sizeof(out));
        puts(out);
        return;
    }

    int esito = bonifico(&banca, iban_m, iban_d, importo, descrizione);

    if (esito == -2) { json_errore("fondi insufficienti", out, sizeof(out)); puts(out); return; }
    if (esito == -3) { json_errore("IBAN destinatario non trovato", out, sizeof(out)); puts(out); return; }
    if (esito < 0)   { json_errore("bonifico fallito", out, sizeof(out)); puts(out); return; }

    salva_dati(&banca);

    char saldo_json[64];
    snprintf(saldo_json, sizeof(saldo_json), "{\"saldo\":%.2f}", mittente->saldo);
    json_ok(saldo_json, out, sizeof(out));
    puts(out);
}

static void cmd_statistiche(const char *json) {
    char token[65];
    char out[MAX_JSON_OUT];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    char stat_json[MAX_JSON_OUT - 64];
    statistiche_json(&banca, id, stat_json, sizeof(stat_json));
    json_ok(stat_json, out, sizeof(out));
    puts(out);
}

/* ---- Entry point ---- */

int main(void) {
    srand((unsigned)time(NULL));

    /* Inizializza strutture */
    memset(&banca, 0, sizeof(StatoBanca));
    utenti_init(&banca);
    conti_init(&banca);
    banca.prossimo_id_utente      = 1;
    banca.prossimo_id_conto       = 1;
    banca.prossimo_id_transazione = 1;

    carica_dati(&banca);

    /* Leggi una riga JSON da stdin */
    char input[MAX_INPUT];
    if (!fgets(input, sizeof(input), stdin)) {
        char out[256];
        json_errore("nessun input ricevuto", out, sizeof(out));
        puts(out);
        goto cleanup;
    }
    str_trim(input);

    /* Estrai il campo "cmd" */
    char cmd[64] = {0};
    if (!json_get_str(input, "cmd", cmd, sizeof(cmd))) {
        char out[256];
        json_errore("campo cmd mancante", out, sizeof(out));
        puts(out);
        goto cleanup;
    }

    /* Dispatch */
    if      (strcmp(cmd, "registra")         == 0) cmd_registra(input);
    else if (strcmp(cmd, "login")            == 0) cmd_login(input);
    else if (strcmp(cmd, "logout")           == 0) cmd_logout(input);
    else if (strcmp(cmd, "profilo")          == 0) cmd_profilo(input);
    else if (strcmp(cmd, "aggiorna_profilo") == 0) cmd_aggiorna_profilo(input);
    else if (strcmp(cmd, "cambia_password")  == 0) cmd_cambia_password(input);
    else if (strcmp(cmd, "apri_conto")       == 0) cmd_apri_conto(input);
    else if (strcmp(cmd, "chiudi_conto")     == 0) cmd_chiudi_conto(input);
    else if (strcmp(cmd, "lista_conti")      == 0) cmd_lista_conti(input);
    else if (strcmp(cmd, "estratto_conto")   == 0) cmd_estratto_conto(input);
    else if (strcmp(cmd, "deposita")         == 0) cmd_deposita(input);
    else if (strcmp(cmd, "preleva")          == 0) cmd_preleva(input);
    else if (strcmp(cmd, "bonifico")         == 0) cmd_bonifico(input);
    else if (strcmp(cmd, "statistiche")      == 0) cmd_statistiche(input);
    else {
        char out[256];
        json_errore("comando sconosciuto", out, sizeof(out));
        puts(out);
    }

cleanup:
    /* Libera memoria */
    conti_libera(&banca);
    utenti_libera(&banca);
    coda_libera(&banca.notifiche);

    return 0;
}
