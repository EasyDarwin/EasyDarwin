<script setup>
import { computed } from 'vue'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import { useSystemStore } from '@/store/layout/system.js'
import OnlyShowContentLayout from '@/layouts/OnlyShowContentLayout.vue'
import SideMenuLayout from '@/layouts/SideMenuLayout.vue'
import TopMenuLayout from '@/layouts/TopMenuLayout.vue'
import MixinMenuLayout from '@/layouts/MixinMenuLayout.vue'
import MobileDeviceLayout from '@/layouts/MobileDeviceLayout.vue'

const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = layoutThemeStore.layoutSetting
const layout_sidemenu = computed(() => layoutSetting.layout === 'sidemenu')
const layout_topmenu = computed(() => layoutSetting.layout === 'topmenu')
const layout_mixinmenu = computed(() => layoutSetting.layout === 'mixinmenu')
const onlyShowContent = computed(() => layoutSetting.onlyShowContent)
const systemStore = useSystemStore()
const device = computed(() => systemStore.device)
</script>

<template>
  <template v-if="!onlyShowContent">
    <template v-if="device === 'desktop'">
      <SideMenuLayout v-if="layout_sidemenu" />
      <TopMenuLayout v-else-if="layout_topmenu" />
      <MixinMenuLayout v-else-if="layout_mixinmenu" />
    </template>
    <template v-else-if="device === 'mobile'">
      <MobileDeviceLayout />
    </template>
  </template>
  <OnlyShowContentLayout v-else />
</template>

<style lang="less" scoped></style>
