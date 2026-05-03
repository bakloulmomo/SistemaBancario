#include <string.h>
#include <stdio.h>
#include <openssl/evp.h> //      <-----------
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

// cripta la password (AES128 CBC)
void password_encrypt(const char *password, char *hex_out) {
    const uint8_t *key = (const uint8_t *)AES_KEY; 
    const uint8_t *iv  = (const uint8_t *)AES_IV;
    uint8_t cipher[48]; // array iniziale di byte , poi si chiama bytes_to_hex() per riconvertirlo in char
    // usato questo procedimento per evitare che, visto che AES puo generare in stringa "0"
    // il compilatore puo capire che finisce la stinga, la interpreta come /0
    // es. AES genera: [ 65, 102, 23, 19, 0, 88, 214, ... ], diventa poi stringa 
    // il comp. fa finire la stringa dopo lo 0. Cosi in esadecimale 0 -> "00"

    int out_len = 0, final_len = 0;  
    // out_len = byte alla volta da cryptare 
    // final_len = ultimi byte rimasti < 16


    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    // qua sta calcolando le sottochiavi necessarie per i 10 giri
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv); 
    // qua avviene la speparazione (16 byte) e per ogni blocco, OpenSSL fa partire i 10 giri
    EVP_EncryptUpdate(ctx, cipher, &out_len,(const uint8_t *)password, (int)strlen(password)); 
    // caso di byte incompleti (es. password = 20 byte) aggiunge ultimi byte rimasti e fa partire ultimi 10 giri
    EVP_EncryptFinal_ex(ctx, cipher + out_len, &final_len);
    // libero la memoria del context
    EVP_CIPHER_CTX_free(ctx);

    // mando l'array di byte per renderlo stringa da mettere in CSV
    bytes_to_hex(cipher, out_len + final_len, hex_out);
}

// cifra la password per verificare se la password è giusta, processo inverso
void password_decrypt(const char *hex_in, char *password_out) {
    const uint8_t *key = (const uint8_t *)AES_KEY;
    const uint8_t *iv  = (const uint8_t *)AES_IV;
    int cipher_len = (int)strlen(hex_in) / 2; // len_array, ogni byte = 2 char 
    uint8_t cipher[48];
    uint8_t plain[48] = {0}; 
    int out_len = 0, final_len = 0; // stessi param dell encrypt

    hex_to_bytes(hex_in, cipher, cipher_len);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();  // alloca      
    EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv); // configura
    EVP_DecryptUpdate(ctx, plain, &out_len, cipher, cipher_len); // decifra dati 
    if (EVP_DecryptFinal_ex(ctx, plain + out_len, &final_len) <= 0)
        final_len = 0;
    EVP_CIPHER_CTX_free(ctx); // libera 

    strncpy(password_out, (char *)plain, out_len + final_len);
    password_out[out_len + final_len] = '\0';
}

// verifica la password per l'autenticazione dell'utente
int password_verifica(const char *password, const char *hex_salvato) {
    char decifrata[48] = {0};
    password_decrypt(hex_salvato, decifrata);
    return strcmp(password, decifrata) == 0 ? 1 : 0;
}
