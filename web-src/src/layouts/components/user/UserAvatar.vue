<script setup>
import { ref, computed, onMounted } from 'vue'
import { useI18n } from 'vue-i18n'
import { useUserStore } from '@/store/business/user.js'
import ChangePassword from './ChangePassword.vue'
const { t } = useI18n()
const userStore = useUserStore()
const userInfo = computed(() => userStore.userInfo)
const open = ref(false)
// 仍在使用默认口令时强制修改密码（CNVD-2026-42897）
const forceChange = computed(() => !!userInfo.value.need_change_password)

onMounted(() => {
  if (forceChange.value) {
    open.value = true
  }
})

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
    <a-modal v-model:open="open" :title="t('setting.changePassword')"
      :closable="!forceChange" :maskClosable="!forceChange" :keyboard="!forceChange">
      <template #footer>
      </template>
      <a-alert v-if="forceChange" type="warning" show-icon class="mb-4"
        :message="t('setting.forceChangePasswordTip')" />
      <ChangePassword />
    </a-modal>
  </div>
</template>

<style scoped lang="less"></style>
