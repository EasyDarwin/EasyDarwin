import NProgress from 'nprogress'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import { getTokenStorage } from '@/utils/storage'
export const routerGuards = router => {
  router.beforeEach((to, from, next) => {
    const layoutThemeStore = useLayoutThemeStore()
    if (layoutThemeStore.layoutSetting.showProgress) {
      NProgress.start()
    }
    if (to.name!= "Login"&&getTokenStorage()==null) {
      next("/login")
      return
    }
    next()
  })

  router.afterEach(to => {
    
    const layoutThemeStore = useLayoutThemeStore()
    if (layoutThemeStore.layoutSetting.showProgress) {
      NProgress.done()
    }
    // console.log('路由跳转', to, from, failure)
  })

  router.onError(() => {
    // console.error('路由错误', error)
  })
}
