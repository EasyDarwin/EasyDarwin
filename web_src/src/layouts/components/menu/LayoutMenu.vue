<script setup>
import { ref, computed, watch, nextTick } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import { useSystemStore } from '@/store/layout/system.js'
import SubMenuItem from '@/layouts/components/menu/SubMenuItem.vue'

const props = defineProps({
  collapsed: {
    type: Boolean,
  },
})

const currentRoute = useRoute()
const router = useRouter()

const selectedKeys = ref([])
const openKeys = ref([])

const systemStore = useSystemStore()
const menus = computed(() => systemStore.menus)
const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = layoutThemeStore.layoutSetting
const layout_topmenu = computed(() => layoutSetting.layout === 'topmenu')
const menuTheme = computed(() => layoutSetting.menuTheme)

// Get the currently open submenu
const getOpenKeys = () =>
  currentRoute.meta.namePath ??
  currentRoute.matched.slice(1).map(item => item.name)

const getRouteByName = name =>
  router.getRoutes().find(item => item.name === name)

// Listening for menu contraction status
watch(
  () => props.collapsed,
  () => {
    selectedKeys.value = currentRoute.name ? [currentRoute.name] : []
    openKeys.value = getOpenKeys()
  },
)

// Toggle menu selection state to follow page routing changes
watch(
  () => currentRoute.fullPath,
  () => {
    selectedKeys.value = currentRoute.name ? [currentRoute.name] : []
    openKeys.value = getOpenKeys()
  },
  {
    immediate: true,
  },
)

const clickMenuItem = ({ key }) => {
  if (key === currentRoute.name) return
  const preSelectedKeys = selectedKeys.value
  const targetRoute = getRouteByName(key)
  const { outsideLink } = targetRoute?.meta || {}
  if (targetRoute && outsideLink) {
    nextTick(() => {
      selectedKeys.value = preSelectedKeys
    })
  }
}
</script>

<template>
  <a-menu
    class="border-none!"
    v-model:selected-keys="selectedKeys"
    :open-keys="!layout_topmenu ? openKeys : []"
    :mode="!layout_topmenu ? 'inline' : 'horizontal'"
    :theme="menuTheme"
    :collapsed="collapsed"
    collapsible
    @click="clickMenuItem"
  >
    <template v-for="item in menus" :key="item.name">
      <SubMenuItem :item="item" />
    </template>
  </a-menu>
</template>

<style lang="less" scoped></style>
