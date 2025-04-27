<script setup>
import { computed,ref } from 'vue'

import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import LayoutSetting from '@/layouts/components/setting/LayoutSetting.vue'
import LoginForm from '@/views/login/LoginForm.vue'

const layoutSetting = useLayoutThemeStore().layoutSetting
const showSetting = computed(() => layoutSetting.showSetting)
const algorithm = computed(() => layoutSetting.algorithm)
const title = computed(() => layoutSetting.title)


const bodyStyle = computed(() => ({
  width: '100%',
  height: '100%',
  backgroundColor: algorithm.value === 'darkAlgorithm' ? '#001529' : '#eee',
  position: 'relative',
}))

</script>

<template>
  <a-card class="w100vw h100vh" :bodyStyle="bodyStyle">
    <a-card
      class="pos-absolute w400px"
      :style="{
        top: 'calc(50% - 240px)',
        left: 'calc(50% - 200px)',
      }"
    >
      <template #title>
        <div class="text-center " style="font-size: 20px;">{{ title }}</div>
      </template>
      <LoginForm />
    </a-card>
  </a-card>
</template>

<style lang="less" scoped></style>
