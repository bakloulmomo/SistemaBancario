<template>
  <div class="page fade-up">
    <div class="page-header">
      <h1 class="page-title">Profilo</h1>
      <p class="page-sub">Gestisci il tuo account</p>
    </div>

    <div class="grid-2" style="align-items:start">
      <!-- Avatar -->
      <div class="card-accent" style="display:flex;flex-direction:column;align-items:center;justify-content:center;gap:1rem;min-height:200px">
        <div class="big-avatar">{{ initials }}</div>
        <p style="font-weight:700;font-size:1.1rem;color:var(--color-dark)">{{ fNome }} {{ fCognome }}</p>
        <span class="badge" style="background:rgba(25,26,35,0.15);color:var(--color-dark)">@{{ utente?.username }}</span>
      </div>

      <!-- Info read-only -->
      <div class="card">
        <h2 class="section-title">Informazioni</h2>
        <div class="info-grid">
          <div class="info-row">
            <span class="info-label">Username</span>
            <span class="info-val monospace">{{ utente?.username }}</span>
          </div>
          <div class="info-row">
            <span class="info-label">IBAN</span>
            <span class="info-val monospace small">{{ iban }}</span>
          </div>
          <div class="info-row">
            <span class="info-label">Data nascita</span>
            <span class="info-val">{{ formatData(utente?.data_nascita) }}</span>
          </div>
        </div>
      </div>
    </div>

    <!-- Modifica nome/cognome -->
    <div class="card" style="margin-top:1.25rem">
      <h2 class="section-title" style="margin-bottom:1.25rem">Modifica profilo</h2>
      <form @submit.prevent="salvaProfilo">
        <div class="grid-2" style="gap:1rem;margin-bottom:1rem">
          <div class="field">
            <label class="cb-label">Nome</label>
            <input v-model="fNome" class="cb-input" required />
          </div>
          <div class="field">
            <label class="cb-label">Cognome</label>
            <input v-model="fCognome" class="cb-input" required />
          </div>
        </div>
        <p v-if="profiloErr" class="error-msg">{{ profiloErr }}</p>
        <p v-if="profiloOk"  class="success-msg">{{ profiloOk }}</p>
        <div style="display:flex;justify-content:flex-end">
          <button type="submit" class="btn-accent" :disabled="profiloLoading">
            {{ profiloLoading ? 'Salvataggio…' : 'Salva modifiche' }}
          </button>
        </div>
      </form>
    </div>

    <!-- Cambia password -->
    <div class="card" style="margin-top:1.25rem">
      <h2 class="section-title" style="margin-bottom:1.25rem">Cambia password</h2>
      <form @submit.prevent="cambiaPassword">
        <div class="field" style="margin-bottom:0.75rem">
          <label class="cb-label">Password attuale</label>
          <input v-model="vecchiaPassword" type="password" class="cb-input" placeholder="••••••••" required />
        </div>
        <div class="field" style="margin-bottom:1rem">
          <label class="cb-label">Nuova password</label>
          <input v-model="nuovaPassword" type="password" class="cb-input" placeholder="••••••••" minlength="6" required />
        </div>
        <p v-if="pwErr" class="error-msg">{{ pwErr }}</p>
        <p v-if="pwOk"  class="success-msg">{{ pwOk }}</p>
        <div style="display:flex;justify-content:flex-end">
          <button type="submit" class="btn-ghost" :disabled="pwLoading">
            {{ pwLoading ? '…' : 'Aggiorna password' }}
          </button>
        </div>
      </form>
    </div>

    <!-- Elimina account -->
    <div class="card danger-card" style="margin-top:1.25rem">
      <h2 class="section-title" style="color:#ff7070;margin-bottom:0.5rem">Zona pericolo</h2>
      <p class="page-sub" style="margin-bottom:1.25rem">L'eliminazione è permanente e non può essere annullata.</p>
      <button class="btn-danger" @click="showElimina = true">Elimina account</button>
    </div>

    <!-- MODAL: Conferma elimina -->
    <div v-if="showElimina" class="modal-overlay" @click.self="showElimina = false">
      <div class="modal-box card">
        <h3 class="modal-title" style="color:#ff7070">⚠ Conferma eliminazione</h3>
        <p style="font-size:0.9rem;color:var(--color-gray-alt)">
          Il tuo account e tutti i dati saranno eliminati definitivamente.
          Inserisci la tua password per confermare.
        </p>
        <form @submit.prevent="eliminaAccount">
          <div class="field">
            <label class="cb-label">Password</label>
            <input v-model="eliminaPassword" type="password" class="cb-input" placeholder="••••••••" required />
          </div>
          <p v-if="eliminaErr" class="error-msg">{{ eliminaErr }}</p>
          <div class="modal-actions">
            <button type="button" class="btn-ghost" @click="showElimina = false">Annulla</button>
            <button type="submit" class="btn-danger" :disabled="eliminaLoading">
              {{ eliminaLoading ? 'Eliminazione…' : 'Elimina definitivamente' }}
            </button>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
definePageMeta({ middleware: 'auth' })

const { utente, token, setAuth, clearAuth } = useAuth()
const { put, del } = useApi()
const { contoPrimario, caricaConti } = useBank()

const fNome    = ref(utente.value?.nome ?? '')
const fCognome = ref(utente.value?.cognome ?? '')
const iban     = ref(contoPrimario.value?.iban ?? '—')

const initials = computed(() => (fNome.value[0] ?? '') + (fCognome.value[0] ?? ''))

const profiloLoading = ref(false)
const profiloErr = ref('')
const profiloOk  = ref('')

const pwLoading = ref(false)
const pwErr = ref('')
const pwOk  = ref('')
const vecchiaPassword = ref('')
const nuovaPassword   = ref('')

const showElimina    = ref(false)
const eliminaLoading = ref(false)
const eliminaErr     = ref('')
const eliminaPassword = ref('')

function formatData(d?: string) {
  if (!d) return '—'
  return new Date(d).toLocaleDateString('it-IT', { day: '2-digit', month: 'long', year: 'numeric' })
}

async function salvaProfilo() {
  profiloLoading.value = true; profiloErr.value = ''; profiloOk.value = ''
  try {
    await put('/api/utenti/profilo', { nome: fNome.value, cognome: fCognome.value })
    if (utente.value && token.value)
      setAuth(token.value, { ...utente.value, nome: fNome.value, cognome: fCognome.value })
    profiloOk.value = 'Profilo aggiornato.'
  } catch (e: any) { profiloErr.value = e.message }
  finally { profiloLoading.value = false }
}

async function cambiaPassword() {
  pwLoading.value = true; pwErr.value = ''; pwOk.value = ''
  try {
    await put('/api/utenti/password', { vecchia_password: vecchiaPassword.value, nuova_password: nuovaPassword.value })
    vecchiaPassword.value = ''; nuovaPassword.value = ''
    pwOk.value = 'Password aggiornata.'
  } catch (e: any) { pwErr.value = e.message }
  finally { pwLoading.value = false }
}

async function eliminaAccount() {
  eliminaLoading.value = true; eliminaErr.value = ''
  try {
    await del('/api/utenti/account', { password: eliminaPassword.value })
    clearAuth()
    await navigateTo('/auth')
  } catch (e: any) { eliminaErr.value = e.message }
  finally { eliminaLoading.value = false }
}

onMounted(async () => {
  await caricaConti()
  iban.value = contoPrimario.value?.iban ?? '—'
})
</script>

<style scoped>
.page { display: flex; flex-direction: column; }
.page-header { margin-bottom: 1.5rem; }
.page-title { font-size: 1.6rem; font-weight: 800; margin: 0; }
.page-sub { font-size: 0.85rem; color: var(--color-gray-alt); margin: 0.25rem 0 0; }
.section-title { font-size: 1rem; font-weight: 700; margin: 0; }

.big-avatar {
  width: 5rem; height: 5rem; border-radius: 50%;
  background: var(--color-dark); color: var(--color-accent);
  display: flex; align-items: center; justify-content: center;
  font-size: 1.6rem; font-weight: 800;
}
.info-grid { display: flex; flex-direction: column; gap: 0.75rem; margin-top: 1rem; }
.info-row { display: flex; justify-content: space-between; align-items: center; }
.info-label { font-size: 0.78rem; color: var(--color-gray-alt); font-weight: 600; text-transform: uppercase; letter-spacing: 0.04em; }
.info-val { font-size: 0.88rem; font-weight: 500; color: var(--color-light); }
.monospace { font-family: monospace; }
.small { font-size: 0.76rem; }

.field { display: flex; flex-direction: column; }
.error-msg { color: #ff7070; font-size: 0.85rem; margin: 0 0 0.75rem; }
.success-msg { color: var(--color-accent); font-size: 0.85rem; margin: 0 0 0.75rem; }

.danger-card { border-color: rgba(255,112,112,0.25) !important; }
.btn-danger {
  background: rgba(255,112,112,0.15); color: #ff7070;
  border: 1.5px solid rgba(255,112,112,0.35); border-radius: 0.75rem;
  padding: 0.65rem 1.4rem; font-weight: 700; cursor: pointer;
  transition: background 0.2s, border-color 0.2s;
}
.btn-danger:hover { background: rgba(255,112,112,0.25); border-color: #ff7070; }

.modal-overlay {
  position: fixed; inset: 0; background: rgba(0,0,0,0.75);
  display: flex; align-items: center; justify-content: center; z-index: 100;
  animation: fadeUp 0.2s ease;
}
.modal-box { width: 100%; max-width: 420px; display: flex; flex-direction: column; gap: 1rem; }
.modal-title { font-size: 1.2rem; font-weight: 700; margin: 0; }
.modal-actions { display: flex; gap: 0.75rem; justify-content: flex-end; margin-top: 0.5rem; }
</style>
