import NProgress from 'nprogress'
import '@/plugins/nprogress/nprogress.less'

export const setupNProgress = () => {
  NProgress.configure({ showSpinner: false })
}
