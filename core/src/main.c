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
#include "../include/crypto.h"

/*
 * Protocollo: Node.js invia una riga JSON su stdin, il core risponde
 * con una riga JSON su stdout e poi termina.
 *
 * Comandi supportati:
 *   registra        — nome, cognome, eta, password, saldo_iniziale
 *   login           — username, password
 *   logout          — token
 *   profilo         — token
 *   aggiorna_profilo— token, nome, cognome
 *   cambia_password — token, vecchia_password, nuova_password
 *   lista_conti     — token
 *   estratto_conto  — token, iban
 *   preleva         — token, importo, descrizione
 *   invia           — token, iban_destinatario, importo, descrizione
 *   cerca_utenti    — token, query
 *   elimina_account — token, password
 */

StatoBanca banca;

/* Verifica token, ritorna id_utente o -1 */
int verifica_sessione(const char *json, char *token_out) {
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

/* Trova il conto primario (primo attivo) di un utente */
Conto *conto_primario(int id_utente) {
    for (int i = 0; i < banca.n_conti; i++) {
        if (banca.conti[i].id_utente == id_utente && banca.conti[i].attivo)
            return &banca.conti[i];
    }
    return NULL;
}

/* ---- Handler per ogni comando ---- */

void cmd_registra(const char *json) {
    char nome[64], cognome[64], password[64];
    double eta_d = 0.0, saldo_d = 0.0;
    char out[MAX_JSON_OUT];

    if (!json_get_str(json, "nome",     nome,     sizeof(nome))     ||
        !json_get_str(json, "cognome",  cognome,  sizeof(cognome))  ||
        !json_get_str(json, "password", password, sizeof(password)) ||
        !json_get_num(json, "eta",      &eta_d)) {
        json_errore("parametri mancanti (nome, cognome, eta, password)", out, sizeof(out));
        puts(out);
        return;
    }

    int eta = (int)eta_d;
    if (eta < 18) {
        json_errore("eta minima 18 anni", out, sizeof(out));
        puts(out);
        return;
    }

    json_get_num(json, "saldo_iniziale", &saldo_d);

    /* Usa username fornito oppure genera automaticamente */
    char username[64] = {0};
    json_get_str(json, "username", username, sizeof(username));

    if (username[0] == '\0') {
        int tentativi = 0;
        do {
            int suffisso = rand() % 1000;
            genera_username(nome, cognome, suffisso, username, sizeof(username));
            tentativi++;
        } while (utente_cerca_username(&banca, username) && tentativi < 200);
        if (tentativi >= 200) {
            json_errore("impossibile generare username univoco", out, sizeof(out));
            puts(out);
            return;
        }
    } else if (utente_cerca_username(&banca, username)) {
        json_errore("username già in uso", out, sizeof(out));
        puts(out);
        return;
    }

    /* Data nascita approssimata dall'eta */
    int anno = 1900 + (int)time(NULL) / 31557600 + 70 - eta;
    /* Calcolo corretto anno corrente */
    time_t t = time(NULL);
    struct tm *tm_now = localtime(&t);
    anno = (tm_now->tm_year + 1900) - eta;
    char data_nascita[32];
    snprintf(data_nascita, sizeof(data_nascita), "%04d-01-01", anno);

    char email_auto[128];
    snprintf(email_auto, sizeof(email_auto), "%s@cbank.local", username);

    int id = utente_aggiungi(&banca, username, password, nome, cognome,
                              email_auto, "0", data_nascita);
    if (id < 0) {
        json_errore("registrazione fallita", out, sizeof(out));
        puts(out);
        return;
    }
    salva_utenti(&banca);

    /* Apri conto corrente automaticamente */
    int id_conto = conto_apri(&banca, id, CONTO_CORRENTE);
    if (id_conto < 0) {
        json_errore("apertura conto fallita", out, sizeof(out));
        puts(out);
        return;
    }

    Conto *c = NULL;
    for (int i = 0; i < banca.n_conti; i++) {
        if (banca.conti[i].id == id_conto) { c = &banca.conti[i]; break; }
    }

    /* Deposito iniziale */
    if (saldo_d > 0.0 && c)
        deposita(&banca, c->iban, saldo_d, "Deposito iniziale");

    salva_dati(&banca);

    /* Auto-login */
    char token[65];
    utente_login(&banca, username, password, token);
    salva_sessioni(&banca);

    Utente *u = utente_cerca_id(&banca, id);
    char utente_json[1024];
    utente_to_json(u, utente_json, sizeof(utente_json));

    char data_json[2048];
    snprintf(data_json, sizeof(data_json),
        "{\"token\":\"%s\",\"username\":\"%s\",\"iban\":\"%s\",\"saldo\":%.2f,\"utente\":%s}",
        token, username, c ? c->iban : "", c ? c->saldo : 0.0, utente_json);

    json_ok(data_json, out, sizeof(out));
    puts(out);
}

void cmd_login(const char *json) {
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

    Conto *c = conto_primario(id);
    char data_json[2048];
    snprintf(data_json, sizeof(data_json),
        "{\"token\":\"%s\",\"iban\":\"%s\",\"saldo\":%.2f,\"utente\":%s}",
        token, c ? c->iban : "", c ? c->saldo : 0.0, utente_json);

    json_ok(data_json, out, sizeof(out));
    salva_sessioni(&banca);
    puts(out);
}

void cmd_logout(const char *json) {
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

void cmd_profilo(const char *json) {
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

    Conto *c = conto_primario(id);
    char utente_json[1024];
    utente_to_json(u, utente_json, sizeof(utente_json));

    /* Inietta iban e saldo nel JSON utente */
    char data_json[2048];
    snprintf(data_json, sizeof(data_json),
        "{\"utente\":%s,\"iban\":\"%s\",\"saldo\":%.2f}",
        utente_json, c ? c->iban : "", c ? c->saldo : 0.0);

    json_ok(data_json, out, sizeof(out));
    puts(out);
}

void cmd_aggiorna_profilo(const char *json) {
    char token[65], nome[64], cognome[64];
    char out[512];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    json_get_str(json, "nome",    nome,    sizeof(nome));
    json_get_str(json, "cognome", cognome, sizeof(cognome));

    if (!utente_aggiorna_profilo(&banca, id, nome, cognome, "", "")) {
        json_errore("aggiornamento profilo fallito", out, sizeof(out));
        puts(out);
        return;
    }

    salva_utenti(&banca);
    json_ok("", out, sizeof(out));
    puts(out);
}

void cmd_cambia_password(const char *json) {
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

void cmd_lista_conti(const char *json) {
    char token[65];
    char out[MAX_JSON_OUT];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    char conti_json[MAX_JSON_OUT - 64];
    conti_utente_to_json(&banca, id, conti_json, sizeof(conti_json));
    json_ok(conti_json, out, sizeof(out));
    puts(out);
}

void cmd_estratto_conto(const char *json) {
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

void cmd_preleva(const char *json) {
    char token[65], descrizione[128];
    double importo = 0.0;
    char out[512];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    if (!json_get_num(json, "importo", &importo)) {
        json_errore("importo mancante", out, sizeof(out));
        puts(out);
        return;
    }
    json_get_str(json, "descrizione", descrizione, sizeof(descrizione));

    Conto *c = conto_primario(id);
    if (!c) {
        json_errore("nessun conto trovato", out, sizeof(out));
        puts(out);
        return;
    }

    if (!preleva(&banca, c->iban, importo, descrizione)) {
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

void cmd_invia(const char *json) {
    char token[65], iban_dest[35], descrizione[128];
    double importo = 0.0;
    char out[512];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    if (!json_get_str(json, "iban_destinatario", iban_dest, sizeof(iban_dest)) ||
        !json_get_num(json, "importo", &importo)) {
        json_errore("parametri mancanti", out, sizeof(out));
        puts(out);
        return;
    }
    json_get_str(json, "descrizione", descrizione, sizeof(descrizione));

    Conto *mittente = conto_primario(id);
    if (!mittente) {
        json_errore("nessun conto trovato", out, sizeof(out));
        puts(out);
        return;
    }

    if (strcmp(mittente->iban, iban_dest) == 0) {
        json_errore("non puoi inviare a te stesso", out, sizeof(out));
        puts(out);
        return;
    }

    int esito = bonifico(&banca, mittente->iban, iban_dest, importo, descrizione);

    if (esito == -2) { json_errore("fondi insufficienti", out, sizeof(out)); puts(out); return; }
    if (esito == -3) { json_errore("IBAN destinatario non trovato", out, sizeof(out)); puts(out); return; }
    if (esito < 0)   { json_errore("invio fallito", out, sizeof(out)); puts(out); return; }

    salva_dati(&banca);

    char saldo_json[64];
    snprintf(saldo_json, sizeof(saldo_json), "{\"saldo\":%.2f}", mittente->saldo);
    json_ok(saldo_json, out, sizeof(out));
    puts(out);
}

void cmd_cerca_utenti(const char *json) {
    char token[65], query[128];
    char out[MAX_JSON_OUT];

    int id_self = verifica_sessione(json, token);
    if (id_self < 0) return;

    query[0] = '\0';
    json_get_str(json, "query", query, sizeof(query));

    char arr[MAX_JSON_OUT - 64];
    int pos = 0;
    arr[pos++] = '[';
    int first = 1;

    for (int i = 0; i < banca.n_utenti; i++) {
        Utente *u = &banca.utenti[i];
        if (!u->attivo || u->id == id_self) continue;

        if (query[0] &&
            !str_contains_ci(u->nome,     query) &&
            !str_contains_ci(u->cognome,  query) &&
            !str_contains_ci(u->username, query))
            continue;

        Conto *c = conto_primario(u->id);
        if (!c) continue;

        char entry[512];
        int elen = snprintf(entry, sizeof(entry),
            "{\"id\":%d,\"nome\":\"%s\",\"cognome\":\"%s\",\"username\":\"%s\",\"iban\":\"%s\"}",
            u->id, u->nome, u->cognome, u->username, c->iban);

        if (pos + elen + 4 >= (int)sizeof(arr)) break;
        if (!first) arr[pos++] = ',';
        memcpy(arr + pos, entry, elen);
        pos += elen;
        first = 0;
    }
    arr[pos++] = ']';
    arr[pos] = '\0';

    json_ok(arr, out, sizeof(out));
    puts(out);
}

void cmd_elimina_account(const char *json) {
    char token[65], password[64];
    char out[256];

    int id = verifica_sessione(json, token);
    if (id < 0) return;

    if (!json_get_str(json, "password", password, sizeof(password))) {
        json_errore("password mancante", out, sizeof(out));
        puts(out);
        return;
    }

    Utente *u = utente_cerca_id(&banca, id);
    if (!u) {
        json_errore("utente non trovato", out, sizeof(out));
        puts(out);
        return;
    }

    if (!password_verifica(password, u->password_hex)) {
        json_errore("password errata", out, sizeof(out));
        puts(out);
        return;
    }

    /* Disattiva tutti i conti dell'utente */
    for (int i = 0; i < banca.n_conti; i++) {
        if (banca.conti[i].id_utente == id)
            banca.conti[i].attivo = 0;
    }

    /* Disattiva utente */
    u->attivo = 0;

    /* Rimuovi tutte le sessioni */
    sessioni_rimuovi_utente(&banca, id);

    salva_dati(&banca);
    salva_utenti(&banca);
    salva_sessioni(&banca);

    json_ok("", out, sizeof(out));
    puts(out);
}

/* ---- Entry point ---- */

int main(void) {
    srand((unsigned)time(NULL));

    memset(&banca, 0, sizeof(StatoBanca));
    utenti_init(&banca);
    conti_init(&banca);
    banca.prossimo_id_utente      = 1;
    banca.prossimo_id_conto       = 1;
    banca.prossimo_id_transazione = 1;

    carica_dati(&banca);

    char input[MAX_INPUT];
    if (!fgets(input, sizeof(input), stdin)) {
        char out[256];
        json_errore("nessun input ricevuto", out, sizeof(out));
        puts(out);
        goto cleanup;
    }
    str_trim(input);

    char cmd[64] = {0};
    if (!json_get_str(input, "cmd", cmd, sizeof(cmd))) {
        char out[256];
        json_errore("campo cmd mancante", out, sizeof(out));
        puts(out);
        goto cleanup;
    }

    if      (strcmp(cmd, "registra")         == 0) cmd_registra(input);
    else if (strcmp(cmd, "login")            == 0) cmd_login(input);
    else if (strcmp(cmd, "logout")           == 0) cmd_logout(input);
    else if (strcmp(cmd, "profilo")          == 0) cmd_profilo(input);
    else if (strcmp(cmd, "aggiorna_profilo") == 0) cmd_aggiorna_profilo(input);
    else if (strcmp(cmd, "cambia_password")  == 0) cmd_cambia_password(input);
    else if (strcmp(cmd, "lista_conti")      == 0) cmd_lista_conti(input);
    else if (strcmp(cmd, "estratto_conto")   == 0) cmd_estratto_conto(input);
    else if (strcmp(cmd, "preleva")          == 0) cmd_preleva(input);
    else if (strcmp(cmd, "invia")            == 0) cmd_invia(input);
    else if (strcmp(cmd, "cerca_utenti")     == 0) cmd_cerca_utenti(input);
    else if (strcmp(cmd, "elimina_account")  == 0) cmd_elimina_account(input);
    else {
        char out[256];
        json_errore("comando sconosciuto", out, sizeof(out));
        puts(out);
    }

cleanup:
    conti_libera(&banca);
    utenti_libera(&banca);
    coda_libera(&banca.notifiche);

    return 0;
}
