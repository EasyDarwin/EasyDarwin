<script setup>
import { ref, computed } from 'vue'
import { RouterView } from 'vue-router'
import { useSystemStore } from '@/store/layout/system.js'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import LayoutSetting from '@/layouts/components/setting/LayoutSetting.vue'

const systemStore = useSystemStore()
const keepAliveList = computed(() => systemStore.keepAliveList)
const layoutSetting = useLayoutThemeStore().layoutSetting
const onlyShowContent = computed(() => layoutSetting.onlyShowContent)
const showAnimation = computed(() => layoutSetting.showAnimation)
const showTabs = computed(() => layoutSetting.showTabs)
const showHeader = computed(() => layoutSetting.showHeader)
const showSetting = computed(() => layoutSetting.showSetting)

const overflow = ref('auto')
const height = computed(() =>
  onlyShowContent.value
    ? '100vh'
    : `calc(100vh - ${layoutSetting.showHeader ? 'var(--app-header-height)' : '0px'} - ${layoutSetting.showTabs ? '50px' : '0px'} - ${layoutSetting.showFooter ? 'var(--app-footer-height)' : '0px'})`,
)

const enterActiveClass = computed(
  () =>
    `animate__animated animate__${layoutSetting.animation}In${layoutSetting.animationDirection === 'Default' ? '' : layoutSetting.animationDirection}`,
)
const leaveActiveClass = computed(
  () =>
    `animate__animated animate__${layoutSetting.animation}Out${layoutSetting.animationDirection === 'Default' ? '' : layoutSetting.animationDirection}`,
)
</script>

<template>
  <div
    class="p20px overflow-hidden"
    :style="{
      overflow,
      height,
    }"
  >
    <RouterView>
      <template #default="{ Component, route }">
        <Suspense>
          <Transition
            v-if="showAnimation"
            mode="out-in"
            appear
            :enter-active-class="enterActiveClass"
            :leave-active-class="leaveActiveClass"
            @before-enter="overflow = 'hidden'"
            @after-enter="overflow = 'auto'"
            @before-leave="overflow = 'hidden'"
          >
            <keep-alive :include="keepAliveList">
              <component :is="Component" :key="route.fullPath" />
            </keep-alive>
          </Transition>
          <template v-else>
            <keep-alive :include="keepAliveList">
              <component :is="Component" :key="route.fullPath" />
            </keep-alive>
          </template>
          <template #fallback> 正在加载... </template>
        </Suspense>
      </template>
    </RouterView>
    <a-card
      class="pos-fixed right--1 top-100 z-999"
      v-if="!showHeader && !showTabs && showSetting"
      :bodyStyle="{ padding: '10px' }"
    >
      <LayoutSetting />
    </a-card>
  </div>
</template>

<style lang="less" scoped></style>
