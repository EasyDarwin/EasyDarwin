<script setup>
import { ref, shallowRef, computed, watch, nextTick } from 'vue'
import { useRouter } from 'vue-router'
import { useDebounceFn, onKeyStroke } from '@vueuse/core'
import { useI18n } from 'vue-i18n'
import { Icon } from '@iconify/vue'
import { SearchOutlined } from '@ant-design/icons-vue'
import { useSystemStore } from '@/store/layout/system.js'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'

const router = useRouter()
const { t } = useI18n()
const systemStore = useSystemStore()
const menuList = computed(() => systemStore.menuList)
const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = computed(() => layoutThemeStore.layoutSetting)

const modalOpen = ref(false)
const keyword = ref('')
const resultOptions = shallowRef([])
const inputRef = ref(null)
const activeKey = ref('')

watch(
  () => modalOpen.value,
  () => {
    if (modalOpen.value) {
      nextTick(() => {
        inputRef.value.focus()
      })
    }
  },
)

const handleSearchMenu = useDebounceFn(() => searchMenu(), 300)
const searchMenu = () => {
  if (keyword.value) {
    resultOptions.value = menuList.value.filter(item =>
      item.searchTitle
        ?.toLocaleLowerCase()
        .includes(keyword.value.toLocaleLowerCase().trim()),
    )
    activeKey.value = resultOptions.value[0]?.name
  } else {
    resultOptions.value = []
    activeKey.value = ''
  }
}

const handleEnter = () => {
  if (activeKey.value) {
    router.push({ name: activeKey.value })
    modalOpen.value = false
    keyword.value = ''
    resultOptions.value = []
  }
}

onKeyStroke('Enter', handleEnter)
onKeyStroke('Escape', () => {
  modalOpen.value = false
  keyword.value = ''
  resultOptions.value = []
})
onKeyStroke('ArrowUp', () => {
  const { length } = resultOptions.value
  if (!length) return
  const index = resultOptions.value.findIndex(
    item => item.name === activeKey.value,
  )
  if (index === 0) {
    activeKey.value = resultOptions.value[length - 1].name
  } else {
    activeKey.value = resultOptions.value[index - 1].name
  }
})
onKeyStroke('ArrowDown', () => {
  const { length } = resultOptions.value
  if (!length) return
  const index = resultOptions.value.findIndex(
    item => item.name === activeKey.value,
  )
  if (index === length - 1) {
    activeKey.value = resultOptions.value[0].name
  } else {
    activeKey.value = resultOptions.value[index + 1].name
  }
})
</script>

<template>
  <a-tooltip :title="t('setting.searchMenu')">
    <SearchOutlined @click="modalOpen = true" />
    <a-modal v-model:open="modalOpen" :closable="false" :keyboard="false">
      <a-input
        ref="inputRef"
        size="large"
        placeholder="搜索菜单"
        v-model:value="keyword"
        @change="handleSearchMenu"
      >
        <template #prefix>
          <SearchOutlined />
        </template>
      </a-input>
      <div class="max-h600px overflow-auto" v-if="resultOptions.length > 0">
        <template v-for="item in resultOptions" :key="item.path">
          <a-card
            class="shadow mt10px cursor-pointer"
            :bodyStyle="{ padding: '18px' }"
            :style="
              activeKey === item.name
                ? { background: layoutSetting.colorPrimary, color: '#fff' }
                : {}
            "
            @mouseenter="activeKey = item.name"
            @click="handleEnter"
          >
            <Icon class="iconify anticon mr10px" :icon="item.meta.icon" />
            {{ item.searchTitle }}
            <Icon
              v-if="activeKey === item.name"
              class="float-right mt5px"
              icon="mdi:keyboard-return"
            />
          </a-card>
        </template>
      </div>
      <a-empty v-else class="p20px m0" />
      <template #footer>
        <div class="flex-bc">
          <a-space :size="20">
            <a-space-compact class="flex items-center">
              <Icon class="shadow mr8px" icon="mdi:keyboard-return" />
              {{ t('setting.confirm') }}
            </a-space-compact>
            <a-space-compact class="flex items-center">
              <Icon class="shadow mr8px" icon="mdi:keyboard-arrow-up" />
              <Icon class="shadow mr8px" icon="mdi:keyboard-arrow-down" />
              {{ t('setting.toggle') }}
            </a-space-compact>
            <a-space-compact class="flex items-center">
              <Icon class="shadow mr8px" icon="mdi:keyboard-esc" />
              {{ t('setting.close') }}
            </a-space-compact>
          </a-space>
          <span v-if="resultOptions.length > 0"
            >共{{ resultOptions.length }}项</span
          >
        </div>
      </template>
    </a-modal>
  </a-tooltip>
</template>

<style lang="less" scoped></style>
