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

// 跟路由 静态路由
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
    // {
    //   path: '/dashboard',
    //   redirect: '/dashboard/workbench',
    //   name: 'Dashboard',
    //   meta: {
    //     title: 'route.dashboard',
    //     icon: 'material-symbols:dashboard-outline',
    //     namePath: ['Dashboard'],
    //   },
    //   children: [
    //     {
    //       path: 'workbench',
    //       name: 'Workbench',
    //       component: () =>
    //         import('@/views/example/workbench/WorkbenchView.vue'),
    //       meta: {
    //         title: 'route.workbench',
    //         icon: 'icon-park-outline:workbench',
    //         namePath: ['Dashboard', 'Workbench'],
    //       },
    //     },
    //   ],
    // },
    // {
    //   path: '/dashboard',
    //   name: 'Dashboard',
    //   component: () => import('@/views/dashboard/index.vue'),
    //   meta: {
    //     title: 'route.dashboard',
    //     icon: 'material-symbols:dashboard-outline',
    //     namePath: ['Dashboard'],
    //   },
    // },
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
    // {
    //   path: '/plans',
    //   name: 'Plans',
    //   component: () => import('@/views/plans/index.vue'),
    //   meta: {
    //     title: 'route.plans',
    //     icon: 'material-symbols:edit-calendar-outline-rounded',
    //     namePath: ['Plans'],
    //   },
    // },
    // {
    //   path: '/records',
    //   name: 'Records',
    //   meta: {
    //     title: 'route.records',
    //     icon: 'material-symbols:cloud-outline',
    //     namePath: ['Records'],
    //   },
    //   component: () => import('@/views/records/index.vue'),
    // },
    // {
    //   path: '/records/:id/:date',
    //   name: 'RecordsChannel',
    //   hide: true,
    //   meta: {
    //     title: 'route.recordsList',
    //     icon: 'material-symbols:cloud-outline',
    //     namePath: ['RecordsChannel'],
    //   },
    //   component: () => import('@/views/records/channel.vue'),
    // },
    // {
    //   path: '/config',
    //   name: 'Config',
    //   component: () => import('@/views/config/index.vue'),
    //   meta: {
    //     title: 'route.config',
    //     icon: 'material-symbols:build-circle-outline-sharp',
    //     namePath: ['Config'],
    //   },
    // },
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
