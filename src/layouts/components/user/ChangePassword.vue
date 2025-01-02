<script setup>
import { ref } from 'vue'
import { useI18n } from 'vue-i18n'
import { message } from 'ant-design-vue'
import { LockOutlined } from '@ant-design/icons-vue'
import { useUserStore } from '@/store/business/user.js'
const { t } = useI18n()
const userStore = useUserStore()
const formState = ref({
  password: "",
  new_password: "",
  confirm_password: "",
})

const onFinish = () => {
  if (formState.value.new_password!=formState.value.confirm_password) {
    message.error(t('message.TwoDifferentPasswords'))
    return
  }
  userStore.changePassword(formState.value).then(()=>{
    message.success(`${t('message.modifySuccess')},${t('message.jumpTo3')}`)
    setTimeout(() => {
      userStore.logout()
    }, 3*1000);
  })
}
const onFinishFailed = errorInfo => {
  console.log('Failed:', errorInfo)
}
</script>

<template>
  <a-form :model="formState"  @finish="onFinish" @finishFailed="onFinishFailed">
  
    <a-form-item name="password" :rules="[{ required: true, message: t('message.PleaseEnterOriginalPassword') }]">
      <a-input-password v-model:value="formState.password" :placeholder="t('message.PleaseEnterOriginalPassword')">
        <template #prefix>
          <LockOutlined />
        </template>
      </a-input-password>
    </a-form-item>
    <a-form-item name="new_password" :rules="[{ required: true, message: t('message.pleaseEnterNewPassword') },
    {min: 6,message: t('message.PasswordLength') ,trigger: 'blur'}]">
      <a-input-password v-model:value="formState.new_password" :placeholder="t('message.pleaseEnterNewPassword')">
        <template #prefix>
          <LockOutlined />
        </template>
      </a-input-password>
    </a-form-item>
    <a-form-item name="confirm_password" :rules="[{ required: true, message: t('message.PleaseEnterConfirmPassword')},
    {min: 6,message: t('message.PasswordLength') ,trigger: 'blur'}]">
      <a-input-password v-model:value="formState.confirm_password" :placeholder="t('message.PleaseEnterConfirmPassword')">
        <template #prefix>
          <LockOutlined />
        </template>
      </a-input-password>
    </a-form-item>
    <a-form-item>
        <a-button class="w100%" type="primary" html-type="submit">
          {{ t('setting.confirm') }}
        </a-button>
    </a-form-item>
  </a-form>
</template>

<style scoped lang="less"></style>
