#ifndef TIPI_H
#define TIPI_H

#include <stdint.h>

// Costanti globali
#define MAX_INPUT               4096
#define MAX_JSON_OUT            65536
#define BANCA_ABI               "01234"
#define BANCA_CAB               "56789"
#define AES_KEY                 "BancaSegreta_K16"  // 16 byte esatti
#define AES_IV                  "BancaIV_16bytes!"  // 16 byte esatti

// Tipo della transazione
typedef enum {
    TX_DEPOSITO = 0,
    TX_PRELIEVO,
    TX_BONIFICO_OUT,
    TX_BONIFICO_IN
} TipoTransazione;

// Struct

// Nodo lista transazioni di un conto
typedef struct Transazione {
    int             id;
    TipoTransazione tipo;
    int             importo;
    char            iban_controparte[35];
    struct Transazione *next;
} Transazione;

// Conto bancario (contiene anche la testa della lista transazioni)
typedef struct {
    int         id;
    char        iban[35];
    int         id_utente;
    int         saldo;
    int         attivo;
    Transazione *transazioni;           // testa lista collegata
} Conto;

// Utente
typedef struct {
    int  id;
    char username[64];
    char password_hex[96];  // AES-128, hex-encoded (max 2 blocchi)
    char nome[64];
    char cognome[64];
    int  attivo;
} Utente;

// Stato globale della banca
typedef struct {
    // Array dinamici
    Utente *utenti;
    int     n_utenti;
    int     cap_utenti;

    Conto  *conti;
    int     n_conti;
    int     cap_conti;

    // Contatori ID auto-increment
    int prossimo_id_utente;
    int prossimo_id_conto;
    int prossimo_id_transazione;
} StatoBanca;

#endif // TIPI_H
