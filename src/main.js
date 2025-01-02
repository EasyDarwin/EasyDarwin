import { createApp } from 'vue'
import { setupPlugins } from '@/plugins'
import { setupStore } from '@/store'
import { setupRouter } from '@/router'

import App from '@/App.vue'

const app = createApp(App)

setupPlugins(app)
setupStore(app)
setupRouter(app)

app.mount('#app')
