#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/tipi.h"
#include "../include/utenti.h"
#include "../include/conti.h"
#include "../include/transazioni.h"
#include "../include/file_io.h"
#include "../include/utils.h"
#include "../include/crypto.h"

// protocollo: Node.js invia una riga JSON su stdin, il core risponde
// con una riga JSON su stdout e poi finisce il tutto
//
// comandi:
//   registra        — username, nome, cognome, eta, password, saldo_iniziale
//   login           — username, password
//   profilo         — username, password
//   aggiorna_profilo  — username, password, nome, cognome
//   cambia_password — username, password, nuova_password
//   lista_conti     — username, password
//   estratto_conto  — username, password, iban
//   preleva         — username, password, importo
//   invia           — username, password, iban_destinatario, importo
//   cerca_utenti    — username, password, query
//   elimina_account — username, password

StatoBanca banca;

// verifica credenziali di ritorno dal JSON, ritorna id_utente o -1
int verifica_credenziali(const char *json) {
    char username[64], password[64];
    char out[256];
    if (!json_get_str(json, "username", username, sizeof(username)) ||
        !json_get_str(json, "password", password, sizeof(password))) {
        json_errore("username o password mancanti", out, sizeof(out));
        puts(out);
        return -1;
    }
    int id = utente_login(&banca, username, password);
    if (id < 0) {
        json_errore("credenziali non valide", out, sizeof(out));
        puts(out);
        return -1;
    }
    return id;
}

void cmd_registra(const char *json) {
    char nome[64], cognome[64], password[64], username[64];
    double eta_d = 0, saldo_d = 0;
    char out[MAX_JSON_OUT];

    if (!json_get_str(json, "nome",     nome,     sizeof(nome))     ||
           !json_get_str(json, "cognome",  cognome,  sizeof(cognome))  ||
        !json_get_str(json, "password", password, sizeof(password)) ||
        !json_get_str(json, "username", username, sizeof(username)) ||
        !json_get_num(json, "eta",      &eta_d)) {
        json_errore("parametri mancanti (nome, cognome, username, eta, password)", out, sizeof(out));
        puts(out);
        return;
    }

    int eta   = (int)eta_d;
    json_get_num(json, "saldo_iniziale", &saldo_d);
    int saldo = (int)saldo_d;

    if (eta < 18) {
        json_errore("eta minima 18 anni", out, sizeof(out));
        puts(out);
        return;
    }

    // controllo se username gia in uso
    if (utente_cerca_username(&banca, username)) {
        json_errore("username già in uso", out, sizeof(out));
        puts(out);
        return;
    }
    
    // aggiunge utente all'array e gli crea un id
    int id = utente_aggiungi(&banca, username, password, nome, cognome);
    if (id < 0) {
        json_errore("registrazione fallita", out, sizeof(out));
        puts(out);
        return;
    }
    // risalva tutto l'array con un nuovo utente
    salva_utenti(&banca);

    // apriamo conto, dandogli un conto.id = all'utente.id
    int id_conto = conto_apri(&banca, id);
    if (id_conto < 0) {
        json_errore("apertura conto fallita", out, sizeof(out));
        puts(out);
        return;
    }

    // crea un conto all'utente
    Conto *c = NULL;
    // aggiungiamo il conto all'array dei conti con il suo id creato prima
    for (int i = 0; i < banca.n_conti; i++) {
        if (banca.conti[i].id == id_conto) {
            c = &banca.conti[i]; 
            break; 
        }
    }

    // se il saldo > 0, salviamo l'importo nel conto (lo aggiunge anche come transizione)
    if (saldo > 0 && c)
        deposita(&banca, c->iban, saldo);

    // risalva dati dell'utente
    salva_dati(&banca);

    // cerca nell'array banca->utenti l'utente tramite il suo id
    Utente *u = utente_cerca_id(&banca, id); 
    char utente_json[512];
    // converte i suoi dati in una stringa JSON e la mette in utente_json
    utente_to_json(u, utente_json, sizeof(utente_json));

    // madiamo anche i dati del conto dell'utente
    char data_json[1024];
    snprintf(data_json, sizeof(data_json),
        "{\"username\":\"%s\",\"iban\":\"%s\",\"saldo\":%d,\"utente\":%s}",
        username, c ? c->iban : "", c ? c->saldo : 0, utente_json); // qua mandiamo tutte le informazioni dell'utente

    // andato tutto ok, scriviamo sul buffer out per il sito per dargli una risposta di successo, con status: ok
    // puo procedere il sito
    json_ok(data_json, out, sizeof(out));
    puts(out); // stampa sul buffer stdout 
}

void cmd_login(const char *json) {
    char out[MAX_JSON_OUT];

    int id = verifica_credenziali(json);
    if (id < 0) return;

    Utente *u = utente_cerca_id(&banca, id);
    char utente_json[512];
    utente_to_json(u, utente_json, sizeof(utente_json));

    Conto *c = NULL;
    for (int i = 0; i < banca.n_conti; i++)
        if (banca.conti[i].id_utente == id && banca.conti[i].attivo) { c = &banca.conti[i]; break; }

    char data_json[1024];
    snprintf(data_json, sizeof(data_json),
        "{\"iban\":\"%s\",\"saldo\":%d,\"utente\":%s}",
        c ? c->iban : "", c ? c->saldo : 0, utente_json);

    json_ok(data_json, out, sizeof(out));
    puts(out);
}

void cmd_profilo(const char *json) {
    char out[MAX_JSON_OUT];

    int id = verifica_credenziali(json);
    if (id < 0) return;

    Utente *u = utente_cerca_id(&banca, id);
    if (!u) {
        json_errore("utente non trovato", out, sizeof(out));
        puts(out);
        return;
    }

    Conto *c = NULL;
    for (int i = 0; i < banca.n_conti; i++)
        if (banca.conti[i].id_utente == id && banca.conti[i].attivo) { c = &banca.conti[i]; break; }

    char utente_json[512];
    utente_to_json(u, utente_json, sizeof(utente_json));

    char data_json[1024];
    snprintf(data_json, sizeof(data_json),
        "{\"utente\":%s,\"iban\":\"%s\",\"saldo\":%d}",
        utente_json, c ? c->iban : "", c ? c->saldo : 0);

    json_ok(data_json, out, sizeof(out));
    puts(out);
}

void cmd_aggiorna_profilo(const char *json) {
    char nome[64], cognome[64];
    char out[256];

    int id = verifica_credenziali(json);
    if (id < 0) return;

    json_get_str(json, "nome",    nome,    sizeof(nome));
    json_get_str(json, "cognome", cognome, sizeof(cognome));

    if (!utente_aggiorna_profilo(&banca, id, nome, cognome)) {
        json_errore("aggiornamento profilo fallito", out, sizeof(out));
        puts(out);
        return;
    }

    salva_utenti(&banca);
    json_ok("", out, sizeof(out));
    puts(out);
}

void cmd_cambia_password(const char *json) {
    char nuova[64];
    char out[256];

    int id = verifica_credenziali(json);
    if (id < 0) return;

    if (!json_get_str(json, "nuova_password", nuova, sizeof(nuova))) {
        json_errore("nuova_password mancante", out, sizeof(out));
        puts(out);
        return;
    }

    if (!utente_cambia_password(&banca, id, nuova)) {
        json_errore("cambio password fallito", out, sizeof(out));
        puts(out);
        return;
    }

    salva_utenti(&banca);
    json_ok("", out, sizeof(out));
    puts(out);
}

void cmd_lista_conti(const char *json) {
    char out[MAX_JSON_OUT];

    int id = verifica_credenziali(json);
    if (id < 0) return;

    char conti_json[MAX_JSON_OUT - 64];
    conti_utente_to_json(&banca, id, conti_json, sizeof(conti_json));
    json_ok(conti_json, out, sizeof(out));
    puts(out);
}

void cmd_estratto_conto(const char *json) {
    char iban[35];
    char out[MAX_JSON_OUT];

    int id = verifica_credenziali(json);
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

    char conto_json[256];
    conto_to_json(c, conto_json, sizeof(conto_json));

    char tx_json[MAX_JSON_OUT - 512];
    transazioni_to_json(c->transazioni, tx_json, sizeof(tx_json));

    char data_json[MAX_JSON_OUT - 256];
    snprintf(data_json, sizeof(data_json),
             "{\"conto\":%s,\"transazioni\":%s}", conto_json, tx_json);

    json_ok(data_json, out, sizeof(out));
    puts(out);
}

void cmd_preleva(const char *json) {
    double importo_d = 0.0;
    char out[256];

    int id = verifica_credenziali(json);
    if (id < 0) return;

    if (!json_get_num(json, "importo", &importo_d)) {
        json_errore("importo mancante", out, sizeof(out));
        puts(out);
        return;
    }

    Conto *c = NULL;
    for (int i = 0; i < banca.n_conti; i++)
        if (banca.conti[i].id_utente == id && banca.conti[i].attivo) { c = &banca.conti[i]; break; }
    if (!c) {
        json_errore("nessun conto trovato", out, sizeof(out));
        puts(out);
        return;
    }

    int importo = (int)importo_d;
    if (!preleva(&banca, c->iban, importo)) {
        json_errore("fondi insufficienti", out, sizeof(out));
        puts(out);
        return;
    }

    salva_dati(&banca);

    char saldo_json[64];
    snprintf(saldo_json, sizeof(saldo_json), "{\"saldo\":%d}", c->saldo);
    json_ok(saldo_json, out, sizeof(out));
    puts(out);
}

void cmd_invia(const char *json) {
    char iban_dest[35];
    double importo_d = 0.0;
    char out[256];

    int id = verifica_credenziali(json);
    if (id < 0) return;

    if (!json_get_str(json, "iban_destinatario", iban_dest, sizeof(iban_dest)) ||
        !json_get_num(json, "importo", &importo_d)) {
        json_errore("parametri mancanti", out, sizeof(out));
        puts(out);
        return;
    }

    Conto *mittente = NULL;
    for (int i = 0; i < banca.n_conti; i++)
        if (banca.conti[i].id_utente == id && banca.conti[i].attivo) { mittente = &banca.conti[i]; break; }
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

    int importo = (int)importo_d;
    int esito = bonifico(&banca, mittente->iban, iban_dest, importo);

    if (esito == -2) { json_errore("fondi insufficienti", out, sizeof(out)); puts(out); return; }
    if (esito == -3) { json_errore("IBAN destinatario non trovato", out, sizeof(out)); puts(out); return; }
    if (esito < 0)   { json_errore("invio fallito", out, sizeof(out)); puts(out); return; }

    salva_dati(&banca);

    char saldo_json[64];
    snprintf(saldo_json, sizeof(saldo_json), "{\"saldo\":%d}", mittente->saldo);
    json_ok(saldo_json, out, sizeof(out));
    puts(out);
}

void cmd_cerca_utenti(const char *json) {
    char query[128];
    char out[MAX_JSON_OUT];

    int id_self = verifica_credenziali(json);
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

        Conto *c = NULL;
        for (int j = 0; j < banca.n_conti; j++)
            if (banca.conti[j].id_utente == u->id && banca.conti[j].attivo) { c = &banca.conti[j]; break; }
        if (!c) continue;

        char entry[256];
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
    char out[256];

    int id = verifica_credenziali(json);
    if (id < 0) return;

    for (int i = 0; i < banca.n_conti; i++) {
        if (banca.conti[i].id_utente == id)
            banca.conti[i].attivo = 0;
    }

    Utente *u = utente_cerca_id(&banca, id);
    u->attivo = 0;

    salva_dati(&banca);
    salva_utenti(&banca);

    json_ok("", out, sizeof(out));
    puts(out);
}

// ENTRYPOINT
int main() {
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

    return 0;
}
