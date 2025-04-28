import { RouterView } from 'vue-router'
import router from '@/router/index.js'

// refresh
const RefreshRoute = {
  path: '/refresh',
  name: 'Refresh',
  component: RouterView,
  meta: {
    title: 'route.refresh',
  },
  beforeEnter: (to, from) => {
    // refresh
    setTimeout(() => {
      router.replace(from.fullPath)
    })
    return true
  },
}

// Follow Route Static Route
/**
 * meta 属性
 * title: 路由标题
 * icon: 路由图标
 * namePath： 路由名称路径（当前路由namePath 祖先name集合）
 * outsideLink：是否外链 (window.open) 起一个新标签页
 * iframe：iframe内嵌
 * */
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
      path: '/vod',
      name: 'Vod',
      component: () => import('@/views/vod/index.vue'),
      meta: {
        title: 'route.vod',
        icon: 'icon-park-outline:video',
        namePath: ['Vod'],
      },
    },
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
      path: '/apidoc.html',
      name: 'Apidoc',
      component: () => import('@/views/apidoc/index.vue'),
      meta: {
        title: 'route.apidoc',
        outsideLink: true,
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
