<script setup>
import LayoutHeader from '@/layouts/components/header/LayoutHeader.vue'
import { ref, computed } from 'vue'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import LayoutTabs from '@/layouts/components/tabs/LayoutTabs.vue'
import LayoutFooter from '@/layouts/components/footer/LayoutFooter.vue'
import LayoutPage from '@/layouts/components/page/LayoutPage.vue'
import LayoutTitle from '@/layouts/components/title/LayoutTitle.vue'
import LayoutMenu from '@/layouts/components/menu/LayoutMenu.vue'

const collapsed = ref(false)
const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = layoutThemeStore.layoutSetting
const sidemenuWidth = computed(() => layoutSetting.sidemenuWidth)
const showTitle = computed(() => layoutSetting.showTitle)
const showHeader = computed(() => layoutSetting.showHeader)
const showFooter = computed(() => layoutSetting.showFooter)
const watermark = computed(() => layoutSetting.watermark)
const watermarkArea_content = computed(
  () => layoutSetting.watermarkArea === 'content',
)
const watermarkText = computed(() => layoutSetting.watermarkText)
</script>

<template>
  <a-layout>
    <a-drawer
      v-model:open="collapsed"
      :placement="'left'"
      :bodyStyle="{ padding: 0 }"
      :width="sidemenuWidth"
      :closable="false"
    >
      <LayoutTitle v-if="showTitle" :collapsed="false" />
      <LayoutMenu :collapsed="collapsed" />
    </a-drawer>
    <LayoutHeader v-if="showHeader" v-model:collapsed="collapsed" />
    <a-layout-content>
      <LayoutTabs />
      <a-watermark
        v-if="watermark && watermarkArea_content"
        :content="watermarkText"
      >
        <LayoutPage />
      </a-watermark>
      <LayoutPage v-else />
    </a-layout-content>
    <LayoutFooter v-if="showFooter" />
  </a-layout>
</template>

<style scoped lang="less"></style>
