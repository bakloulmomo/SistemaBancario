#include <string.h>
#include <stdio.h>
#include <openssl/evp.h> ///
#include "../include/crypto.h"
#include "../include/tipi.h"

// gestisce cifratura/crittografia per tutte le altre funzioni

// bytes = array di byte cifrati (output di AES)
// len = byte da convertire
// hex_out = stringa che viene in input da criptare
// puntatore alla posizione del buffer dove scrivere

// Dopo aver cifrato password, salva i byte in stringa
void bytes_to_hex(const uint8_t *bytes, int len, char *hex_out) {
    for (int i = 0; i < len; i++)
        sprintf(hex_out + i * 2, "%02x", bytes[i]); // ogni byte = 2 char
    // 02x, x = esadecimale minuscolo, 02 = minimo 2 cifre (es. 9 salva "09" non "9")
    hex_out[len * 2] = '\0'; // qua l '/0' lo si mette manualmente perche il for lo sovrascive dopo
    //sprintf scrive la stringa formattata nel buffer
}

// da stringa a bytes
void hex_to_bytes(const char *hex, uint8_t *bytes_out, int len) {
    for (int i = 0; i < len; i++) {
        unsigned int val;
        sscanf(hex + i * 2, "%02x", &val); //sscanf legge stringa (cioe esadecimali)
        bytes_out[i] = (uint8_t)val; // li mettiamo nel buffer per essere pronti per la cifratura
    }
}

// cripta la password 
void password_encrypt(const char *password, char *hex_out) {
    const uint8_t *key = (const uint8_t *)AES_KEY;
    const uint8_t *iv  = (const uint8_t *)AES_IV;
    uint8_t cipher[48];

    int out_len = 0, final_len = 0;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);
    EVP_EncryptUpdate(ctx, cipher, &out_len,
                      (const uint8_t *)password, (int)strlen(password));
    EVP_EncryptFinal_ex(ctx, cipher + out_len, &final_len);
    EVP_CIPHER_CTX_free(ctx);

    bytes_to_hex(cipher, out_len + final_len, hex_out);
}

// cifra la password per verificare se la password è giusta
void password_decrypt(const char *hex_in, char *password_out) {
    const uint8_t *key = (const uint8_t *)AES_KEY;
    const uint8_t *iv  = (const uint8_t *)AES_IV;
    int cipher_len = (int)strlen(hex_in) / 2;
    uint8_t cipher[48];
    uint8_t plain[48] = {0};
    int out_len = 0, final_len = 0;

    hex_to_bytes(hex_in, cipher, cipher_len);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);
    EVP_DecryptUpdate(ctx, plain, &out_len, cipher, cipher_len);
    EVP_DecryptFinal_ex(ctx, plain + out_len, &final_len);
    EVP_CIPHER_CTX_free(ctx);

    strncpy(password_out, (char *)plain, out_len + final_len);
    password_out[out_len + final_len] = '\0';
}

// verifica la password per l'autenticazione dell'utente
int password_verifica(const char *password, const char *hex_salvato) {
    char decifrata[48] = {0};
    password_decrypt(hex_salvato, decifrata);
    return strcmp(password, decifrata) == 0 ? 1 : 0;
}

