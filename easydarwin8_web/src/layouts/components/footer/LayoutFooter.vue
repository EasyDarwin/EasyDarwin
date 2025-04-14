<script setup>
import { computed } from 'vue'
import { useSystemStore } from '@/store/layout/system.js'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'

const systemStore = useSystemStore()
const systemTime = computed(() => systemStore.systemTime)
const layoutThemeStore = useLayoutThemeStore()
const copyright = computed(() => layoutThemeStore.layoutSetting.copyright)
</script>

<template>
  <a-layout-footer style="padding: 0 20px 20px 20px">
    <a-card class="text-center" :bodyStyle="{ padding: '10px' }">
      Copyright © 2012~{{ systemTime.year }} 
      <a href="http://www.easydarwin.org" target="_blank" >www.{{ copyright }}.org</a>
    </a-card>
  </a-layout-footer>
</template>

<style lang="less" scoped></style>
