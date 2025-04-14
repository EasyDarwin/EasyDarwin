import { createPinia } from 'pinia'
import { createPersistedState } from 'pinia-plugin-persistedstate'

const pinia = createPinia()

// 持久化插件
pinia.use(createPersistedState())

export const setupStore = app => {
  app.use(pinia)
}
