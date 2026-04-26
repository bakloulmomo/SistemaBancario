<template>
  <NuxtLayout name="auth">

    <!-- STEP 1: Login / Register -->
    <div v-if="step === 1">
      <div class="tab-toggle">
        <button :class="['tab-btn', { active: tab === 'login' }]" @click="tab = 'login'">Accedi</button>
        <button :class="['tab-btn', { active: tab === 'register' }]" @click="tab = 'register'">Registrati</button>
      </div>

      <!-- LOGIN -->
      <form v-if="tab === 'login'" class="auth-form fade-up" @submit.prevent="doLogin">
        <h2 class="form-title">Bentornato</h2>
        <div class="field">
          <label class="cb-label">Username</label>
          <input v-model="form.username" class="cb-input" placeholder="mario_ross_123" required autocomplete="username" />
        </div>
        <div class="field">
          <label class="cb-label">Password</label>
          <input v-model="form.password" type="password" class="cb-input" placeholder="••••••••" required autocomplete="current-password" />
        </div>
        <p v-if="errore" class="error-msg">{{ errore }}</p>
        <button type="submit" class="btn-accent w-full" :disabled="loading">
          <span v-if="loading">Accesso in corso…</span>
          <span v-else class="arrow-link">Entra <span class="arrow">→</span></span>
        </button>
      </form>

      <!-- REGISTER -->
      <form v-else class="auth-form fade-up" @submit.prevent="doRegister">
        <h2 class="form-title">Crea account</h2>
        <div class="grid-2">
          <div class="field">
            <label class="cb-label">Nome</label>
            <input v-model="form.nome" class="cb-input" placeholder="Mario" required />
          </div>
          <div class="field">
            <label class="cb-label">Cognome</label>
            <input v-model="form.cognome" class="cb-input" placeholder="Rossi" required />
          </div>
        </div>
        <div class="field">
          <label class="cb-label">Età</label>
          <input v-model.number="form.eta" type="number" min="18" max="120" class="cb-input" placeholder="30" required />
        </div>
        <div class="field">
          <label class="cb-label">Username</label>
          <input v-model="form.username" class="cb-input" placeholder="mario_rossi" required autocomplete="username" pattern="[a-zA-Z0-9_]+" title="Solo lettere, numeri e _" />
        </div>
        <div class="field">
          <label class="cb-label">Password</label>
          <input v-model="form.password" type="password" class="cb-input" placeholder="••••••••" required autocomplete="new-password" minlength="6" />
        </div>
        <div class="field">
          <label class="cb-label">Deposito iniziale (€)</label>
          <input v-model.number="form.saldo_iniziale" type="number" min="0" step="0.01" class="cb-input" placeholder="0.00" />
        </div>
        <p v-if="errore" class="error-msg">{{ errore }}</p>
        <button type="submit" class="btn-accent w-full" :disabled="loading">
          <span v-if="loading">Creazione account…</span>
          <span v-else class="arrow-link">Apri conto <span class="arrow">→</span></span>
        </button>
      </form>
    </div>

    <!-- STEP 2: Benvenuto — mostra username generato -->
    <div v-else-if="step === 2" class="auth-form fade-up">
      <div class="welcome-icon">🎉</div>
      <h2 class="form-title">Conto aperto!</h2>
      <p class="form-sub">Salva il tuo username per accedere in futuro.</p>

      <div class="info-box">
        <div class="info-row">
          <span class="info-label">Username</span>
          <span class="info-val monospace">{{ generatedUsername }}</span>
        </div>
        <div class="info-row">
          <span class="info-label">IBAN</span>
          <span class="info-val monospace small">{{ generatedIban }}</span>
        </div>
        <div class="info-row">
          <span class="info-label">Saldo</span>
          <span class="info-val accent">{{ formatEuro(generatedSaldo) }}</span>
        </div>
      </div>

      <button class="btn-accent w-full" @click="navigateTo('/dashboard')">
        <span class="arrow-link">Vai alla Dashboard <span class="arrow">→</span></span>
      </button>
    </div>

  </NuxtLayout>
</template>

<script setup lang="ts">
definePageMeta({ middleware: 'auth', layout: false })

const { setAuth } = useAuth()
const { post } = useApi()

const tab  = ref<'login' | 'register'>('login')
const step = ref(1)
const loading = ref(false)
const errore  = ref('')

const form = reactive({
  username: '',
  nome: '', cognome: '', eta: null as number | null,
  password: '', saldo_iniziale: 0
})

const generatedUsername = ref('')
const generatedIban     = ref('')
const generatedSaldo    = ref(0)

function formatEuro(n: number) {
  return new Intl.NumberFormat('it-IT', { style: 'currency', currency: 'EUR' }).format(n)
}

async function doLogin() {
  loading.value = true; errore.value = ''
  try {
    const r = await post<any>('/api/auth/login', { username: form.username, password: form.password })
    setAuth(r.data.token, r.data.utente)
    await navigateTo('/dashboard')
  } catch (e: any) { errore.value = e.message }
  finally { loading.value = false }
}

async function doRegister() {
  loading.value = true; errore.value = ''
  try {
    const r = await post<any>('/api/auth/registra', {
      username: form.username,
      nome: form.nome,
      cognome: form.cognome,
      eta: form.eta,
      password: form.password,
      saldo_iniziale: form.saldo_iniziale || 0
    })
    setAuth(r.data.token, r.data.utente)
    generatedUsername.value = r.data.username
    generatedIban.value     = r.data.iban
    generatedSaldo.value    = r.data.saldo ?? 0
    step.value = 2
  } catch (e: any) { errore.value = e.message }
  finally { loading.value = false }
}
</script>

<style scoped>
.tab-toggle {
  display: flex;
  background: #ffffff0d;
  border-radius: 0.75rem;
  padding: 0.25rem;
  margin-bottom: 1.5rem;
}
.tab-btn {
  flex: 1; padding: 0.6rem; border: none; background: transparent;
  color: var(--color-gray-alt); border-radius: 0.5rem;
  font-weight: 600; font-size: 0.9rem; cursor: pointer;
  transition: background 0.2s, color 0.2s;
}
.tab-btn.active { background: var(--color-accent); color: var(--color-dark); }

.auth-form { display: flex; flex-direction: column; gap: 1rem; }
.form-title { font-size: 1.5rem; font-weight: 800; color: var(--color-light); margin: 0; }
.form-sub { font-size: 0.9rem; color: var(--color-gray-alt); margin: 0; }
.field { display: flex; flex-direction: column; }
.error-msg { color: #ff7070; font-size: 0.85rem; margin: 0; }
.w-full { width: 100%; }
.welcome-icon { font-size: 2.5rem; text-align: center; }

.info-box {
  background: #ffffff0a;
  border: 1.5px solid #ffffff15;
  border-radius: 0.75rem;
  padding: 1rem;
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}
.info-row { display: flex; justify-content: space-between; align-items: center; gap: 0.5rem; }
.info-label { font-size: 0.75rem; font-weight: 600; color: var(--color-gray-alt); text-transform: uppercase; letter-spacing: 0.04em; flex-shrink: 0; }
.info-val { font-size: 0.9rem; font-weight: 600; color: var(--color-light); }
.info-val.accent { color: var(--color-accent); }
.monospace { font-family: monospace; }
.small { font-size: 0.78rem; }
</style>
