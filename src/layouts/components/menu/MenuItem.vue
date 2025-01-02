<script setup>
import { useRouter } from 'vue-router'
import MenuItemContent from '@/layouts/components/menu/MenuItemContent.vue'

defineProps({
  item: {
    type: Object,
    default: () => ({}),
  },
})

const router = useRouter()
const handleMenuItemClick = item => {
  const { outsideLink } = item.meta || {}
  if (outsideLink) {
    window.open(item.path)
  } else {
    router.push({ name: item.name })
  }
}
</script>

<template>
  <a-menu-item :key="item?.name" @click="handleMenuItemClick(item)">
    <MenuItemContent :item="item" />
  </a-menu-item>
</template>

<style lang="less" scoped></style>
