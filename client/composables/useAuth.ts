import type { Utente } from '~/types/bank'

export const useAuth = () => {
  const token = useState<string | null>('auth.token', () => null)
  const utente = useState<Utente | null>('auth.utente', () => null)

  const isLoggedIn = computed(() => !!token.value)

  function setAuth(t: string, u: Utente) {
    token.value = t
    utente.value = u
    if (import.meta.client) {
      localStorage.setItem('cb_token', t)
      localStorage.setItem('cb_utente', JSON.stringify(u))
    }
  }

  function clearAuth() {
    token.value = null
    utente.value = null
    if (import.meta.client) {
      localStorage.removeItem('cb_token')
      localStorage.removeItem('cb_utente')
    }
  }

  function initFromStorage() {
    if (!import.meta.client) return
    const t = localStorage.getItem('cb_token')
    const u = localStorage.getItem('cb_utente')
    if (t && u) {
      token.value = t
      utente.value = JSON.parse(u)
    }
  }

  return { token, utente, isLoggedIn, setAuth, clearAuth, initFromStorage }
}
