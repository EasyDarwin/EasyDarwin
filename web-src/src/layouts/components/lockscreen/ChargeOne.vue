<script setup>
import { computed } from 'vue'
import { useI18n } from 'vue-i18n'
import { useLayoutThemeStore } from '@/store/layout/layoutTheme.js'

const props = defineProps({
  battery: {
    type: Object,
    default: () => ({}),
  },
})

const { t } = useI18n()
const layoutThemeStore = useLayoutThemeStore()
const language = computed(() => layoutThemeStore.layoutSetting.language)

const batteryStatus = computed(() => {
  if (props.battery.charging) {
    if (props.battery.level >= 1) {
      return t('setting.fullCharge')
    } else {
      return t('setting.charging')
    }
  } else {
    return t('setting.powerSupplyDisconnected')
  }
})
const calcDischargingTime = computed(() => {
  if (
    Number.isFinite(props.battery.dischargingTime) &&
    props.battery.dischargingTime !== 0
  ) {
    const time = props.battery.dischargingTime / 60
    const hour = Math.floor(time / 60)
    const minute = Math.floor(time % 60)
    return `${hour}${t('unit.hour')}${minute}${t('unit.minute')}`
  } else {
    return t('setting.unKnown')
  }
})
const calcChargingTime = computed(() => {
  if (
    Number.isFinite(props.battery.chargingTime) &&
    props.battery.chargingTime !== 0
  ) {
    const time = props.battery.chargingTime / 60
    const hour = Math.floor(time / 60)
    const minute = Math.floor(time % 60)
    return `${hour}${t('unit.hour')}${minute}${t('unit.minute')}`
  } else {
    return t('setting.unKnown')
  }
})
</script>

<template>
  <div class="charging">
    <div>{{ batteryStatus }}</div>
    <div v-if="calcDischargingTime !== t('setting.unKnown')">
      {{ t('setting.remainingAvailableTime') }}
      {{ language === 'zh-CN' ? '：' : ':' }}：
      {{ calcDischargingTime }}
    </div>
    <div v-if="calcChargingTime !== t('setting.unKnown')">
      {{ t('setting.distanceBatteryChargeFull') }}
      {{ language === 'zh-CN' ? '：' : ':' }}
      {{ calcChargingTime }}
    </div>
  </div>
  <div class="charge">
    <div v-for="i in 3" :key="i" class="outer">
      <div
        class="circle"
        :style="{ transform: `scale(${1.01 - 0.04 * (i - 1)})` }"
      />
    </div>
    <div class="line-box">
      <div class="line-left" />
      <div class="line-left line-right" />
      <div class="line-center line-center-left-2" v-if="battery.charging" />
      <div class="line-center line-center-left-1" v-if="battery.charging" />
      <div class="line-center" v-if="battery.charging" />
      <div class="line-center line-center-right-1" v-if="battery.charging" />
      <div class="line-center line-center-right-2" v-if="battery.charging" />
    </div>
    <div class="outer" style="transform: scale(0.68)">
      <div class="circle circle-blur" style="padding: 30px" />
    </div>
    <div v-for="i in 4" :key="i" class="outer">
      <div
        class="circle-white"
        :style="{
          transform: `scale(${1 - 0.02 * (i - 1)})`,
          animationDuration: `${500 - 20 * (i - 1)}ms`,
        }"
      />
    </div>
    <div class="outer">
      <div class="text">
        {{ (battery.level * 100).toFixed(0) }}<span class="sub">%</span>
      </div>
    </div>
    <div class="light" />
  </div>
</template>

<style lang="less" scoped>
.charging {
  position: absolute;
  bottom: 550px;
  left: calc(50vw - 250px);
  width: 500px;
  font-size: 20px;
  text-align: center;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  div {
    margin-bottom: 10px;
  }
}
.charge {
  @keyframes rotate {
    0% {
      transform: rotate(0deg);
    }

    100% {
      transform: rotate(360deg);
    }
  }

  @keyframes up {
    0% {
      transform: translateY(80px);
    }

    100% {
      transform: translateY(-400px);
    }
  }

  @keyframes light {
    0% {
      transform: scale(0.3);
      opacity: 0.3;
    }

    40% {
      transform: scale(1);
      opacity: 0.6;
    }

    100% {
      transform: scale(0.3);
      opacity: 0;
    }
  }

  display: flex;
  position: absolute;
  bottom: 0;
  left: 50vw;
  justify-content: center;
  width: 300px;
  height: 400px;
  transform: translateX(-50%);

  .circle {
    position: absolute;
    width: 286px;
    height: 286px;
    padding: 2px;
    border-radius: 50%;
    background: linear-gradient(#c71ff1, #2554ea);
  }

  .circle::after {
    content: ' ';
    display: block;
    width: 100%;
    height: 100%;
    border-radius: 50%;
    background: #000;
  }

  .circle-blur {
    filter: blur(5px);
    animation: rotate 5s linear infinite;
  }

  .circle-white {
    position: absolute;
    width: 220px;
    height: 220px;
    animation: rotate 500ms linear infinite;
    border-top: solid 1px rgb(255 255 255 / 6%);
    border-bottom: solid 1px rgb(255 255 255 / 8%);
    border-radius: 50%;
  }

  .outer {
    display: flex;
    position: absolute;
    bottom: 400px;
    align-items: center;
    justify-content: center;
  }

  .line-box {
    position: absolute;
    bottom: 0;
    width: 80px;
    height: 400px;
    overflow: hidden;
    background: #000;
  }

  .line-left {
    position: absolute;
    bottom: 0;
    left: -15px;
    box-sizing: border-box;
    width: 30px;
    height: 267px;
    border-top: solid 2px #2554ea;
    border-right: solid 2px #2554ea;
    border-top-right-radius: 40px;
  }

  .line-left::before {
    content: '';
    position: absolute;
    top: -8px;
    left: 0;
    box-sizing: border-box;
    width: 30px;
    height: 100%;
    transform: scaleY(0.96);
    transform-origin: center top;
    border-top: solid 2px #2554ea;
    border-right: solid 2px #2554ea;
    border-top-right-radius: 50px;
  }

  .line-left::after {
    content: '';
    position: absolute;
    top: -14px;
    left: 0;
    box-sizing: border-box;
    width: 30px;
    height: 100%;
    transform: scaleY(0.92);
    transform-origin: center top;
    border-top: solid 2px #2554ea;
    border-right: solid 2px #2554ea;
    border-top-right-radius: 60px;
  }

  .line-right {
    transform: scaleX(-1);
    transform-origin: 55px;
  }

  .line-center {
    position: absolute;
    top: 0;
    left: 39px;
    width: 2px;
    height: 100%;
    background: #231779;
  }

  .line-center::before {
    content: '';
    position: absolute;
    bottom: 10px;
    width: 2px;
    height: 80px;
    animation: up 700ms linear infinite;
    border-top-left-radius: 2px;
    border-top-right-radius: 2px;
    background: linear-gradient(#79ccea, transparent);
  }

  .line-center-left-1 {
    transform: translateX(-9px);
  }

  .line-center-left-2 {
    transform: translateX(-18px);
  }

  .line-center-right-1 {
    transform: translateX(9px);
  }

  .line-center-right-2 {
    transform: translateX(18px);
  }

  .line-center-left-1::before {
    animation-delay: -200ms;
  }

  .line-center-left-2::before {
    animation-delay: -400ms;
  }

  .line-center-right-1::before {
    animation-delay: -300ms;
  }

  .line-center-right-2::before {
    animation-delay: -500ms;
  }

  .text {
    position: absolute;
    width: 200px;
    height: 80px;
    color: turquoise;
    font-size: 70px;
    line-height: 80px;
    text-align: center;
  }

  .sub {
    font-size: 30px;
  }

  .light {
    position: absolute;
    bottom: -150px;
    width: 300px;
    height: 350px;
    animation: light 1.2s linear 1 forwards;
    border-radius: 50%;
    background: radial-gradient(#2554ea, transparent 60%);
  }
}
</style>
