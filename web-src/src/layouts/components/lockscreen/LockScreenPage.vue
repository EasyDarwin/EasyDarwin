<script setup>
import { ref, computed } from 'vue'
import { useI18n } from 'vue-i18n'
import { LockOutlined } from '@ant-design/icons-vue'
import { useSystemStore } from '@/store/layout/system.js'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'
import { message } from 'ant-design-vue'
import { Icon } from '@iconify/vue'
import ChargeOne from '@/layouts/components/lockscreen/ChargeOne.vue'
import ChargeTwo from '@/layouts/components/lockscreen/ChargeTwo.vue'

const { t } = useI18n()
const systemStore = useSystemStore()
const systemTime = computed(() => systemStore.systemTime)
const battery = computed(() => systemStore.battery)

const layoutThemeStore = useLayoutThemeStore()
const language = computed(() => layoutThemeStore.layoutSetting.language)

const isShowForm = ref(!systemStore.lockScreenPassword)
const password = ref('')
const pwdPlaceholder = computed(() =>
  systemStore.lockScreenPassword
    ? t('setting.pleaseEnterYourUnlockCode')
    : t('setting.pleaseEnterYourLockScreenPassword'),
)

const hideLockForm = () => {
  isShowForm.value = false
  password.value = ''
}

const onLogin = () => {
  const pwd = password.value.trim()
  if (pwd === '') return message.warn(t('message.passwordCanNotBeEmpty'))
  if (systemStore.verifyLockScreenPassword(pwd)) {
    unlockScreen()
  } else {
    message.warn(t('message.wrongPasswordPleaseEnterAgain'))
  }
}

const cancelLock = () => {
  isShowForm.value = false
  systemStore.setLockScreenState(false)
}

const lockScreen = () => {
  const pwd = password.value.trim()
  if (pwd === '') return message.warn(t('message.passwordCanNotBeEmpty'))
  systemStore.setLockScreenPassword(pwd)
  hideLockForm()
}

const unlockScreen = () => {
  isShowForm.value = false
  systemStore.setLockScreenState(false)
}

const hintPassword = () => {
  message.info(
    `${t('setting.passwordHint')}${language.value === 'zhCN' ? '：' : ':'}` +
      systemStore.decodeLockScreenPassword(),
  )
}

const chargeComponent = computed(() =>
  Math.random() > 0.5 ? ChargeOne : ChargeTwo,
)

const batteryBar = computed(() => {
  const num = battery.value.level * 100
  if (num < 15) {
    return '0-bar'
  } else if (num < 30) {
    return '1-bar'
  } else if (num < 45) {
    return '2-bar'
  } else if (num < 60) {
    return '3-bar'
  } else if (num < 75) {
    return '4-bar'
  } else if (num < 90) {
    return '5-bar'
  } else if (num < 100) {
    return '6-bar'
  } else {
    return 'full'
  }
})
const batteryCharging = computed(() => {
  const num = battery.value.level * 100
  if (num < 20) {
    return 'charging-full-outline'
  } else if (num < 30) {
    return 'charging-20'
  } else if (num < 50) {
    return 'charging-30'
  } else if (num < 60) {
    return 'charging-50'
  } else if (num < 80) {
    return 'charging-60'
  } else if (num < 90) {
    return 'charging-80'
  } else if (num < 100) {
    return 'charging-90'
  } else {
    return 'charging-full'
  }
})

const handleSubmitPassword = () => {
  if (systemStore.lockScreenPassword) {
    onLogin()
  } else {
    lockScreen()
  }
}
</script>

<template>
  <div
    :class="['lockscreen', isShowForm ? 'unLockLogin' : '']"
    @keyup="isShowForm = true"
    @mousedown.stop
    @contextmenu.prevent
  >
    <template v-if="!isShowForm">
      <div class="lock-box">
        <div class="lock">
          <span
            class="lock-icon"
            :title="t('setting.unlockScreen')"
            @click="isShowForm = true"
          >
            <LockOutlined :style="{ fontSize: '24px' }" />
          </span>
        </div>
        <h6 class="tips">{{ t('setting.clickToUnlock') }}</h6>
      </div>
      <component :is="chargeComponent" :battery="battery" />
      <div class="local-time">
        <div class="time">
          {{ systemTime.hour < 10 ? '0' + systemTime.hour : systemTime.hour }}
          :
          {{
            systemTime.minute < 10 ? '0' + systemTime.minute : systemTime.minute
          }}
          :
          {{
            systemTime.second < 10 ? '0' + systemTime.second : systemTime.second
          }}
        </div>
        <div class="date">
          {{ systemTime.year }}/{{ systemTime.month }}/{{ systemTime.day }}
          {{ systemTime.week }}
        </div>
      </div>
      <div class="computer-status">
        <Icon
          class="iconify anticon"
          icon="material-symbols:wifi"
          v-if="systemStore.online"
        />
        <Icon
          class="iconify anticon"
          icon="material-symbols:wifi-off"
          v-if="!systemStore.online"
        />
        <Icon
          class="iconify anticon"
          :icon="`material-symbols:battery-${batteryCharging}-rounded`"
          v-if="battery.charging"
        />
        <Icon
          class="iconify anticon"
          :icon="`material-symbols:battery-${batteryBar}-rounded`"
          v-if="!battery.charging"
        />
      </div>
    </template>
    <template v-else>
      <div class="login-box">
        <a-form @submit="handleSubmitPassword">
          <a-input-password
            style="width: 300px"
            v-model:value="password"
            autofocus
            :placeholder="pwdPlaceholder"
          />
        </a-form>
        <div class="flex justify-between w-full">
          <template v-if="systemStore.lockScreenPassword">
            <a-button type="link" size="small" @click="hideLockForm">
              {{ t('setting.turnBack') }}
            </a-button>
            <a-button type="link" size="small" @click="hintPassword">
              {{ t('setting.passwordHint') }}
            </a-button>
            <a-button type="link" size="small" @click="onLogin">
              {{ t('setting.enterSystem') }}
            </a-button>
          </template>
          <template v-else>
            <a-button type="link" size="small" @click="cancelLock">
              {{ t('setting.cancelLockScreen') }}
            </a-button>
            <a-button type="link" size="small" @click="lockScreen">
              {{ t('setting.confirmLockScreen') }}
            </a-button>
          </template>
        </div>
      </div>
    </template>
  </div>
</template>

<style lang="less" scoped>
.lockscreen {
  display: flex;
  position: fixed;
  z-index: 999;
  inset: 0;
  overflow: hidden;
  background: #000;
  color: white;
  &.unLockLogin {
    background-color: rgb(25 28 34 / 78%);
    backdrop-filter: blur(7px);
  }
  .login-box {
    display: flex;
    position: absolute;
    top: 45%;
    left: 50%;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    width: 350px;
    transform: translate(-50%, -50%);
    > * {
      margin-bottom: 14px;
    }
    .username {
      font-size: 22px;
      font-weight: 700;
    }
  }
  .lock-box {
    position: absolute;
    top: 12vh;
    left: 50%;
    transform: translateX(-50%);
    font-size: 34px;
    .tips {
      color: white;
      cursor: text;
    }
    .lock {
      display: flex;
      justify-content: center;
      .lock-icon {
        cursor: pointer;
        .lock-icon {
          display: none;
        }
        &:hover .lock-icon {
          display: initial;
        }
        &:hover .lock-icon {
          display: none;
        }
      }
    }
  }
  .local-time {
    position: absolute;
    bottom: 60px;
    left: 60px;
    .time {
      font-size: 70px;
    }
    .date {
      font-size: 40px;
    }
  }
  .computer-status {
    position: absolute;
    right: 60px;
    bottom: 60px;
    font-size: 24px;
    > * {
      margin-left: 14px;
    }
  }
}
</style>
