#ifndef TIPI_H
#define TIPI_H

#include <stdint.h>

/* ---- Costanti globali ---- */
#define MAX_SESSIONI            100
#define MAX_INPUT               4096
#define MAX_JSON_OUT            65536
#define SESSION_DURATA_SEC      3600    /* 1 ora */
#define BANCA_ABI               "01234"
#define BANCA_CAB               "56789"
#define AES_KEY                 "BancaSegreta_K16"  /* 16 byte esatti */

/* ---- Enumerazioni ---- */

typedef enum {
    CONTO_CORRENTE = 0,
    CONTO_RISPARMIO
} TipoConto;

typedef enum {
    TX_DEPOSITO = 0,
    TX_PRELIEVO,
    TX_BONIFICO_OUT,
    TX_BONIFICO_IN
} TipoTransazione;

/* ---- Strutture ---- */

/* Nodo lista collegata per le transazioni di un conto */
typedef struct Transazione {
    int             id;
    TipoTransazione tipo;
    double          importo;
    char            descrizione[128];
    char            data[20];           /* "YYYY-MM-DD HH:MM:SS" */
    char            iban_controparte[35];
    struct Transazione *next;
} Transazione;

/* Conto bancario — contiene testa della lista transazioni */
typedef struct {
    int         id;
    char        iban[35];
    int         id_utente;
    double      saldo;
    TipoConto   tipo;
    char        data_apertura[20];
    int         attivo;
    Transazione *transazioni;           /* testa lista collegata */
} Conto;

/* Utente registrato */
typedef struct {
    int  id;
    char username[64];
    char password_hex[96];  /* AES-128 ECB, hex-encoded (max 2 blocchi) */
    char nome[64];
    char cognome[64];
    char email[128];
    char telefono[20];
    char data_nascita[12];
    char data_registrazione[20];
    int  attivo;
} Utente;

/* Sessione attiva — array di dimensione fissa MAX_SESSIONI */
typedef struct {
    char token[65];     /* 64 hex char + null */
    int  id_utente;
    long scadenza;      /* time_t */
} Sessione;

/* Nodo coda FIFO per notifiche/operazioni pendenti */
typedef struct NodoCoda {
    char            messaggio[256];
    int             id_utente;
    struct NodoCoda *next;
} NodoCoda;

/* Coda FIFO delle notifiche */
typedef struct {
    NodoCoda *testa;
    NodoCoda *coda;
    int       n;
} CodaNotifiche;

/* ---- Stato globale della banca ---- */
typedef struct {
    /* Array dinamici */
    Utente *utenti;
    int     n_utenti;
    int     cap_utenti;

    Conto  *conti;
    int     n_conti;
    int     cap_conti;

    /* Sessioni attive (array fisso) */
    Sessione sessioni[MAX_SESSIONI];
    int      n_sessioni;

    /* Contatori ID auto-increment */
    int prossimo_id_utente;
    int prossimo_id_conto;
    int prossimo_id_transazione;

    /* Coda notifiche pendenti */
    CodaNotifiche notifiche;
} StatoBanca;

#endif /* TIPI_H */
