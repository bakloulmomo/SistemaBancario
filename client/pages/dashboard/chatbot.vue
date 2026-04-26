<template>
  <div class="page fade-up">
    <div class="page-header">
      <div>
        <h1 class="page-title">Assistente AI</h1>
        <p class="page-sub">Chiedi qualsiasi cosa sulla tua banca</p>
      </div>
      <span class="badge badge-green">● Online</span>
    </div>

    <div class="chat-shell card">
      <!-- Messages -->
      <div ref="chatEl" class="chat-messages">
        <div v-if="messaggi.length === 0" class="chat-empty">
          <div class="chat-empty-icon">🤖</div>
          <p>Ciao! Sono il tuo assistente bancario. Come posso aiutarti?</p>
        </div>
        <div v-for="(m, i) in messaggi" :key="i" :class="['msg', m.ruolo]">
          <div class="msg-bubble">{{ m.testo }}</div>
          <span class="msg-time">{{ m.ora }}</span>
        </div>
        <div v-if="loading" class="msg assistant">
          <div class="msg-bubble typing"><span/><span/><span/></div>
        </div>
      </div>

      <!-- Input -->
      <form class="chat-input-row" @submit.prevent="invia">
        <input
          v-model="testo"
          class="cb-input"
          placeholder="Scrivi un messaggio…"
          :disabled="loading"
          @keydown.enter.exact.prevent="invia"
        />
        <button type="submit" class="btn-accent send-btn" :disabled="loading || !testo.trim()">
          <UIcon name="i-heroicons-paper-airplane" />
        </button>
      </form>
    </div>
  </div>
</template>

<script setup lang="ts">
definePageMeta({ middleware: 'auth' })

interface Messaggio { ruolo: 'user' | 'assistant'; testo: string; ora: string }

const { post } = useApi()
const testo    = ref('')
const loading  = ref(false)
const messaggi = ref<Messaggio[]>([])
const chatEl   = ref<HTMLElement | null>(null)

function oraNow() {
  return new Date().toLocaleTimeString('it-IT', { hour: '2-digit', minute: '2-digit' })
}

function scrollBottom() {
  nextTick(() => { if (chatEl.value) chatEl.value.scrollTop = chatEl.value.scrollHeight })
}

async function invia() {
  const msg = testo.value.trim()
  if (!msg || loading.value) return
  testo.value = ''
  messaggi.value.push({ ruolo: 'user', testo: msg, ora: oraNow() })
  scrollBottom()
  loading.value = true
  try {
    const r = await post<any>('/api/chatbot/messaggio', { messaggio: msg })
    messaggi.value.push({ ruolo: 'assistant', testo: r.risposta ?? r.data?.risposta ?? '…', ora: oraNow() })
  } catch (e: any) {
    messaggi.value.push({ ruolo: 'assistant', testo: `Errore: ${e.message}`, ora: oraNow() })
  } finally {
    loading.value = false
    scrollBottom()
  }
}
</script>

<style scoped>
.page { display: flex; flex-direction: column; height: calc(100vh - 4rem); }
.page-header { display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 1.25rem; flex-shrink: 0; }
.page-title { font-size: 1.6rem; font-weight: 800; margin: 0; }
.page-sub { font-size: 0.85rem; color: var(--color-gray-alt); margin: 0.25rem 0 0; }

.chat-shell {
  flex: 1;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  padding: 0;
}

.chat-messages {
  flex: 1;
  overflow-y: auto;
  padding: 1.25rem;
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}

.chat-empty {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 0.75rem;
  flex: 1;
  color: var(--color-gray-alt);
  text-align: center;
}
.chat-empty-icon { font-size: 2.5rem; }

.msg { display: flex; flex-direction: column; max-width: 72%; }
.msg.user { align-self: flex-end; align-items: flex-end; }
.msg.assistant { align-self: flex-start; align-items: flex-start; }

.msg-bubble {
  padding: 0.7rem 1rem;
  border-radius: 1rem;
  font-size: 0.9rem;
  line-height: 1.5;
}
.msg.user .msg-bubble { background: var(--color-accent); color: var(--color-dark); border-bottom-right-radius: 0.25rem; }
.msg.assistant .msg-bubble { background: #ffffff12; color: var(--color-light); border-bottom-left-radius: 0.25rem; }

.msg-time { font-size: 0.7rem; color: var(--color-gray-alt); margin-top: 0.25rem; padding: 0 0.25rem; }

.typing { display: flex; gap: 5px; align-items: center; padding: 0.9rem 1rem; }
.typing span {
  width: 7px; height: 7px; border-radius: 50%;
  background: var(--color-gray-alt);
  animation: bounce 1.2s infinite ease-in-out;
}
.typing span:nth-child(2) { animation-delay: 0.2s; }
.typing span:nth-child(3) { animation-delay: 0.4s; }
@keyframes bounce {
  0%, 80%, 100% { transform: translateY(0); }
  40% { transform: translateY(-6px); }
}

.chat-input-row {
  display: flex;
  gap: 0.75rem;
  padding: 1rem 1.25rem;
  border-top: 1px solid #ffffff10;
  flex-shrink: 0;
}
.send-btn {
  padding: 0.65rem 1rem;
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: center;
}
</style>
