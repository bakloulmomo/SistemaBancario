#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>

/*
 * AES-128 ECB — cifratura/decifratura di un singolo blocco (16 byte).
 * Usato per cifrare le password prima di salvarle su CSV.
 */
void aes128_encrypt(const uint8_t key[16],
                    const uint8_t plaintext[16],
                    uint8_t       ciphertext[16]);

void aes128_decrypt(const uint8_t key[16],
                    const uint8_t ciphertext[16],
                    uint8_t       plaintext[16]);

/*
 * Cifra una stringa di password (max 32 char) con AES-128 ECB
 * e restituisce la rappresentazione esadecimale (max 65 char).
 */
void password_encrypt(const char *password, char *hex_out);

/*
 * Decifra un hash esadecimale e restituisce la password originale.
 * Usato internamente per la verifica al login.
 */
void password_decrypt(const char *hex_in, char *password_out);

/*
 * Confronta una password in chiaro con il suo hash salvato.
 * Ritorna 1 se corrispondono, 0 altrimenti.
 */
int password_verifica(const char *password, const char *hex_salvato);

/* Converte bytes in stringa esadecimale */
void bytes_to_hex(const uint8_t *bytes, int len, char *hex_out);

/* Converte stringa esadecimale in bytes */
void hex_to_bytes(const char *hex, uint8_t *bytes_out, int len);

#endif /* CRYPTO_H */
