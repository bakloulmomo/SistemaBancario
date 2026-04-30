<template>
  <div class="auth-shell">
    <div class="auth-brand">
      <span class="brand-dot" />
      <span class="brand-name">PayByte</span>
    </div>
    <div class="auth-content fade-up">
      <slot />
    </div>
    <p class="auth-footer">©  PayByte — Sistema Bancario</p>
  </div>
</template>

<style scoped>
.auth-shell {
  min-height: 100vh;
  background: var(--color-dark);
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 2rem;
  gap: 2rem;
}
.auth-brand {
  display: flex;
  align-items: center;
  gap: 0.6rem;
  font-size: 1.5rem;
  font-weight: 800;
  color: var(--color-light);
}
.brand-dot {
  width: 12px;
  height: 12px;
  border-radius: 50%;
  background: var(--color-accent);
  display: inline-block;
}
.auth-content {
  width: 100%;
  max-width: 440px;
}
.auth-footer {
  font-size: 0.75rem;
  color: var(--color-gray-alt);
}
</style>
