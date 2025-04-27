<script setup>
import { ref, computed } from 'vue'
import { message } from 'ant-design-vue'
import { SettingOutlined } from '@ant-design/icons-vue'
import { useI18n } from 'vue-i18n'
import {
  layouts,
  themeColors,
  uiSettings,
  animations,
} from '@/settings/layoutTheme.js'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import { useSystemStore } from '@/store/layout/system.js'

const { t } = useI18n()
const layoutThemeStore = useLayoutThemeStore()
const layoutSetting = computed(() => layoutThemeStore.layoutSetting)
const colorPrimary = computed(() => layoutThemeStore.layoutSetting.colorPrimary)
const colorPickerStyle = computed(() => ({
  '--custom-color': colorPrimary.value,
}))

const systemStore = useSystemStore()

const visible = ref(false)
const showDrawer = () => {
  visible.value = true
}

const setLayout = layout => {
  layoutThemeStore.updateLayoutSetting({ layout })
}

const setThemeColor = colorPrimary => {
  layoutThemeStore.updateLayoutSetting({ colorPrimary })
}

const getThemeColorVisible = color =>
  colorPrimary.value === color ? 'visible' : 'hidden'

const copySettings = () => {
  const settings = JSON.stringify(layoutSetting.value)
  navigator.clipboard.writeText(settings)
  message.success(t('message.copySuccess'))
}

const handleColorPickerInput = e => {
  setThemeColor(e.target.value)
}
</script>

<template>
  <a-tooltip :title="t('setting.projectConfig')" :placement="'bottomRight'">
    <SettingOutlined @click="showDrawer" />
  </a-tooltip>
  <a-drawer
    v-model:open="visible"
    placement="right"
    :title="t('setting.projectConfig')"
  >
    <a-descriptions :title="t('setting.menuLayout')" :column="5">
      <a-descriptions-item v-for="item in layouts" :key="item.value">
        <a-tooltip :title="t(item.label)">
          <div
            class="style-checkbox-item"
            :class="{ active: layoutSetting.layout === item.value }"
            @click="setLayout(item.value)"
          >
            <svg class="w60px h60px" aria-hidden="true">
              <use :xlink:href="`#svg-icon-${item.value}`" />
            </svg>
          </div>
        </a-tooltip>
      </a-descriptions-item>
    </a-descriptions>
    <a-descriptions
      :title="t('setting.themeColor')"
      :column="themeColors.length - 1"
    >
      <a-descriptions-item v-for="item in themeColors" :key="item.key">
        <div class="style-checkbox-item" v-if="item.tag === 'checkbox'">
          <a-tooltip :title="t(item.title)">
            <a-tag :color="item.value" @click="setThemeColor(item.value)">
              <span :style="{ visibility: getThemeColorVisible(item.value) }">
                ✔
              </span>
            </a-tag>
          </a-tooltip>
        </div>
        <a-flex
          justify="space-between"
          class="w-full flex items-center style-checkbox-item"
          v-if="item.tag === 'input-color'"
        >
          {{ t('setting.customColor') }}
          <a-tag :color="colorPrimary" class="relative overflow-hidden">
            <a-input
              type="color"
              class="cursor-pointer absolute top-0 left-0 w-full h-full"
              :style="colorPickerStyle"
              v-model="colorPrimary"
              @input="handleColorPickerInput"
            />
            <span :style="{ visibility: getThemeColorVisible(colorPrimary) }">
              ✔
            </span>
          </a-tag>
        </a-flex>
      </a-descriptions-item>
    </a-descriptions>
    <a-descriptions :title="t('setting.pageDisplay')" :column="1">
      <a-descriptions-item v-for="item in uiSettings" :key="item.value">
        <a-flex
          justify="space-between"
          class="w-full flex items-center"
          v-if="item.tag === 'switch'"
        >
          {{ t(item.label) }}
          <a-switch v-model:checked="layoutSetting[item.value]" />
        </a-flex>
        <a-flex
          justify="space-between"
          class="w-full flex items-center"
          v-if="item.tag === 'input-number'"
        >
          {{ t(item.label) }}
          <a-input-number
            style="width: 200px"
            v-model:value="layoutSetting[item.value]"
            :min="item.min"
            :max="item.max"
            :addon-after="t(item.unit)"
          />
        </a-flex>
        <a-flex
          justify="space-between"
          class="w-full flex items-center"
          v-if="item.tag === 'input'"
        >
          {{ t(item.label) }}
          <a-input
            style="width: 200px"
            v-model:value="layoutSetting[item.value]"
          />
        </a-flex>
        <a-flex
          justify="space-between"
          class="w-full flex items-center"
          v-if="item.tag === 'select'"
        >
          {{ t(item.label) }}
          <a-select
            style="width: 200px"
            v-if="item.value === 'animation'"
            v-model:value="layoutSetting.animation"
          >
            <a-select-option v-for="val in animations" :key="val.animation">
              {{ t(val.name) }}
            </a-select-option>
          </a-select>
          <a-select
            style="width: 200px"
            v-else-if="item.value === 'animationDirection'"
            v-model:value="layoutSetting.animationDirection"
          >
            <a-select-option
              v-for="val in animations.find(
                i => i.animation === layoutSetting.animation,
              ).options"
              :key="val"
            >
              {{ val }}
            </a-select-option>
          </a-select>
          <a-select
            v-else
            style="width: 200px"
            v-model:value="layoutSetting[item.value]"
          >
            <a-select-option
              v-for="val in item.options"
              :key="val.value"
              :value="val.value"
            >
              {{ t(val.label) }}
            </a-select-option>
          </a-select>
        </a-flex>
        <a-flex
          justify="space-between"
          class="w-full flex items-center"
          v-if="item.tag === 'segmented'"
        >
          {{ t(item.label) }}
          <a-segmented
            v-model:value="layoutSetting[item.value]"
            :options="
              item.options.map(option => ({
                value: option.value,
                label: t(option.label),
              }))
            "
          />
        </a-flex>
      </a-descriptions-item>
    </a-descriptions>
    <template #footer>
      <div class="flex-ac">
        <a-button class="w-45%" type="primary" @click="copySettings">
          {{ t('setting.copy') }}
        </a-button>
        <a-button
          class="w-45%"
          type="primary"
          danger
          @click="systemStore.clearCacheReload"
        >
          {{ t('setting.clearCacheAndReset') }}
        </a-button>
      </div>
    </template>
  </a-drawer>
</template>

<style lang="less" scoped>
.style-checkbox-item {
  position: relative;
  cursor: pointer;
  &.active::after {
    content: '✔';
    position: absolute;
    right: 12px;
    bottom: 10px;
  }
}
input[type='color'] {
  width: 40px;
  height: 40px;
  padding: 0;
  border: 0;
  outline: none;
  appearance: none;
  &::-webkit-color-swatch-wrapper {
    background: var(--custom-color);
  }
  &::-webkit-color-swatch {
    display: none;
  }
}
</style>
