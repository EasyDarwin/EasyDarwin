<script setup>
import { useRouter } from 'vue-router'
import { useSystemStore } from '@/store/layout/system.js'

const router = useRouter()
const systemStore = useSystemStore()

const dynamicRoutes = [
  {
    path: '/dynamic-route',
    name: 'DynamicRoute',
    component: () =>
      import('@/views/example/dynamicRoute/DynamicRouteView.vue'),
    meta: {
      title: 'route.dynamicRoute',
      icon: 'material-symbols:route',
      namePath: ['DynamicRoute'],
    },
  },
]

const addDynamicRoute = () => {
  dynamicRoutes.forEach(dynamicRoute => {
    const routes = router.getRoutes()
    const hasRoute = routes.some(route => route.name === dynamicRoute.name)
    if (hasRoute) return
    // 动态添加路由
    router.addRoute('Layout', dynamicRoute)
    // 动态添加菜单
    systemStore.updateMenus('add', dynamicRoute)
  })
}

const deleteDynamicRoute = () => {
  dynamicRoutes.forEach(dynamicRoute => {
    // 动态删除路由
    router.removeRoute(dynamicRoute.name)

    // 动态删除菜单
    systemStore.updateMenus('delete', dynamicRoute)
  })
}
</script>

<template>
  <a-card>
    <a-button-group>
      <a-button type="primary" @click="addDynamicRoute">增加动态路由</a-button>
      <a-button type="primary" danger @click="deleteDynamicRoute">
        删除动态路由
      </a-button>
    </a-button-group>
  </a-card>
</template>

<style scoped lang="less"></style>
