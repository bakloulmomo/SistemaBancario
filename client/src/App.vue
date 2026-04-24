<script setup lang="ts">
import { ref, onMounted, computed } from 'vue'

interface Conto {
  iban: string
  titolare: string
  saldo: number
}

const conti = ref<Conto[]>([])
const loading = ref(true)
const error = ref<string | null>(null)

const loadData = async () => {
  loading.value = true
  error.value = null

  try {
    const response = await fetch('/data.json')
    if (!response.ok) throw new Error(`HTTP ${response.status}`)

    const data = await response.json() as { conti?: Conto[] }
    conti.value = data.conti ?? []
  } catch (err) {
    error.value = err instanceof Error ? err.message : String(err)
  } finally {
    loading.value = false
  }
}

const numeroConti = computed(() => conti.value.length)
const saldoTotale = computed(() => conti.value.reduce((sum, conto) => sum + conto.saldo, 0))
const formatMoney = (value: number) =>
  new Intl.NumberFormat('it-IT', {
    style: 'currency',
    currency: 'EUR',
  }).format(value)

onMounted(loadData)
</script>

<template>
  <main class="dashboard">
    <header class="topbar">
      <div>
        <p class="eyebrow">C-BANK MANAGER</p>
        <h1>Dashboard Vue</h1>
        <p class="subtitle">
          Visualizza i conti esportati dal progetto C-Bank in tempo reale.
        </p>
      </div>
      <button class="refresh" @click="loadData" :disabled="loading">
        {{ loading ? 'Aggiornamento...' : 'Ricarica dati' }}
      </button>
    </header>

    <section class="metrics">
      <article class="metric">
        <span class="metric-label">Conti caricati</span>
        <strong>{{ numeroConti }}</strong>
      </article>
      <article class="metric">
        <span class="metric-label">Saldo totale</span>
        <strong>{{ formatMoney(saldoTotale) }}</strong>
      </article>
    </section>

    <section class="content">
      <div v-if="loading" class="empty-state">
        Caricamento dei dati in corso...
      </div>

      <div v-else-if="error" class="empty-state error-state">
        <h2>Impossibile caricare i dati</h2>
        <p>{{ error }}</p>
        <p>
          Assicurati di eseguire il programma C e generare
          <code>public/data.json</code> nel frontend.
        </p>
      </div>

      <div v-else-if="conti.length === 0" class="empty-state">
        Nessun conto trovato. Esegui il backend C per esportare i dati.
      </div>

      <div v-else class="cards">
        <article class="card" v-for="conto in conti" :key="conto.iban">
          <div class="card-row">
            <span>IBAN</span>
            <strong>{{ conto.iban }}</strong>
          </div>
          <div class="card-row">
            <span>Titolare</span>
            <strong>{{ conto.titolare }}</strong>
          </div>
          <div class="card-row">
            <span>Saldo</span>
            <strong>{{ formatMoney(conto.saldo) }}</strong>
          </div>
        </article>
      </div>
    </section>
  </main>
</template>
