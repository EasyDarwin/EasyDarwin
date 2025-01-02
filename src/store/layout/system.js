import { defineStore } from 'pinia'
import { ref, computed, watch } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import { useI18n } from 'vue-i18n'
import {
  useIntervalFn,
  useBattery,
  useOnline,
  useResizeObserver,
} from '@vueuse/core'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import dayjs from 'dayjs'

export const useSystemStore = defineStore(
  'system',
  () => {
    const { t } = useI18n()
    const router = useRouter()
    const currentRoute = useRoute()
    const layoutThemeStore = useLayoutThemeStore()

    const menus = ref(
      router.options.routes
        .find(item => item.name === 'Layout')
        .children.filter(item => item.name !== 'Refresh'),
    )

    // menus 平铺
    const menuList = ref([])
    const getMenuList = (menu, parent) => {
      
      menu.forEach(item => {
        item.searchTitle = t(item.meta.title)
        if (item.children) {
          getMenuList(item.children, item)
        } else {
          if (parent) {
            item.searchTitle = `${parent.searchTitle} / ${t(item.meta.title)}`
          }
          menuList.value.push(item)
        }
      })
    }
    getMenuList(menus.value)

    const updateMenus = (type, route) => {
      switch (type) {
        case 'add':
          menus.value.push(route)
          break
        case 'delete':
          menus.value = menus.value.filter(item => item.name !== route.name)
          break
        default:
          break
      }

      menuList.value = []
      getMenuList(menus.value)
    }

    watch(
      () => menus.value,
      () => {
        getMenuList(menus.value)
      },
    )

    const tabsList = ref([])
    const getTabsList = computed(() =>
      tabsList.value.filter(item => router.hasRoute(item.name)),
    )
    const getCurrentTab = computed(() =>
      tabsList.value.find(item => item.fullPath === currentRoute.fullPath),
    )

    let keepAliveList = ref([])
    const addKeepAliveList = name => {
      if (typeof name === 'string') {
        !keepAliveList.value.includes(name) && keepAliveList.value.push(name)
      } else {
        name.forEach(item => {
          item &&
            !keepAliveList.value.includes(item) &&
            keepAliveList.value.push(item)
        })
      }
    }
    const removeKeepAliveList = name => {
      if (typeof name === 'string') {
        keepAliveList.value = keepAliveList.value.filter(item => item !== name)
      } else {
        keepAliveList.value = keepAliveList.value.filter(
          item => !name.includes(item),
        )
      }
    }
    const clearKeepAliveList = () => {
      keepAliveList.value = []
    }

    // 获取原始路由信息
    const getRawRoute = route => ({
      ...route,
      matched: route.matched.map(item => {
        const { meta, path, name } = item
        return { meta, path, name }
      }),
    })
    // 将已关闭的标签页的组件从keep-alive中移除
    const delCompFromClosedTabs = closedTabs => {
      const routes = router.getRoutes()
      const compNames = closedTabs.reduce((acc, cur) => {
        if (cur.name && router.hasRoute(cur.name)) {
          const componentName = routes.find(item => item.name === cur.name)
            ?.components?.default?.name
          componentName && acc.push(componentName)
        }
        return acc
      }, [])
      removeKeepAliveList(compNames)
    }
    // 添加tabs
    const addTabs = route => {
      const isExist = tabsList.value.some(
        item => item.fullPath === route.fullPath,
      )
      if (!isExist) {
        tabsList.value.push(getRawRoute(route))
      }
    }
    // 关闭当前tab
    const closeCurrentTab = route => {
      const index = tabsList.value.findIndex(
        item => item.fullPath === route.fullPath,
      )
      const isDelCurrentTab = Object.is(
        getCurrentTab.value,
        tabsList.value[index],
      )
      delCompFromClosedTabs(tabsList.value.splice(index, 1))
      // 如果关闭的tab就是当前激活的tab，则重定向页面
      if (isDelCurrentTab) {
        const currentRoute =
          tabsList.value[Math.max(0, tabsList.value.length - 1)]
        router.push(currentRoute)
      }
    }
    // 关闭左侧tabs
    const closeLeftTabs = route => {
      const index = tabsList.value.findIndex(
        item => item.fullPath === route.fullPath,
      )
      delCompFromClosedTabs(tabsList.value.splice(0, index))
    }
    // 关闭右侧tabs
    const closeRightTabs = route => {
      const index = tabsList.value.findIndex(
        item => item.fullPath === route.fullPath,
      )
      delCompFromClosedTabs(tabsList.value.splice(index + 1))
    }
    // 关闭其他tabs
    const closeOtherTabs = route => {
      const index = tabsList.value.findIndex(
        item => item.fullPath === route.fullPath,
      )
      if (index !== -1) {
        const current = tabsList.value.splice(index, 1)
        delCompFromClosedTabs(tabsList.value)
        tabsList.value = current
      }
    }
    // 关闭所有tabs
    const closeAllTabs = () => {
      delCompFromClosedTabs(tabsList.value)
      tabsList.value = []
    }

    watch(
      () => currentRoute.fullPath,
      () => {
        const whiteList = ['Login', 'NotFound', 'Refresh', undefined]
        if (whiteList.includes(currentRoute.name)) return
        addTabs(currentRoute)
      },
      {
        immediate: true,
      },
    )

    window.addEventListener('beforeunload', () => {
      if (!layoutThemeStore.layoutSetting.cacheTabs) {
        tabsList.value = [getCurrentTab.value || tabsList.value[0]].filter(
          Boolean,
        )
      }
    })

    const systemTime = ref({})
    const weekArrZh_Cn = ['日', '一', '二', '三', '四', '五', '六']
    const weekArrEn = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat']
    useIntervalFn(() => {
      systemTime.value = {
        year: dayjs().year(),
        month: dayjs().month() + 1,
        day: dayjs().date(),
        hour: dayjs().hour(),
        minute: dayjs().minute(),
        second: dayjs().second(),
        week:
          layoutThemeStore.layoutSetting.language === 'zhCN'
            ? `星期${weekArrZh_Cn[dayjs().day()]}`
            : weekArrEn[dayjs().day()],
      }
    })

    const lockScreenPassword = ref('')
    const lockScreenState = ref(false)
    const lockScreenTime = computed(
      () => layoutThemeStore.layoutSetting.lockScreenTime * 60 * 60,
    )
    const lockScreenCounter = ref(0)
    setInterval(() => {
      lockScreenCounter.value += 1
    }, 1000)

    const setLockScreenState = state => {
      lockScreenState.value = state
      if (!state) {
        resetLockScreenPassword()
        lockScreenCounter.value = 0
      }
    }
    const setLockScreenPassword = password => {
      lockScreenPassword.value = encodeURI(password)
    }
    const resetLockScreenPassword = () => {
      lockScreenPassword.value = ''
    }
    const verifyLockScreenPassword = password =>
      encodeURI(password) === lockScreenPassword.value
    // 解码
    const decodeLockScreenPassword = () => decodeURI(lockScreenPassword.value)

    watch(lockScreenTime, () => {
      lockScreenCounter.value = 0
    })
    watch(lockScreenCounter, () => {
      if (
        lockScreenCounter.value >= lockScreenTime.value &&
        layoutThemeStore.layoutSetting.showLockScreen
      ) {
        setLockScreenState(true)
        setLockScreenPassword('123456') // 默认密码
      }
    })

    const { charging, chargingTime, dischargingTime, level } = useBattery()
    const battery = ref({
      charging,
      chargingTime,
      dischargingTime,
      level,
    })

    const online = useOnline()

    const device = ref('desktop')
    useResizeObserver(document.body, entries => {
      const entry = entries[0]
      const { width } = entry.contentRect
      device.value = width < 992 ? 'mobile' : 'desktop'
    })

    const clearCacheReload = () => {
      localStorage.clear()
      sessionStorage.clear()
      location.reload()

      menus.value = router.options.routes
        .find(item => item.name === 'Layout')
        .children.filter(item => item.name !== 'Refresh')

      menuList.value = []
      getMenuList(menus.value)

      lockScreenState.value = false
      lockScreenPassword.value = ''

      tabsList.value = []
      keepAliveList.value = []
    }

    return {
      menus,
      menuList,
      tabsList,
      getTabsList,
      getCurrentTab,
      keepAliveList,
      systemTime,
      lockScreenState,
      lockScreenPassword,
      battery,
      online,
      device,
      updateMenus,
      addTabs,
      closeCurrentTab,
      closeLeftTabs,
      closeRightTabs,
      closeOtherTabs,
      closeAllTabs,
      addKeepAliveList,
      removeKeepAliveList,
      clearKeepAliveList,
      setLockScreenState,
      setLockScreenPassword,
      verifyLockScreenPassword,
      decodeLockScreenPassword,
      clearCacheReload,
    }
  },
)
