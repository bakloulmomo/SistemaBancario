#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/utenti.h"
#include "../include/crypto.h"
#include "../include/utils.h"

#define CAP_INIZIALE 16

// array di utenti del programma

// inizializzazione utenti
void utenti_init(StatoBanca *banca) {
    banca->utenti     = (Utente *)malloc(CAP_INIZIALE * sizeof(Utente));
    banca->n_utenti   = 0;
    banca->cap_utenti = CAP_INIZIALE;
}

// libera a fine programma
void utenti_libera(StatoBanca *banca) {
    free(banca->utenti);
    banca->utenti     = NULL;
    banca->n_utenti   = 0;
    banca->cap_utenti = 0;
}

// aggiunge un utente all'array
int utente_aggiungi(StatoBanca *banca,
                    const char *username, const char *password,
                    const char *nome,     const char *cognome) {

    // se username gia esistente, torna -1
    if (utente_cerca_username(banca, username)) return -1;

    //n_utenti = quanti utenti ci sono attualmente
    //cap_utenti = massimo di utenti che possono registrarsi

    if (banca->n_utenti >= banca->cap_utenti) {
        // se si supera il massimo di utenti che possono registrasi
        // si realloca il max di utenti possibili
        banca->cap_utenti *= 2;
        banca->utenti = (Utente *)realloc(banca->utenti,
                            banca->cap_utenti * sizeof(Utente));
        if (!banca->utenti) return -1;
    }

    // ultimo el. degli utenti vuoto gia allocato
    Utente *u = &banca->utenti[banca->n_utenti];
    // si azzera per poterlo poi riempire *
    memset(u, 0, sizeof(Utente));

    // * qua
    u->id = banca->prossimo_id_utente++;
    // id univoco per l'utente
    strncpy(u->username, username, 63);
    // copia username in u->username
    strncpy(u->nome,     nome,     63);
    strncpy(u->cognome,  cognome,  63);
    u->attivo = 1;
    //attivo = 1, (per gestire anche l'eliminazione dell'account, in quel caso attivo = 0)

    //cripta password
    password_encrypt(password, u->password_hex);

    // aggiunge 1 elemento per prossimo utente
    banca->n_utenti++;
    return u->id;
}

// cerca username, usato per creare un username non esistente, e per effettuare login
Utente *utente_cerca_username(StatoBanca *banca, const char *username) {
    for (int i = 0; i < banca->n_utenti; i++) {
        if (banca->utenti[i].attivo && strcmp(banca->utenti[i].username, username) == 0)
            return &banca->utenti[i];
    }
    // se non trova
    return NULL;
}

// cerca l'utente tramite il suo id, usato per operazioni su account
Utente *utente_cerca_id(StatoBanca *banca, int id) {
    for (int i = 0; i < banca->n_utenti; i++) {
        if (banca->utenti[i].id == id && banca->utenti[i].attivo)
            return &banca->utenti[i];
    }
    return NULL;
}

// operazione login utente
int utente_login(StatoBanca *banca, const char *username, const char *password) {
    Utente *u = utente_cerca_username(banca, username);
    // se non trova utente
    if (!u) return -1;

    // verifica password
    if (!password_verifica(password, u->password_hex)) return -1;

    return u->id;
}

// aggiorna profilo (nome, cognome)
int utente_aggiorna_profilo(StatoBanca *banca, int id_utente,
                             const char *nome, const char *cognome) {
    Utente *u = utente_cerca_id(banca, id_utente);
    if (!u) return 0;

    if (nome    && nome[0])    strncpy(u->nome,    nome,    63);
    if (cognome && cognome[0]) strncpy(u->cognome, cognome, 63);
    return 1;
}

// cambia password, qui diverso
int utente_cambia_password(StatoBanca *banca, int id_utente, const char *nuova_pw) {
    Utente *u = utente_cerca_id(banca, id_utente);
    if (!u) return 0;

    // e criptarla
    password_encrypt(nuova_pw, u->password_hex);
    return 1;
}


// out = il buffer
// outsize = DIM_MAX di out, per evitare overflow

// si passa buf[4096] e la si passa alla funzione,
// snprintf riempa il buf col JSON, cosi da mandare via socket

void utente_to_json(const Utente *u, char *out, int outsize) {
    snprintf(out, outsize,
        "{"
        "\"id\":%d,"
        "\"username\":\"%s\","
        "\"nome\":\"%s\","
        "\"cognome\":\"%s\""
        "}",
        u->id, u->username, u->nome, u->cognome);
}
