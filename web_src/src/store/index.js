import { createPinia } from 'pinia'
import { createPersistedState } from 'pinia-plugin-persistedstate'

const pinia = createPinia()

// Persistence plug-ins
pinia.use(createPersistedState())

export const setupStore = app => {
  app.use(pinia)
}
