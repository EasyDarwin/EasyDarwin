<script setup>
import { computed } from 'vue'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import LayoutSetting from '@/layouts/components/setting/LayoutSetting.vue'
import LayoutPage from '@/layouts/components/page/LayoutPage.vue'

const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = layoutThemeStore.layoutSetting
const watermark = computed(() => layoutSetting.watermark)
const watermarkArea_content = computed(
  () => layoutSetting.watermarkArea === 'content',
)
const watermarkText = computed(() => layoutSetting.watermarkText)
const showSetting = computed(() => layoutSetting.showSetting)
</script>

<template>
  <a-layout class="h100vh overflow-hidden">
    <a-watermark
      v-if="watermark && watermarkArea_content"
      :content="watermarkText"
    >
      <LayoutPage />
    </a-watermark>
    <LayoutPage v-else />
    <a-card
      class="pos-fixed right--1 top-100 z-999"
      :bodyStyle="{ padding: '10px' }"
      v-if="showSetting"
    >
      <LayoutSetting />
    </a-card>
  </a-layout>
</template>

<style lang="less" scoped></style>
