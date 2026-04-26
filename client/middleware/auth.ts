export default defineNuxtRouteMiddleware((to) => {
  const { isLoggedIn } = useAuth()
  if (!isLoggedIn.value && !to.path.startsWith('/auth')) {
    return navigateTo('/auth')
  }
  if (isLoggedIn.value && to.path === '/auth') {
    return navigateTo('/dashboard')
  }
})
