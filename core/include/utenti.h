#ifndef UTENTI_H
#define UTENTI_H

#include "tipi.h"

// Inizializza array dinamico utenti
void utenti_init(StatoBanca *banca);

// Libera memoria array utenti
void utenti_libera(StatoBanca *banca);

// Aggiunge utente, ridimensiona array se necessario. Ritorna id o -1.
int utente_aggiungi(StatoBanca *banca,
                    const char *username, const char *password,
                    const char *nome,     const char *cognome);

// Cerca utente per username. Ritorna puntatore o NULL.
Utente *utente_cerca_username(StatoBanca *banca, const char *username);

// Cerca utente per ID. Ritorna puntatore o NULL.
Utente *utente_cerca_id(StatoBanca *banca, int id);

// Login: verifica credenziali. Ritorna id_utente o -1.
int utente_login(StatoBanca *banca, const char *username, const char *password);

// Modifica profilo utente. Ritorna 1 ok / 0 errore.
int utente_aggiorna_profilo(StatoBanca *banca, int id_utente,
                             const char *nome, const char *cognome);

// Cambia password. Ritorna 1 ok / 0 errore.
int utente_cambia_password(StatoBanca *banca, int id_utente,
                            const char *nuova_pw);

// Serializza profilo utente come JSON object string.
void utente_to_json(const Utente *u, char *out, int outsize);

#endif // UTENTI_H
