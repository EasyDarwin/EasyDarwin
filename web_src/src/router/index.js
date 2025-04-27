import { createRouter, createWebHistory, createWebHashHistory } from 'vue-router'
import { routerGuards } from '@/router/guards.js'
import rootRoute from '@/router/rootRoute.js'
import outsideLayoutRoute from '@/router/outsideLayoutRoute.js'

const routes = [rootRoute, ...outsideLayoutRoute]

const router = createRouter({
  // history: createWebHistory(import.meta.env.BASE_URL),
  history: createWebHashHistory(),
  routes,
})

export const setupRouter = app => {
  routerGuards(router)
  app.use(router)
}

export default router
