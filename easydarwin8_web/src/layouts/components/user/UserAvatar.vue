<script setup>
import { ref, computed } from 'vue'
import { useI18n } from 'vue-i18n'
import { useUserStore } from '@/store/business/user.js'
import ChangePassword from './ChangePassword.vue'
const { t } = useI18n()
const userStore = useUserStore()
const userInfo = computed(() => userStore.userInfo)
const open = ref(false)

const logout = () => {
  userStore.logout()
}
const onChangePassword = () => {
  open.value = true
}
</script>

<template>
  <div>
    <a-dropdown class="cp">
      <template #overlay>
        <a-menu>
          <a-menu-item @click="onChangePassword">{{ t('setting.changePassword') }}</a-menu-item>
          <a-menu-item @click="logout">{{ t('setting.logout') }}</a-menu-item>
        </a-menu>
      </template>
      <span>{{ userInfo.user && userInfo.user.username || "admin" }}</span>
    </a-dropdown>
    <a-modal v-model:open="open" :title="t('setting.changePassword')">
      <template #footer>
      </template>
      <ChangePassword />
    </a-modal>
  </div>
</template>

<style scoped lang="less"></style>
