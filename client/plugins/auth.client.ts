export default defineNuxtPlugin(() => {
  const { initFromStorage } = useAuth()
  initFromStorage()
})
