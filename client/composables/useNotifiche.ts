interface Notifica {
  messaggio: string
  timestamp: string
}

const notifiche = ref<Notifica[]>([])
const loading = ref(false)

export const useNotifiche = () => {
  const { get } = useApi()
  const { token } = useAuth()

  const count = computed(() => notifiche.value.length)

  async function fetchNotifiche() {
    if (!token.value) return
    loading.value = true
    try {
      const r = await get<any>('/api/notifiche')
      if (r.data?.notifiche) notifiche.value = r.data.notifiche
    } catch {}
    loading.value = false
  }

  function clear() {
    notifiche.value = []
  }

  return { notifiche, count, loading, fetchNotifiche, clear }
}
