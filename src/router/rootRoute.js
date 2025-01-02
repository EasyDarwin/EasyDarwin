import { RouterView } from 'vue-router'
import router from '@/router/index.js'

// 刷新
const RefreshRoute = {
  path: '/refresh',
  name: 'Refresh',
  component: RouterView,
  meta: {
    title: 'route.refresh',
  },
  beforeEnter: (to, from) => {
    // 刷新
    setTimeout(() => {
      router.replace(from.fullPath)
    })
    return true
  },
}

const RootRoute = {
  path: '/',
  redirect: '/live',
  name: 'Layout',
  component: () => import('@/layouts/LayoutDefault.vue'),
  meta: {
    title: 'route.rootRoute',
    icon: 'material-symbols:account-tree-outline-rounded',
  },
  children: [
    {
      path: '/live',
      name: 'Live',
      component: () => import('@/views/live/index.vue'),
      meta: {
        title: 'route.live',
        icon: 'icon-park-outline:workbench',
        namePath: ['Live'],
      },
    },
    {
      path: '/config',
      name: 'Config',
      component: () => import('@/views/config/index.vue'),
      meta: {
        title: 'route.config',
        icon: 'material-symbols:build-circle-outline-sharp',  
        namePath: ['Config'],
      },
    },
    {
      path: '/apidoc.html',
      name: 'Apidoc',
      component: () => import('@/views/apidoc/index.vue'),
      meta: {
        title: 'route.apidoc',
        outsideLink:true,
        icon: 'material-symbols:unknown-document-outline-rounded',  
        namePath: ['Apidoc'],
      },
    },
    {
      path: '/version',
      name: 'Version',
      component: () => import('@/views/version/index.vue'),
      meta: {
        title: 'route.about',
        icon: 'material-symbols:info-outline-rounded',
        namePath: ['Version'],
      },
    },
    RefreshRoute,
  ],
}

export default RootRoute
