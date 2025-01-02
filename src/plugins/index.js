import { setupAntDesignVue } from '@/plugins/ant-design-vue'
import { setupVueI18n } from '@/plugins/vue-i18n'
import { setupUnoCSS } from '@/plugins/unocss'
import { setupSvg } from '@/plugins/svg'
import { setupNProgress } from '@/plugins/nprogress'
import { setupAnimate } from '@/plugins/animate'
import { setupDayjs } from '@/plugins/dayjs'
import { setupStyle } from '@/plugins/style'

export const setupPlugins = app => {
  setupAntDesignVue()
  setupVueI18n(app)
  setupUnoCSS()
  setupSvg()
  setupNProgress()
  setupAnimate()
  setupDayjs()
  setupStyle()
}
