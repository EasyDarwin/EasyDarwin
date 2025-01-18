<script setup>
import { ref, computed } from 'vue'
import { user } from "@/api";
import { useI18n } from 'vue-i18n'
import { message } from 'ant-design-vue'
import { UserOutlined, LockOutlined } from '@ant-design/icons-vue'
import { useUserStore } from '@/store/business/user.js'
import SliderVerify from '@/components/SliderVerify/index.vue'

const { t } = useI18n()
const userStore = useUserStore()
const formRef = ref(null)
const isSlider = ref(false)
const sliderVConf = ref({
    captcha_id: 0,
    master: "",
    tile: "",
    w: 0,
    h: 0,
    x: 0,
    y: 0,
    expired: 0
})
const formState = ref({
  username: '',
  password: '',
  captcha: "",
  captcha_id: 0
})
const getCaptcha = () => {
  user.captcha({}).then(res=>{
    if (res.status == 200) {
      sliderVConf.value = res.data
    }
  })
}
const emitChangeOk = (value) => {
  formState.value.captcha_id = sliderVConf.value.captcha_id
  formState.value.captcha = `${value},${sliderVConf.value.y}`
  isSlider.value = false
  onFinish()
}
const emitChangeClose = () => {
    isSlider.value = false
}
const emitChangeReload = () => {
    getCaptcha()
}
const onChangeOpen = () => {
  if (sliderVConf.value.captcha_id==0) {
    onFinish()
  } else {
    isSlider.value = true
  }
}
const onFinish = () => {
  formRef.value
    .validate()
    .then(() => {
      userStore.login(formState.value).catch(err=>{
        getCaptcha()
      })
    })
    .catch(error => {
      console.log('error', error);
    });
  // message.success(t('message.loginSuccess'))
}
const onFinishFailed = errorInfo => {
  console.log('Failed:', errorInfo)
}

getCaptcha()

</script>

<template>
  <a-form ref="formRef" :model="formState" @finishFailed="onFinishFailed">
    <a-form-item name="username" :rules="[{ required: true, message: t('message.pleaseEnterUsername') }]">
      <a-input v-model:value="formState.username"  placeholder="admin">
        <template #prefix>
          <UserOutlined />
        </template>
      </a-input>
    </a-form-item>
    <a-form-item name="password" :rules="[{ required: true, message: t('message.pleaseEnterPassword') }]">
      <a-input-password v-model:value="formState.password" placeholder="admin" @pressEnter="onChangeOpen">
        <template #prefix>
          <LockOutlined />
        </template>
      </a-input-password>
    </a-form-item>
    <a-form-item>
      <a-popconfirm :open="isSlider">
        <template #cancelButton></template>
        <template #okButton></template>
        <template #icon></template>
        <template #title>
          <SliderVerify
            v-if="sliderVConf.captcha_id!=0"
            :imgL="sliderVConf.master"
            :imgX="sliderVConf.tile"
            :width="sliderVConf.width"
            :height="sliderVConf.height"
            :w="sliderVConf.w"
            :h="sliderVConf.h"
            :top="sliderVConf.y"
            @reload="emitChangeReload"
            @close="emitChangeClose"
            @ok="emitChangeOk"
          ></SliderVerify>
        </template>
        <a-button class="w100%" type="primary"  @click="onChangeOpen">
          {{ t('setting.login') }}
        </a-button>
      </a-popconfirm>
    </a-form-item>
  </a-form>
</template>

<style scoped lang="less"></style>
