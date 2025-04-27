<script setup>
import { computed } from 'vue'
import { useI18n } from 'vue-i18n'

const props = defineProps({
  battery: {
    type: Object,
    default: () => ({}),
  },
})

const { t } = useI18n()

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
  <div class="charge">
    <div class="number">{{ (battery.level * 100).toFixed(0) }}%</div>
    <div class="contrast">
      <div class="circle" />
      <ul class="bubbles" style="">
        <li v-for="i in battery.charging ? 15 : 0" :key="i" />
      </ul>
    </div>
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
  </div>
</template>

<style lang="less" scoped>
.charge {
  .generate-columns(15);

  @keyframes trotate {
    50% {
      border-radius: 45% / 42% 38% 58% 49%;
    }

    100% {
      transform: translate(-50%, -50%) rotate(720deg);
    }
  }

  @keyframes move-to-top {
    90% {
      opacity: 1;
    }

    100% {
      transform: translate(-50%, -180px);
      opacity: 0.1;
    }
  }

  @keyframes hue-rotate {
    100% {
      filter: contrast(15) hue-rotate(360deg);
    }
  }

  position: absolute;
  bottom: 20vh;
  left: 50vw;
  width: 300px;
  height: 400px;
  transform: translateX(-50%);
  .generate-columns(@n, @i: 0) when (@i =< @n) {
    .generate-columns(@n, (@i + 1));
    .column-@{i} {
      width: (@i * 100% / @n);
    }
    li:nth-child(@{i}) {
      @width: unit(~`Math.round(15 + Math.random() * 15) `, px);

      top: 50%;
      left: unit(~`Math.round(Math.random() * 70) `, px);
      width: @width;
      height: @width;
      transform: translate(-50%, -50%);
      animation: move-to-top unit(~`(Math.round(Math.random() * 6) + 3) `, s)
        ease-in-out unit(~`-(Math.random() * 5000 / 1000) `, s) infinite;
    }
  }

  .number {
    position: absolute;
    z-index: 10;
    top: 27%;
    width: 300px;
    color: #fff;
    font-size: 32px;
    text-align: center;
  }

  .contrast {
    width: 300px;
    height: 400px;
    overflow: hidden;
    animation: hue-rotate 10s infinite linear;
    background-color: #000;
    filter: contrast(15) hue-rotate(0);

    .circle {
      position: relative;
      box-sizing: border-box;
      width: 300px;
      height: 300px;
      filter: blur(8px);

      &::after {
        content: '';
        position: absolute;
        top: 40%;
        left: 50%;
        width: 200px;
        height: 200px;
        transform: translate(-50%, -50%) rotate(0);
        animation: trotate 10s infinite linear;
        border-radius: 42% 38% 62% 49% / 45%;
        background-color: #00ff6f;
      }

      &::before {
        content: '';
        position: absolute;
        z-index: 10;
        top: 40%;
        left: 50%;
        width: 176px;
        height: 176px;
        transform: translate(-50%, -50%);
        border-radius: 50%;
        background-color: #000;
      }
    }

    .bubbles {
      position: absolute;
      bottom: 0;
      left: 50%;
      width: 100px;
      height: 40px;
      transform: translate(-50%, 0);
      border-radius: 100px 100px 0 0;
      background-color: #00ff6f;
      filter: blur(5px);

      li {
        position: absolute;
        border-radius: 50%;
        background: #00ff6f;
      }
    }
  }

  .charging {
    font-size: 20px;
    text-align: center;
  }
}
</style>
