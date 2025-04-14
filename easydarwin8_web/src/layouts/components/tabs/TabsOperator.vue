<script setup>
import { computed, unref } from 'vue'
import { useI18n } from 'vue-i18n'
import { useRoute, useRouter } from 'vue-router'
import { useSystemStore } from '@/store/layout/system.js'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import { message } from 'ant-design-vue'
import { isFunction } from 'lodash-es'
import {
  DownOutlined,
  ReloadOutlined,
  CloseOutlined,
  VerticalRightOutlined,
  VerticalLeftOutlined,
  ColumnWidthOutlined,
  MinusOutlined,
} from '@ant-design/icons-vue'
import { Icon } from '@iconify/vue'

const props = defineProps({
  tabItem: {
    type: Object,
    required: true,
  },
  isExtra: {
    type: Boolean,
    default: false,
  },
})

const { t } = useI18n()
const route = useRoute()
const router = useRouter()
const systemStore = useSystemStore()
const isDevMode = import.meta.env.DEV

const activeKey = computed(() => systemStore.getCurrentTab?.fullPath)
const tabsList = computed(() => systemStore.getTabsList)

const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = layoutThemeStore.layoutSetting
const tabsIcon = computed(() => layoutSetting.tabsIcon)

// 目标路由是否与当前路由相同
const isCurrentRoute = route =>
  router.currentRoute.value.matched.some(item => item.name === route.name)

const reloadPage = () => {
  router.replace({
    name: 'Refresh',
    path: unref(route).fullPath,
  })
}
const removeTab = () => {
  if (tabsList.value.length === 1)
    return message.warning(t('message.lastPageCannotBeClosed'))
  systemStore.closeCurrentTab(props.tabItem)
}
defineExpose({
  removeTab,
})
const closeLeft = () => {
  systemStore.closeLeftTabs(props.tabItem)
  !isCurrentRoute(props.tabItem) && router.replace(props.tabItem.fullPath)
}
const closeRight = () => {
  systemStore.closeRightTabs(props.tabItem)
  !isCurrentRoute(props.tabItem) && router.replace(props.tabItem.fullPath)
}
const closeOther = () => {
  systemStore.closeOtherTabs(props.tabItem)
  !isCurrentRoute(props.tabItem) && router.replace(props.tabItem.fullPath)
}
const closeAll = () => {
  systemStore.closeAllTabs()
  router.replace('/')
}
/** 打开页面所在的文件(仅在开发环境有效) */
const openPageFile = async () => {
  if (!isDevMode) {
    console.warn('仅在开发环境有效')
    return
  }

  const routes = router.getRoutes()
  const target = routes.find(n => n.name === props.tabItem.name)
  if (target) {
    const comp = target.components?.default
    let __file = comp?.__file
    if (isFunction(comp)) {
      try {
        const res = await comp()
        __file = res?.default?.__file
      } catch (error) {
        console.log(error)
      }
    }
    if (__file) {
      const filePath = `/__open-in-editor?file=${__file}`
      console.log(filePath)
      await fetch(filePath)
    }
  }
}
</script>

<template>
  <a-dropdown :trigger="[isExtra ? 'click' : 'contextmenu']">
    <span v-if="isExtra" class="cursor-pointer" @click.prevent>
      <DownOutlined />
    </span>
    <div v-else style="display: inline-block">
      <template v-if="tabsIcon">
        <Icon class="iconify anticon" :icon="tabItem.meta.icon" />
      </template>
      <span>{{ t(tabItem.meta?.title) }}</span>
    </div>
    <template #overlay>
      <a-menu style="user-select: none">
        <a-menu-item
          key="1"
          :disabled="activeKey !== tabItem.fullPath"
          @click="reloadPage"
        >
          <ReloadOutlined />
          重新加载
        </a-menu-item>
        <a-menu-item key="2" @click="removeTab">
          <CloseOutlined />
          关闭标签页
        </a-menu-item>
        <a-divider class="m0" />
        <a-menu-item key="3" @click="closeLeft">
          <VerticalRightOutlined />
          关闭左侧标签页
        </a-menu-item>
        <a-menu-item key="4" @click="closeRight">
          <VerticalLeftOutlined />
          关闭右侧标签页
        </a-menu-item>
        <a-divider class="m0" />
        <a-menu-item key="5" @click="closeOther">
          <ColumnWidthOutlined />
          关闭其他标签页
        </a-menu-item>
        <a-menu-item key="6" @click="closeAll">
          <MinusOutlined />
          关闭全部标签页
        </a-menu-item>
        <template v-if="isDevMode">
          <a-divider class="m0" />
          <a-menu-item key="7" @click="openPageFile">
            <ColumnWidthOutlined />
            打开页面文件
          </a-menu-item>
        </template>
      </a-menu>
    </template>
  </a-dropdown>
</template>

<style lang="less" scoped></style>
