<script setup>
import { computed } from 'vue'
import { RouterView } from 'vue-router'
import dayjs from 'dayjs'
import antd_zhCN from 'ant-design-vue/es/locale/zh_CN'
import antd_enUS from 'ant-design-vue/es/locale/en_US'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import LockScreen from '@/layouts/components/lockscreen/LockScreen.vue'
const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = layoutThemeStore.layoutSetting
const theme = computed(() => layoutThemeStore.theme)
const watermark = computed(() => layoutSetting.watermark)
const watermarkArea_all = computed(() => layoutSetting.watermarkArea === 'all')
const watermarkText = computed(() => layoutSetting.watermarkText)
const locale = computed(() => {
  switch (layoutSetting.locale) {
    case 'zh_CN':
      dayjs.locale('zh-cn')
      return antd_zhCN
    case 'en':
      dayjs.locale('en')
      return antd_enUS
    default:
      return antd_zhCN
  }
})

</script>

<template>
  <a-config-provider :theme="theme" :locale="locale">
    <a-watermark v-if="watermark && watermarkArea_all" :content="watermarkText">
      <RouterView />
      <!-- <LockScreen /> -->
    </a-watermark>
    <template v-else>
      <RouterView />
      <!-- <LockScreen /> -->
    </template>
  </a-config-provider>
</template>

<style lang="less" scoped></style>
