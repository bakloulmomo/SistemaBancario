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

const initials = computed(() => {
  if (!utente.value) return '?'
  return (utente.value.nome[0] ?? '') + (utente.value.cognome[0] ?? '')
})

async function logout() {
  try { await get('/api/auth/logout') } catch {}
  clearAuth()
  await navigateTo('/auth')
}
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
