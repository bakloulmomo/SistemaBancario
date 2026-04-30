<template>
  <div class="app-shell">
    <!-- Sidebar -->
    <aside class="sidebar">
      <div class="sidebar-brand">
        <span class="brand-dot" />
        <span class="brand-name">CBank</span>
      </div>

      <nav class="sidebar-nav">
        <NuxtLink to="/dashboard" class="nav-item" :class="{ active: route.path === '/dashboard' }">
          <UIcon name="i-heroicons-home" class="nav-icon" />
          <span>Home</span>
        </NuxtLink>
        <NuxtLink to="/dashboard/settings" class="nav-item" :class="{ active: route.path === '/dashboard/settings' }">
          <UIcon name="i-heroicons-user-circle" class="nav-icon" />
          <span>Profilo</span>
        </NuxtLink>
        <NuxtLink to="/dashboard/chatbot" class="nav-item" :class="{ active: route.path === '/dashboard/chatbot' }">
          <UIcon name="i-heroicons-chat-bubble-left-right" class="nav-icon" />
          <span>Assistente</span>
        </NuxtLink>

        <!-- Notifiche -->
        <button class="nav-item notif-btn" @click="toggleNotifiche">
          <div class="notif-icon-wrap">
            <UIcon name="i-heroicons-bell" class="nav-icon" />
            <span v-if="count > 0" class="notif-badge">{{ count }}</span>
          </div>
          <span>Notifiche</span>
        </button>

        <div v-if="showNotifiche" class="notif-panel">
          <p class="notif-title">Notifiche</p>
          <div v-if="notifiche.length === 0" class="notif-empty">Nessuna notifica</div>
          <div v-else class="notif-list">
            <div v-for="(n, i) in notifiche" :key="i" class="notif-item">
              <p class="notif-msg">{{ n.messaggio }}</p>
              <p class="notif-ts">{{ n.timestamp }}</p>
            </div>
          </div>
        </div>
      </nav>

      <div class="sidebar-user">
        <div class="user-avatar">{{ initials }}</div>
        <div class="user-info">
          <p class="user-name">{{ utente?.nome }} {{ utente?.cognome }}</p>
          <p class="user-sub">{{ utente?.username }}</p>
        </div>
        <button class="logout-btn" @click="logout" title="Esci">
          <UIcon name="i-heroicons-arrow-right-on-rectangle" />
        </button>
      </div>
    </aside>

    <!-- Main -->
    <main class="main-content">
      <slot />
    </main>
  </div>
</template>

<script setup lang="ts">
const route = useRoute()
const { utente, clearAuth } = useAuth()
const { get } = useApi()
const { notifiche, count, fetchNotifiche, clear } = useNotifiche()

const initials = computed(() => {
  if (!utente.value) return '?'
  return (utente.value.nome[0] ?? '') + (utente.value.cognome[0] ?? '')
})

const showNotifiche = ref(false)

function toggleNotifiche() {
  showNotifiche.value = !showNotifiche.value
}

async function logout() {
  try { await get('/api/auth/logout') } catch {}
  clear()
  clearAuth()
  await navigateTo('/auth')
}

let pollInterval: ReturnType<typeof setInterval> | null = null

onMounted(async () => {
  await fetchNotifiche()
  pollInterval = setInterval(fetchNotifiche, 30000)
})

onUnmounted(() => {
  if (pollInterval) clearInterval(pollInterval)
})
</script>

<style scoped>
.app-shell {
  display: flex;
  min-height: 100vh;
  background: var(--color-dark);
}

.sidebar {
  width: 240px;
  min-height: 100vh;
  background: var(--color-dark);
  border-right: 1.5px solid #ffffff10;
  display: flex;
  flex-direction: column;
  padding: 1.5rem 1rem;
  position: fixed;
  top: 0;
  left: 0;
  bottom: 0;
  z-index: 10;
}

.sidebar-brand {
  display: flex;
  align-items: center;
  gap: 0.6rem;
  font-size: 1.3rem;
  font-weight: 800;
  color: var(--color-light);
  margin-bottom: 2rem;
  padding: 0 0.5rem;
}

.brand-dot {
  width: 10px;
  height: 10px;
  border-radius: 50%;
  background: var(--color-accent);
}

.sidebar-nav {
  display: flex;
  flex-direction: column;
  gap: 0.25rem;
  flex: 1;
}

.sidebar-user {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  padding: 0.75rem;
  border-radius: 0.75rem;
  background: #ffffff08;
  margin-top: 1rem;
}

.user-avatar {
  width: 2rem;
  height: 2rem;
  border-radius: 50%;
  background: var(--color-accent);
  color: var(--color-dark);
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: 700;
  font-size: 0.75rem;
  flex-shrink: 0;
}

.user-info { flex: 1; min-width: 0; }
.user-name {
  font-size: 0.82rem;
  font-weight: 600;
  color: var(--color-light);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}
.user-sub {
  font-size: 0.72rem;
  color: var(--color-gray-alt);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.logout-btn {
  background: none;
  border: none;
  color: var(--color-gray-alt);
  cursor: pointer;
  padding: 0.25rem;
  border-radius: 0.4rem;
  transition: color 0.2s, background 0.2s;
  display: flex;
  align-items: center;
}
.logout-btn:hover { color: #ff7070; background: rgba(255,112,112,0.1); }

.notif-btn {
  background: none;
  border: none;
  color: inherit;
  cursor: pointer;
  width: 100%;
  text-align: left;
}

.notif-icon-wrap {
  position: relative;
  display: flex;
  align-items: center;
}

.notif-badge {
  position: absolute;
  top: -6px;
  right: -8px;
  background: var(--color-accent);
  color: var(--color-dark);
  font-size: 0.6rem;
  font-weight: 800;
  border-radius: 99px;
  padding: 1px 5px;
  min-width: 16px;
  text-align: center;
  line-height: 1.4;
}

.notif-panel {
  background: #1a1a1a;
  border: 1.5px solid #ffffff15;
  border-radius: 0.75rem;
  padding: 0.75rem;
  margin-top: 0.25rem;
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
  max-height: 260px;
  overflow-y: auto;
}

.notif-title {
  font-size: 0.75rem;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.05em;
  color: var(--color-gray-alt);
  margin: 0;
}

.notif-empty {
  font-size: 0.8rem;
  color: var(--color-gray-alt);
  text-align: center;
  padding: 0.5rem 0;
}

.notif-list { display: flex; flex-direction: column; gap: 0.4rem; }

.notif-item {
  background: #ffffff08;
  border-radius: 0.5rem;
  padding: 0.5rem 0.65rem;
}

.notif-msg {
  font-size: 0.82rem;
  font-weight: 600;
  color: var(--color-light);
  margin: 0;
}

.notif-ts {
  font-size: 0.7rem;
  color: var(--color-gray-alt);
  margin: 0.15rem 0 0;
}

.main-content {
  margin-left: 240px;
  flex: 1;
  padding: 2rem 2.5rem;
  background: var(--color-dark-alt);
  min-height: 100vh;
}

@media (max-width: 768px) {
  .sidebar { display: none; }
  .main-content { margin-left: 0; padding: 1rem; }
}
</style>
