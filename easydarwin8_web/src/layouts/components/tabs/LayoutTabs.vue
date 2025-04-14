<script setup>
import { ref, computed, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { LockOutlined, ReloadOutlined } from '@ant-design/icons-vue'
import { useSystemStore } from '@/store/layout/system.js'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import TabsOperator from '@/layouts/components/tabs/TabsOperator.vue'
import FullScreen from '@/layouts/components/fullscreen/FullScreen.vue'
import LayoutSetting from '@/layouts/components/setting/LayoutSetting.vue'
import SearchMenu from '@/layouts/components/searchMenu/SearchMenu.vue'

const route = useRoute()
const router = useRouter()

const systemStore = useSystemStore()
const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = layoutThemeStore.layoutSetting
const showHeader = computed(() => layoutSetting.showHeader)
const showTabs = computed(() => layoutSetting.showTabs)
const showSearchMenu = computed(() => layoutSetting.showSearchMenu)
const showFullScreen = computed(() => layoutSetting.showFullScreen)
const showLockScreen = computed(() => layoutSetting.showLockScreen)
const showSetting = computed(() => layoutSetting.showSetting)
const showRefreshReset = computed(() => layoutSetting.showRefreshReset)
const activeKey = ref(route.fullPath)

watch(
  () => route.fullPath,
  val => {
    activeKey.value = val
  },
)

const itemRefs = {}
const changePage = key => {
  Object.is(route.fullPath, key) || router.push(key)
}
const editTabItem = (targetKey, action) => {
  if (action === 'remove') {
    itemRefs[targetKey]?.removeTab()
  }
}

const background = computed(() =>
  layoutSetting.algorithm === 'darkAlgorithm' ? 'black' : 'white',
)
const borderRadius = computed(() => `${layoutSetting.borderRadius}px`)
const border = computed(() =>
  layoutSetting.algorithm === 'darkAlgorithm'
    ? '1px solid black'
    : '1px solid #eee',
)

const tabBarStyle = ref({
  margin: 0,
  padding: '5px 20px',
  userSelect: 'none',
  background,
  '--border-radius': borderRadius,
  '--border': border,
})
</script>

<template>
  <div class="tabs-view">
    <a-tabs
      class="tabs"
      type="editable-card"
      v-if="showTabs"
      v-model:activeKey="activeKey"
      :hideAdd="true"
      :tabBarGutter="5"
      :tabBarStyle="tabBarStyle"
      @change="changePage"
      @edit="editTabItem"
    >
      <a-tab-pane
        v-for="tabItem in systemStore.getTabsList"
        :key="tabItem.fullPath"
      >
        <template #tab>
          <TabsOperator
            :ref="ins => (itemRefs[tabItem.fullPath] = ins)"
            :tabItem="tabItem"
          />
        </template>
      </a-tab-pane>
      <template #rightExtra>
        <a-space>
          <template #split>
            <a-divider type="vertical" />
          </template>
          <TabsOperator :tabItem="route" :isExtra="true" />
          <SearchMenu v-if="!showHeader && showSearchMenu" />
          <a-tooltip title="锁屏" v-if="!showHeader && showLockScreen">
            <LockOutlined @click="systemStore.setLockScreenState(true)" />
          </a-tooltip>
          <FullScreen v-if="!showHeader && showFullScreen" />
          <a-tooltip title="刷新重置" v-if="!showHeader && showRefreshReset">
            <ReloadOutlined @click="systemStore.clearCacheReload()" />
          </a-tooltip>
          <LayoutSetting v-if="!showHeader && showSetting" />
        </a-space>
      </template>
    </a-tabs>
    <slot></slot>
  </div>
</template>

<style lang="less" scoped>
.tabs-view {
  :deep(.tabs) {
    .ant-tabs-nav {
      .ant-tabs-tab {
        border-radius: var(--border-radius);
        border: var(--border);
      }
    }
  }
}
</style>
