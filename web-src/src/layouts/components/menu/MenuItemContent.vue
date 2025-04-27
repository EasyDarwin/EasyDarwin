<script setup>
import { useI18n } from 'vue-i18n'
import { Icon } from '@iconify/vue'

defineProps({
  item: {
    type: Object,
    default: () => ({}),
  },
})

const { t } = useI18n()
</script>

<template>
  <template v-if="item?.meta?.icon">
    <Icon class="iconify anticon" :icon="item.meta.icon" />
  </template>
  <span class="ml10px text-14px" :title="t(item?.meta?.title)">
    {{ t(item.meta.title) }}
  </span>
</template>

<style lang="less" scoped></style>
