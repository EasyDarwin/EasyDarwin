<script setup>
import { computed } from 'vue'
import { useI18n } from 'vue-i18n'
import {
  MenuFoldOutlined,
  MenuUnfoldOutlined,
  LockOutlined,
  SyncOutlined,
} from '@ant-design/icons-vue'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import { useSystemStore } from '@/store/layout/system.js'
import LayoutBreadcrumb from '@/layouts/components/breadcrumb/LayoutBreadcrumb.vue'
import LayoutSetting from '@/layouts/components/setting/LayoutSetting.vue'
import FullScreen from '@/layouts/components/fullscreen/FullScreen.vue'
import SearchMenu from '@/layouts/components/searchMenu/SearchMenu.vue'
import LocaleLanguage from '@/layouts/components/locale/LocaleLanguage.vue'
import UserAvatar from '@/layouts/components/user/UserAvatar.vue'

defineProps({
  collapsed: {
    type: Boolean,
  },
})

const { t } = useI18n()
const systemStore = useSystemStore()
const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = layoutThemeStore.layoutSetting
const layout_sidemenu = computed(() => layoutSetting.layout === 'sidemenu')
const layout_topmenu = computed(() => layoutSetting.layout === 'topmenu')
const layout_mixinmenu = computed(() => layoutSetting.layout === 'mixinmenu')
const showBreadcrumb = computed(() => layoutSetting.showBreadcrumb)
const showSearchMenu = computed(() => layoutSetting.showSearchMenu)
const showLockScreen = computed(() => layoutSetting.showLockScreen)
const showFullScreen = computed(() => layoutSetting.showFullScreen)
const showSetting = computed(() => layoutSetting.showSetting)
const showRefreshReset = computed(() => layoutSetting.showRefreshReset)
const locale = computed(() => layoutSetting.locale)
const sidemenuWidth = computed(() => layoutSetting.sidemenuWidth)
const headerBackground = computed(() => layoutThemeStore.headerBackground)
const headerColor = computed(() => layoutThemeStore.headerColor)
const border = computed(() => layoutThemeStore.border)

const emit = defineEmits(['update:collapsed'])

const layoutHeaderStyle = computed(() => {
  return {
    height: 'var(--app-header-height)',
    padding: layout_sidemenu.value ? '0 20px' : '0 20px 0 0',
    background: headerBackground.value,
    color: headerColor.value,
    borderBottom: border.value,
  }
})
</script>

<template>
  <a-layout-header class="flex-bc" :style="layoutHeaderStyle">
    <div :style="{ width: `${sidemenuWidth}px` }" v-if="!layout_sidemenu">
      <slot name="title"></slot>
    </div>
    <div
      :style="{ paddingLeft: layout_mixinmenu ? '20px' : 0 }"
      v-if="!layout_topmenu"
    >
      <slot name="left"> </slot>
      <a-space :size="20" v-if="!layout_topmenu">
        <span
          class="cursor-pointer"
          @click="() => emit('update:collapsed', !collapsed)"
        >
          <component :is="collapsed ? MenuUnfoldOutlined : MenuFoldOutlined" />
        </span>
        <LayoutBreadcrumb v-if="showBreadcrumb" />
      </a-space>
    </div>
    <div class="flex-cc flex-1">
      <slot name="menu" />
    </div>
    <div>
      <a-space :size="20">
        <!-- <SearchMenu v-if="showSearchMenu" /> -->
        <!-- <a-tooltip :title="t('setting.lockScreen')" v-if="showLockScreen">
          <LockOutlined @click="systemStore.setLockScreenState(true)" />
        </a-tooltip> -->
        <!-- <FullScreen v-if="showFullScreen" /> -->
        <!-- <a-tooltip :title="t('setting.refreshReset')" v-if="showRefreshReset">
          <SyncOutlined @click="systemStore.clearCacheReload()" />
        </a-tooltip> -->
        <!-- <LocaleLanguage v-if="locale" /> -->
        <UserAvatar />
        <!-- <LayoutSetting v-if="showSetting" /> -->
      </a-space>
    </div>
  </a-layout-header>
</template>

<style lang="less" scoped></style>
