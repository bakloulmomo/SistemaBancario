# Documentazione — Sistema Bancario

---

## 1. Architettura e Processi

```
Browser (porta 5173)          Node.js server (porta 3000)          banca_core (processo C)
         │                              │                                    │
         └──── HTTP /api/* ────────────►│                                    │
                                        └──── spawn() + JSON stdin ─────────►│
                                                                              │ legge CSV
                                                                              │ elabora
                                                                              │◄─ JSON stdout
                                        ◄──── risposta JSON ─────────────────┘
         ◄──── risposta HTTP ───────────┘
```

### Processo 1 — Avvio server
```bash
cd server && node server.js
```
- Express si mette in ascolto su porta 3000
- Monta routes: `/api/auth`, `/api/conti`, `/api/transazioni`, `/api/utenti`, `/api/chatbot`
- Nessun processo C viene avviato — il core parte solo on-demand

### Processo 2 — Avvio client
```bash
cd client && npm run dev
```
- Nuxt 3 compila Vue → serve su porta 5173
- Vite proxy: ogni `/api/*` viene inoltrato a `localhost:3000`
- Il browser non parla mai direttamente col server Node — passa dal proxy Vite

### Processo 3 — Apertura sito (non loggato)
1. Browser carica `app.vue` → `NuxtLayout` + `NuxtPage`
2. `plugins/auth.client.ts` legge `localStorage` → nessun token trovato
3. `middleware/auth.ts` scatta → `navigateTo('/auth')`
4. Viene mostrata `pages/auth.vue` con layout `auth.vue`
5. Nessuna chiamata al server ancora

### Processo 4 — Registrazione
```
Browser                    Node.js              banca_core
POST /api/auth/registra ──►routes/auth.js
  {username, nome,          eseguiCore({        spawn banca_core
   cognome, eta,             cmd:"registra",  ──►carica_dati() → legge 4 CSV
   password,                 username,...})      utente_aggiungi() → check username
   saldo_iniziale}                               conto_apri() → genera IBAN
                                                 deposita() → aggiunge tx
                                                 utente_login() → crea token
                                                 salva_dati() → riscrive 4 CSV
                                                 puts(JSON) → stdout
                            JSON.parse(stdout)◄──processo termina
◄── {token, username,       res.json()
     iban, saldo, utente}
```
Client riceve risposta:
- `setAuth(token, utente)` → salva in `useState` + `localStorage`
- Mostra step 2: "Il tuo username è X, IBAN: Y"
- Click → `navigateTo('/dashboard')`

### Processo 5 — Login
Identico alla registrazione ma `cmd:"login"`:
- C core verifica `password_verifica()` (AES-128-CBC decrypt + confronto)
- Crea sessione token (64 char hex random)
- Salva in `sessioni.csv`
- Ritorna `{token, iban, saldo, utente}`

### Processo 6 — Caricamento Dashboard
1. `middleware/auth.ts` → token in stato → passa
2. `layouts/default.vue` monta la sidebar
3. `pages/dashboard/index.vue` → `onMounted()`:
   - `caricaConti()` → `GET /api/conti` → spawn core → `cmd:"lista_conti"`
   - `caricaTransazioni(iban)` → `GET /api/conti/:iban/estratto` → spawn core → `cmd:"estratto_conto"`
4. Due processi C vengono spawnati in sequenza

### Processo 7 — Invia Soldi
```
Click "Invia" →
  Ricerca utente (debounce 350ms) →
    GET /api/utenti/cerca?q=mario →
      spawn core → cmd:"cerca_utenti" →
        scorre banca.utenti[] → match nome/cognome/username →
        per ogni match trova conto primario →
        ritorna [{nome, cognome, iban}]

Seleziona destinatario, inserisce importo →
  POST /api/transazioni/invia →
    spawn core → cmd:"invia" →
      trova conto mittente (auto dal token)
      trova conto destinatario (per IBAN)
      sottrae saldo mittente
      aggiunge saldo destinatario
      aggiunge TX_BONIFICO_OUT al mittente
      aggiunge TX_BONIFICO_IN al destinatario
      salva_dati()
      ritorna {saldo: nuovoSaldo}
```

### Processo 8 — Preleva / Spendi
```
POST /api/transazioni/preleva {importo, descrizione}
  → spawn core → cmd:"preleva"
  → trova conto primario dal token
  → saldo >= importo? sì → sottrae
  → aggiunge TX_PRELIEVO
  → salva_dati()
  → ritorna {saldo}
```

### Processo 9 — Modifica profilo
```
PUT /api/utenti/profilo {nome, cognome}
  → spawn core → cmd:"aggiorna_profilo"
  → trova utente dal token
  → aggiorna nome/cognome in RAM
  → salva_utenti()
```

### Processo 10 — Cambia password
```
PUT /api/utenti/password {vecchia_password, nuova_password}
  → spawn core → cmd:"cambia_password"
  → verifica vecchia password (decrypt AES + confronto)
  → se ok → encrypt nuova → sovrascrive password_hex
  → salva_utenti()
```

### Processo 11 — Chatbot
```
POST /api/chatbot/messaggio {messaggio}
  → chatbot/routes.js → chatbot/gemini.js
  → HTTPS request diretto a Google Gemini API
  → NO processo C, NO CSV
  → ritorna {risposta: "..."}
```

### Processo 12 — Elimina Account
```
DELETE /api/utenti/account {password}
  → spawn core → cmd:"elimina_account"
  → verifica password
  → utente.attivo = 0
  → tutti i conti dell'utente.attivo = 0
  → sessioni_rimuovi_utente()
  → salva tutto
Client: clearAuth() → localStorage pulito → redirect /auth
```

### Processo 13 — Logout
```
POST /api/auth/logout (header: Bearer <token>)
  → spawn core → cmd:"logout"
  → sessione_rimuovi(token)
  → salva_sessioni()
Client: clearAuth() → redirect /auth
```

### File su disco (stato persistente)
```
core/data/
  utenti.csv       → id, username, password_hex, nome, cognome, email, data_nascita, attivo
  conti.csv        → id, iban, id_utente, saldo, tipo, data_apertura, attivo
  transazioni.csv  → id, id_conto, tipo, importo, descrizione, data, iban_controparte
  sessioni.csv     → token, id_utente, scadenza (timestamp Unix)
```
Le sessioni scadono dopo 1 ora. I dati utente rimangono per sempre finché non si elimina l'account.

---

## 2. Cosa può essere migliorato

### Sicurezza

**AES key hardcoded nel sorgente**
`AES_KEY` e `AES_IV` sono in `tipi.h` — chiunque legga il codice conosce la chiave. In produzione andrebbero in variabili d'ambiente o un key management system (AWS KMS, HashiCorp Vault).

**Token generato con `rand()`**
`rand()` non è crittograficamente sicuro. In produzione si usa `/dev/urandom` o `RAND_bytes()` di OpenSSL per generare token di sessione.

**Nessun HTTPS**
Tutto viaggia in HTTP — token e password visibili in chiaro su rete. In produzione: TLS/HTTPS con certificato.

**Nessun rate limiting**
Nessun limite di tentativi di login — vulnerabile a brute force sulla password. Fix: blocco IP dopo N tentativi falliti.

**Password hash debole**
Le password sono cifrate con AES (cifratura reversibile) invece di un hash one-way come **bcrypt** o **Argon2**. Se qualcuno ruba il file CSV e conosce la chiave AES, recupera tutte le password in chiaro.

**CSV Injection**
I dati vengono scritti in CSV senza sanitizzazione completa — un nome come `","` potrebbe corrompere il file.

### Architettura / Programmazione

**Un processo C per ogni richiesta**
Fork + lettura CSV da disco ad ogni HTTP request. In produzione: processo C persistente in memoria (socket o pipe), oppure riscrivere in un linguaggio con server nativo (Go, Rust, Java).

**CSV come database**
Nessuna transazione atomica — se il server crasha durante `salva_dati()` i file rimangono in stato inconsistente. In produzione: SQLite minimo, o PostgreSQL.

**Nessuna validazione input lato server**
Importi negativi, stringhe enormi, caratteri speciali non vengono tutti controllati prima di arrivare al core C.

**Buffer fissi nel core C**
Array `char username[64]` — se l'input supera 64 byte, `strncpy` tronca silenziosamente. Gestione errori migliorabile.

**Sessioni non condivise tra invocazioni**
Le sessioni vengono salvate su file ma ricaricate solo all'avvio del processo C. Funziona solo perché ogni processo ricarica tutto da zero.

---

## 3. DSA e Algoritmi usati nel core C

### Strutture Dati

**Array dinamico (Dynamic Array / Vector)**
```c
// utenti.c, conti.c
banca->utenti = malloc(CAP_INIZIALE * sizeof(Utente));
// quando pieno:
banca->cap_utenti *= 2;
banca->utenti = realloc(banca->utenti, banca->cap_utenti * sizeof(Utente));
```
Stessa idea di `std::vector` in C++. Capacità raddoppia ogni volta → ammortizzato O(1) per inserimento.

**Lista collegata (Linked List)**
```c
// tipi.h — transazioni di ogni conto
typedef struct Transazione {
    ...
    struct Transazione *next;   // puntatore al nodo successivo
} Transazione;
```
Ogni conto ha una lista collegata di transazioni. Inserimento in testa O(1).

**Array fisso con swap-and-shrink**
```c
// utils.c — rimozione sessione
banca->sessioni[i] = banca->sessioni[--banca->n_sessioni];
```
Invece di shiftare tutti gli elementi, copia l'ultimo al posto di quello rimosso. Rimozione O(1) invece di O(n).

**Coda FIFO (Queue)**
```c
// utils.c — notifiche
void coda_enqueue(CodaNotifiche *coda, ...)  // aggiunge in coda
NodoCoda *coda_dequeue(CodaNotifiche *coda)  // rimuove dalla testa
```
Implementata con lista collegata con puntatori a testa e coda.

### Algoritmi

**Ricerca lineare**
```c
// utenti.c — cerca username
for (int i = 0; i < banca->n_utenti; i++) {
    if (strcmp(banca->utenti[i].username, username) == 0)
        return &banca->utenti[i];
}
```
O(n) — accettabile per progetto scolastico. In produzione: hash table O(1).

**Ricerca case-insensitive (substring search)**
```c
// utils.c — cerca_utenti
for (int i = 0; haystack[i]; i++)
    for (int j = 0; j < nlen; j++)
        tolower(haystack[i+j]) != tolower(needle[j])
```
Naive string matching O(n×m). Alternativa professionale: KMP O(n+m).

**AES-128-CBC (cifratura a blocchi)**
```c
// crypto.c — via OpenSSL EVP
EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);
EVP_EncryptUpdate(...);
EVP_EncryptFinal_ex(...);  // aggiunge PKCS7 padding
```
Algoritmo a blocchi da 16 byte, modalità CBC con IV per evitare pattern identici.

**Generazione token pseudocasuale**
```c
// utils.c
for (int i = 0; i < 64; i++)
    token_out[i] = "0123456789abcdef"[rand() % 16];
```
Token da 256 bit. `rand()` non crittograficamente sicuro — migliorabile con `RAND_bytes()` di OpenSSL.

### Riepilogo

| DSA | Dove | Complessità |
|---|---|---|
| Array dinamico | utenti, conti | insert O(1) ammortizzato |
| Lista collegata | transazioni per conto | insert O(1) |
| Array fisso swap-and-shrink | sessioni | remove O(1) |
| Coda FIFO | notifiche | enqueue/dequeue O(1) |
| Ricerca lineare | tutti i lookup | O(n) |
| Substring search case-insensitive | cerca_utenti | O(n×m) |
| AES-128-CBC | cifratura password | O(1) blocchi fissi |
| Generazione token pseudocasuale | login/registra | O(1) |
