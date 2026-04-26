export default defineNuxtConfig({
  modules: ['@nuxt/ui'],
  ssr: false,
  devtools: { enabled: false },
  css: ['~/assets/css/main.css'],
  vite: {
    server: {
      proxy: {
        '/api': { target: 'http://localhost:3000', changeOrigin: true }
      }
    }
  },
  colorMode: { preference: 'dark' }
})
