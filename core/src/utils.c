#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/utils.h"
#include "../include/tipi.h"


void genera_iban(int id_conto, char *iban_out) {
    // Formato: IT60 + ABI(5) + CAB(5) + CC(12 cifre zero-padded)
    snprintf(iban_out, 35, "IT60%s%s%012d", BANCA_ABI, BANCA_CAB, id_conto);
}

void json_ok(const char *data_json, char *out, int outsize) {
    if (data_json && data_json[0] != '\0')
        snprintf(out, outsize, "{\"status\":\"ok\",\"data\":%s}", data_json);
    else
        snprintf(out, outsize, "{\"status\":\"ok\"}");
}

void json_errore(const char *messaggio, char *out, int outsize) {
    snprintf(out, outsize, "{\"status\":\"error\",\"message\":\"%s\"}", messaggio);
}

int json_get_str(const char *json, const char *chiave,
                 char *val_out, int val_size) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", chiave);

    const char *pos = strstr(json, pattern);
    if (!pos) return 0;

    pos += strlen(pattern);

    while (*pos && *pos != ':') pos++;
    if (!*pos) return 0;
    pos++;

    while (*pos && isspace((unsigned char)*pos)) pos++;

    if (*pos != '"') return 0;
    pos++;

    int i = 0;
    while (*pos && *pos != '"' && i < val_size - 1) {
        // Gestione escape JSON base
        if (*pos == '\\' && *(pos + 1)) {
            pos++;
            switch (*pos) {
                case 'n': val_out[i++] = '\n'; break;
                case 't': val_out[i++] = '\t'; break;
                case '"': val_out[i++] = '"';  break;
                case '\\':val_out[i++] = '\\'; break;
                default:  val_out[i++] = *pos; break;
            }
        } else {
            val_out[i++] = *pos;
        }
        pos++;
    }
    val_out[i] = '\0';
    return 1;
}

int json_get_num(const char *json, const char *chiave, double *val_out) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", chiave);

    const char *pos = strstr(json, pattern);
    if (!pos) return 0;

    pos += strlen(pattern);

    while (*pos && *pos != ':') pos++;
    if (!*pos) return 0;
    pos++;

    while (*pos && isspace((unsigned char)*pos)) pos++;

    if (*pos == '"') return 0;

    return sscanf(pos, "%lf", val_out) == 1 ? 1 : 0;
}

void str_trim(char *s) {
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
    char *start = s;
    while (*start && isspace((unsigned char)*start))
        start++;
    if (start != s)
        memmove(s, start, strlen(start) + 1);
}

// salva in sicuro stringa in CSV (ad esempio, quando un utente->nome = Marco, Rossi,
// toglie la virgola con \, )
void csv_escape(const char *in, char *out, int outsize) {
    int j = 0;
    for (int i = 0; in[i] && j < outsize - 2; i++) {
        if (in[i] == ',') {
            out[j++] = '\\';
            out[j++] = ',';
        } else if (in[i] == '\n') {
            out[j++] = '\\';
            out[j++] = 'n';
        } else {
            out[j++] = in[i];
        }
    }
    out[j] = '\0';
}

// operazione inversa, per riportare le virgole
void csv_unescape(const char *in, char *out, int outsize) {
    int j = 0;
    for (int i = 0; in[i] && j < outsize - 1; i++) {
        if (in[i] == '\\' && in[i + 1] == ',') {
            out[j++] = ',';
            i++;
        } else if (in[i] == '\\' && in[i + 1] == 'n') {
            out[j++] = '\n';
            i++;
        } else {
            out[j++] = in[i];
        }
    }
    out[j] = '\0';
}

// per cercare destinatario del bonifico
int str_contains_ci(const char *haystack, const char *needle) {
    if (!needle || !needle[0]) return 1;
    int nlen = (int)strlen(needle);
    for (int i = 0; haystack[i]; i++) {
        int j;
        for (j = 0; j < nlen && haystack[i+j]; j++) {
            if (tolower((unsigned char)haystack[i+j]) != tolower((unsigned char)needle[j]))
                break;
        }
        if (j == nlen) return 1;
    }
    return 0;
}
