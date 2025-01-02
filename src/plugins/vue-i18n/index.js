import { createI18n } from 'vue-i18n'
import zhCN from './zhCN'
import en from './en'

export const setupVueI18n = app => {
  const i18n = createI18n({
    legacy: false,
    locale: 'zhCN',
    messages: {
      zhCN,
      en,
    },
  })

  app.use(i18n)
}
