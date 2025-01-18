<script setup>
import SubMenuItem from '@/layouts/components/menu/SubMenuItem.vue'
import MenuItem from '@/layouts/components/menu/MenuItem.vue'
import MenuItemContent from '@/layouts/components/menu/MenuItemContent.vue'

defineProps({
  item: {
    type: Object,
    default: () => ({}),
  },
})

const isShowSubMenu = menuItem => menuItem?.children?.length
// isShowSubMenu(item)
</script>

<template>
  <!-- 目录 -->
  <a-sub-menu v-if="false" v-bind="$attrs" :key="item?.name">
    <template #title>
      <MenuItemContent :item="item" />
    </template>
    <template v-for="child in item.children || []" :key="child.name">
      <SubMenuItem :item="child" />
    </template>
  </a-sub-menu>
  <!-- 菜单 -->
  <MenuItem v-else :item="item" />
</template>

<style lang="less" scoped></style>
