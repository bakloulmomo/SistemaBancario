#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>

/* Cifra password (max 31 char) con AES-128-CBC via OpenSSL.
 * Output: hex string, max 96 char (48 byte cifrati). */
void password_encrypt(const char *password, char *hex_out);

/* Decifra hex → password in chiaro. Usato internamente per verifica. */
void password_decrypt(const char *hex_in, char *password_out);

/* Ritorna 1 se password corrisponde all'hex salvato, 0 altrimenti. */
int password_verifica(const char *password, const char *hex_salvato);

/* Converte bytes in stringa esadecimale */
void bytes_to_hex(const uint8_t *bytes, int len, char *hex_out);

/* Converte stringa esadecimale in bytes */
void hex_to_bytes(const char *hex, uint8_t *bytes_out, int len);

#endif /* CRYPTO_H */
