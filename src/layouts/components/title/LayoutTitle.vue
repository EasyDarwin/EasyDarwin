<script setup>
import { computed } from 'vue'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import { useRouter } from 'vue-router'
import { useBaseStore } from '@/store/business/base.js'
const baseStore = useBaseStore()
const router = useRouter()
defineProps({
  collapsed: {
    type: Boolean,
  },
})
const version = computed(() => baseStore.serverInfo&&baseStore.serverInfo.version)
const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = layoutThemeStore.layoutSetting
const title = computed(() => layoutSetting.title)
const titleColor = computed(() => layoutThemeStore.titleColor)

const style = computed(() => {
  return {
    height: `var(--app-header-height)`,
    color: titleColor.value,
  }
})
const onRouter = ()=>{
  router.push({name:"Live"})
}
baseStore.getServerInfo()
</script>

<template>
  <div
    class="flex-cc overflow-hidden whitespace-nowrap font-500 text-20px cp"
    :style="style"
  >
    <span v-show="collapsed">E</span>
    <div v-show="!collapsed" :class="[{'pr24px':!collapsed}]" @click="onRouter">
      {{ title }}<sup>{{ version }}</sup>
    </div>
  </div>
</template>

<style lang="less" scoped></style>
