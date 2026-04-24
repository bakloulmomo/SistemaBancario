#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/utenti.h"
#include "../include/crypto.h"
#include "../include/utils.h"

#define CAP_INIZIALE 16

void utenti_init(StatoBanca *banca) {
    banca->utenti     = (Utente *)malloc(CAP_INIZIALE * sizeof(Utente));
    banca->n_utenti   = 0;
    banca->cap_utenti = CAP_INIZIALE;
}

void utenti_libera(StatoBanca *banca) {
    free(banca->utenti);
    banca->utenti     = NULL;
    banca->n_utenti   = 0;
    banca->cap_utenti = 0;
}

int utente_aggiungi(StatoBanca *banca,
                    const char *username, const char *password,
                    const char *nome,     const char *cognome,
                    const char *email,    const char *telefono,
                    const char *data_nascita) {
    /* Username già esistente? */
    if (utente_cerca_username(banca, username)) return -1;

    /* Ridimensiona se necessario */
    if (banca->n_utenti >= banca->cap_utenti) {
        banca->cap_utenti *= 2;
        banca->utenti = (Utente *)realloc(banca->utenti,
                            banca->cap_utenti * sizeof(Utente));
        if (!banca->utenti) return -1;
    }

    Utente *u = &banca->utenti[banca->n_utenti];
    memset(u, 0, sizeof(Utente));

    u->id = banca->prossimo_id_utente++;
    strncpy(u->username,     username,    63);
    strncpy(u->nome,         nome,        63);
    strncpy(u->cognome,      cognome,     63);
    strncpy(u->email,        email,       127);
    strncpy(u->telefono,     telefono,    19);
    strncpy(u->data_nascita, data_nascita,11);
    u->attivo = 1;

    password_encrypt(password, u->password_hex);
    data_ora_corrente(u->data_registrazione);

    banca->n_utenti++;
    return u->id;
}

Utente *utente_cerca_username(StatoBanca *banca, const char *username) {
    for (int i = 0; i < banca->n_utenti; i++) {
        if (banca->utenti[i].attivo &&
            strcmp(banca->utenti[i].username, username) == 0)
            return &banca->utenti[i];
    }
    return NULL;
}

Utente *utente_cerca_id(StatoBanca *banca, int id) {
    for (int i = 0; i < banca->n_utenti; i++) {
        if (banca->utenti[i].id == id && banca->utenti[i].attivo)
            return &banca->utenti[i];
    }
    return NULL;
}

int utente_login(StatoBanca *banca,
                 const char *username, const char *password,
                 char *token_out) {
    Utente *u = utente_cerca_username(banca, username);
    if (!u) return -1;

    if (!password_verifica(password, u->password_hex)) return -1;

    if (!sessione_crea(banca, u->id, token_out)) return -1;

    return u->id;
}

void utente_logout(StatoBanca *banca, const char *token) {
    sessione_rimuovi(banca, token);
}

int utente_aggiorna_profilo(StatoBanca *banca, int id_utente,
                             const char *nome,    const char *cognome,
                             const char *email,   const char *telefono) {
    Utente *u = utente_cerca_id(banca, id_utente);
    if (!u) return 0;

    if (nome     && nome[0])     strncpy(u->nome,     nome,     63);
    if (cognome  && cognome[0])  strncpy(u->cognome,  cognome,  63);
    if (email    && email[0])    strncpy(u->email,    email,    127);
    if (telefono && telefono[0]) strncpy(u->telefono, telefono, 19);

    return 1;
}

int utente_cambia_password(StatoBanca *banca, int id_utente,
                            const char *vecchia_pw, const char *nuova_pw) {
    Utente *u = utente_cerca_id(banca, id_utente);
    if (!u) return 0;

    if (!password_verifica(vecchia_pw, u->password_hex)) return 0;

    password_encrypt(nuova_pw, u->password_hex);
    return 1;
}

int utente_disattiva(StatoBanca *banca, int id_utente) {
    Utente *u = utente_cerca_id(banca, id_utente);
    if (!u) return 0;
    u->attivo = 0;
    return 1;
}

void utente_to_json(const Utente *u, char *out, int outsize) {
    snprintf(out, outsize,
        "{"
        "\"id\":%d,"
        "\"username\":\"%s\","
        "\"nome\":\"%s\","
        "\"cognome\":\"%s\","
        "\"email\":\"%s\","
        "\"telefono\":\"%s\","
        "\"data_nascita\":\"%s\","
        "\"data_registrazione\":\"%s\""
        "}",
        u->id, u->username, u->nome, u->cognome,
        u->email, u->telefono, u->data_nascita, u->data_registrazione);
}
