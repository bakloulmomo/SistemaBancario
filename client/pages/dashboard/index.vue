<template>
  <div class="page fade-up">
    <!-- Header -->
    <div class="page-header">
      <div>
        <h1 class="page-title">Ciao, {{ utente?.nome }} 👋</h1>
        <p class="page-sub">{{ oggi }}</p>
      </div>
      <button class="btn-accent" @click="showInvia = true">+ Invia Soldi</button>
    </div>

    <!-- Balance + IBAN -->
    <div class="grid-2" style="margin-bottom:1.5rem">
      <div class="card-accent">
        <p class="card-label-dark">Saldo disponibile</p>
        <p class="balance-num balance-glow">{{ formatEuro(saldoTotale) }}</p>
        <div style="margin-top:0.75rem;display:flex;align-items:center;gap:0.5rem">
          <span style="font-size:0.72rem;font-weight:600;color:var(--color-dark);opacity:0.6;text-transform:uppercase;letter-spacing:0.04em">IBAN</span>
          <span style="font-family:monospace;font-size:0.78rem;color:var(--color-dark);font-weight:600">{{ contoPrimario?.iban ?? '—' }}</span>
        </div>
      </div>
      <div class="card" style="display:flex;flex-direction:column;gap:1rem;justify-content:center">
        <button class="quick-action" @click="showPreleva = true">
          <span class="qa-icon">↑</span>
          <span>Preleva / Spendi</span>
        </button>
        <button class="quick-action" @click="showInvia = true">
          <span class="qa-icon" style="transform:rotate(-45deg)">→</span>
          <span>Invia a qualcuno</span>
        </button>
      </div>
    </div>

    <!-- Transactions -->
    <div class="card">
      <div class="section-header">
        <h2 class="section-title">Movimenti recenti</h2>
        <button class="btn-refresh" @click="aggiornaTransazioni">↻ Aggiorna</button>
      </div>

      <div v-if="transazioni.length === 0" class="empty-state">
        <p>Nessun movimento trovato.</p>
      </div>
      <table v-else class="cb-table">
        <thead>
          <tr>
            <th>Data</th>
            <th>Descrizione</th>
            <th>Controparte</th>
            <th>Importo</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="tx in transazioniOrdinati" :key="tx.id">
            <td class="cell-muted">{{ formatData(tx.data) }}</td>
            <td>{{ tx.descrizione || tipoLabel(tx.tipo) }}</td>
            <td class="cell-muted small">{{ (tx as any).iban_controparte || '—' }}</td>
            <td>
              <span :class="['badge', isEntrata(tx) ? 'badge-green' : 'badge-red']">
                {{ isEntrata(tx) ? '+' : '-' }}{{ formatEuro(tx.importo) }}
              </span>
            </td>
          </tr>
        </tbody>
      </table>
    </div>

    <!-- MODAL: Invia soldi -->
    <div v-if="showInvia" class="modal-overlay" @click.self="showInvia = false">
      <div class="modal-box card">
        <h3 class="modal-title">Invia Soldi</h3>

        <!-- Ricerca destinatario -->
        <div class="field">
          <label class="cb-label">Cerca destinatario</label>
          <div class="search-row">
            <input v-model="searchQuery" class="cb-input" placeholder="Nome o cognome…" @input="cercaUtenti" />
          </div>
        </div>

        <!-- Risultati ricerca -->
        <div v-if="risultatiRicerca.length > 0" class="search-results">
          <button
            v-for="u in risultatiRicerca" :key="u.id"
            :class="['search-result', { selected: destinatario?.id === u.id }]"
            type="button"
            @click="selezionaDestinatario(u)"
          >
            <div class="result-avatar">{{ (u.nome[0] ?? '') + (u.cognome[0] ?? '') }}</div>
            <div>
              <p class="result-name">{{ u.nome }} {{ u.cognome }}</p>
              <p class="result-iban">{{ u.iban }}</p>
            </div>
            <span v-if="destinatario?.id === u.id" class="result-check">✓</span>
          </button>
        </div>
        <p v-else-if="searchQuery && !cercandoUtenti" class="cell-muted" style="font-size:0.82rem">Nessun utente trovato.</p>

        <form v-if="destinatario" @submit.prevent="eseguiInvia">
          <div class="selected-dest">
            <span>→ {{ destinatario.nome }} {{ destinatario.cognome }}</span>
            <button type="button" class="clear-btn" @click="destinatario = null; searchQuery = ''">✕</button>
          </div>
          <div class="field" style="margin-top:0.75rem">
            <label class="cb-label">Importo (€)</label>
            <input v-model.number="inviaImporto" type="number" min="0.01" step="0.01" class="cb-input" placeholder="50.00" required />
          </div>
          <div class="field">
            <label class="cb-label">Causale</label>
            <input v-model="inviaDescrizione" class="cb-input" placeholder="Cena di ieri…" />
          </div>
          <p v-if="opErr" class="error-msg">{{ opErr }}</p>
          <div class="modal-actions">
            <button type="button" class="btn-ghost" @click="showInvia = false">Annulla</button>
            <button type="submit" class="btn-accent" :disabled="opLoading">
              {{ opLoading ? 'Invio…' : 'Invia' }}
            </button>
          </div>
        </form>
        <div v-else class="modal-actions">
          <button type="button" class="btn-ghost" @click="showInvia = false">Chiudi</button>
        </div>
      </div>
    </div>

    <!-- MODAL: Preleva -->
    <div v-if="showPreleva" class="modal-overlay" @click.self="showPreleva = false">
      <div class="modal-box card">
        <h3 class="modal-title">Preleva / Spendi</h3>
        <form @submit.prevent="eseguiPreleva">
          <div class="field">
            <label class="cb-label">Importo (€)</label>
            <input v-model.number="prelievoImporto" type="number" min="0.01" step="0.01" class="cb-input" placeholder="100.00" required />
          </div>
          <div class="field">
            <label class="cb-label">Descrizione</label>
            <input v-model="prelievoDescrizione" class="cb-input" placeholder="Spesa al supermercato…" />
          </div>
          <p v-if="opErr" class="error-msg">{{ opErr }}</p>
          <div class="modal-actions">
            <button type="button" class="btn-ghost" @click="showPreleva = false">Annulla</button>
            <button type="submit" class="btn-accent" :disabled="opLoading">
              {{ opLoading ? '…' : 'Conferma' }}
            </button>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
definePageMeta({ middleware: 'auth' })

const { utente } = useAuth()
const { get } = useApi()
const { conti, transazioni, saldoTotale, contoPrimario, caricaConti, caricaTransazioni, preleva, invia } = useBank()

const oggi = new Date().toLocaleDateString('it-IT', { weekday: 'long', day: 'numeric', month: 'long', year: 'numeric' })

const showInvia   = ref(false)
const showPreleva = ref(false)
const opLoading   = ref(false)
const opErr       = ref('')

const searchQuery      = ref('')
const cercandoUtenti   = ref(false)
const risultatiRicerca = ref<any[]>([])
const destinatario     = ref<any>(null)
const inviaImporto     = ref(0)
const inviaDescrizione = ref('')
const prelievoImporto     = ref(0)
const prelievoDescrizione = ref('')

const transazioniOrdinati = computed(() =>
  [...transazioni.value].sort((a, b) => new Date(b.data).getTime() - new Date(a.data).getTime())
)

function formatEuro(n: number) {
  return new Intl.NumberFormat('it-IT', { style: 'currency', currency: 'EUR' }).format(n)
}
function formatData(d: string) {
  return new Date(d).toLocaleDateString('it-IT', { day: '2-digit', month: 'short', year: '2-digit' })
}
function tipoLabel(tipo: string | number) {
  const map: Record<string, string> = { '0': 'Deposito', '1': 'Prelievo', '2': 'Bonifico uscita', '3': 'Bonifico entrata' }
  return map[String(tipo)] ?? tipo
}
function isEntrata(tx: any) {
  return tx.tipo === 3 || tx.tipo === '3' || tx.tipo === 0 || tx.tipo === '0'
}

let searchTimer: ReturnType<typeof setTimeout> | null = null
function cercaUtenti() {
  if (searchTimer) clearTimeout(searchTimer)
  if (!searchQuery.value.trim()) { risultatiRicerca.value = []; return }
  searchTimer = setTimeout(async () => {
    cercandoUtenti.value = true
    try {
      const r = await get<any>(`/api/utenti/cerca?q=${encodeURIComponent(searchQuery.value)}`)
      risultatiRicerca.value = r.data ?? []
    } catch {}
    cercandoUtenti.value = false
  }, 350)
}

function selezionaDestinatario(u: any) {
  destinatario.value = u
}

async function aggiornaTransazioni() {
  if (contoPrimario.value) await caricaTransazioni(contoPrimario.value.iban)
}

async function eseguiInvia() {
  if (!destinatario.value) return
  opLoading.value = true; opErr.value = ''
  try {
    await invia(destinatario.value.iban, inviaImporto.value, inviaDescrizione.value)
    showInvia.value = false
    destinatario.value = null; searchQuery.value = ''; risultatiRicerca.value = []
    inviaImporto.value = 0; inviaDescrizione.value = ''
  } catch (e: any) { opErr.value = e.message }
  finally { opLoading.value = false }
}

async function eseguiPreleva() {
  opLoading.value = true; opErr.value = ''
  try {
    await preleva(prelievoImporto.value, prelievoDescrizione.value)
    showPreleva.value = false; prelievoImporto.value = 0; prelievoDescrizione.value = ''
  } catch (e: any) { opErr.value = e.message }
  finally { opLoading.value = false }
}

onMounted(async () => {
  await caricaConti()
  if (contoPrimario.value) await caricaTransazioni(contoPrimario.value.iban)
})
</script>

<style scoped>
.page { display: flex; flex-direction: column; gap: 0; }
.page-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 1.5rem; }
.page-title { font-size: 1.6rem; font-weight: 800; margin: 0; }
.page-sub { font-size: 0.85rem; color: var(--color-gray-alt); margin: 0.25rem 0 0; text-transform: capitalize; }

.card-label-dark { font-size: 0.75rem; font-weight: 600; text-transform: uppercase; letter-spacing: 0.05em; color: var(--color-dark); opacity: 0.7; margin: 0; }
.balance-num { font-size: 2rem; font-weight: 800; color: var(--color-dark); margin: 0.25rem 0; }

.quick-action {
  display: flex; align-items: center; gap: 0.75rem;
  background: #ffffff08; border: 1.5px solid #ffffff12; border-radius: 0.75rem;
  color: var(--color-light); padding: 0.9rem 1rem; cursor: pointer; width: 100%;
  transition: background 0.2s, border-color 0.2s, transform 0.15s;
  font-size: 0.85rem; font-weight: 600;
}
.quick-action:hover { background: #ffffff12; border-color: var(--color-accent); transform: translateY(-2px); }
.qa-icon {
  width: 2.2rem; height: 2.2rem; border-radius: 50%; background: var(--color-accent);
  color: var(--color-dark); display: flex; align-items: center; justify-content: center;
  font-size: 1rem; font-weight: 800; flex-shrink: 0; transition: transform 0.2s;
}
.quick-action:hover .qa-icon { transform: scale(1.1); }

.section-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 1rem; }
.section-title { font-size: 1rem; font-weight: 700; margin: 0; }
.btn-refresh { background: none; border: none; color: var(--color-accent); font-size: 0.82rem; cursor: pointer; font-weight: 600; }
.empty-state { text-align: center; color: var(--color-gray-alt); padding: 2rem; }
.cell-muted { color: var(--color-gray-alt); font-size: 0.8rem; }
.small { font-size: 0.75rem; }

.modal-overlay {
  position: fixed; inset: 0; background: rgba(0,0,0,0.75);
  display: flex; align-items: center; justify-content: center; z-index: 100;
  animation: fadeUp 0.2s ease;
}
.modal-box { width: 100%; max-width: 440px; display: flex; flex-direction: column; gap: 1rem; max-height: 90vh; overflow-y: auto; }
.modal-title { font-size: 1.2rem; font-weight: 700; margin: 0; }
.modal-actions { display: flex; gap: 0.75rem; justify-content: flex-end; margin-top: 0.5rem; }
.field { display: flex; flex-direction: column; gap: 0.4rem; }
.error-msg { color: #ff7070; font-size: 0.85rem; margin: 0; }

.search-row { display: flex; gap: 0.5rem; }
.search-results { display: flex; flex-direction: column; gap: 0.25rem; max-height: 200px; overflow-y: auto; }
.search-result {
  display: flex; align-items: center; gap: 0.75rem;
  padding: 0.65rem 0.75rem; border-radius: 0.6rem; border: 1.5px solid #ffffff12;
  background: transparent; color: var(--color-light); cursor: pointer; width: 100%;
  text-align: left; transition: background 0.15s, border-color 0.15s;
}
.search-result:hover { background: #ffffff08; }
.search-result.selected { border-color: var(--color-accent); background: rgba(185,255,102,0.07); }
.result-avatar {
  width: 2rem; height: 2rem; border-radius: 50%; background: var(--color-accent);
  color: var(--color-dark); display: flex; align-items: center; justify-content: center;
  font-weight: 700; font-size: 0.75rem; flex-shrink: 0;
}
.result-name { font-size: 0.88rem; font-weight: 600; margin: 0; }
.result-iban { font-size: 0.72rem; font-family: monospace; color: var(--color-gray-alt); margin: 0; }
.result-check { margin-left: auto; color: var(--color-accent); font-weight: 700; }

.selected-dest {
  display: flex; justify-content: space-between; align-items: center;
  padding: 0.5rem 0.75rem; border-radius: 0.5rem; background: rgba(185,255,102,0.1);
  border: 1px solid var(--color-accent); font-size: 0.85rem; font-weight: 600; color: var(--color-accent);
}
.clear-btn { background: none; border: none; color: var(--color-gray-alt); cursor: pointer; font-size: 0.9rem; }
</style>
