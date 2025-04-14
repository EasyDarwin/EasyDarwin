<script setup>
import { ref, onMounted } from 'vue'
import { useSystemStore } from '@/store/layout/system.js'
import LockScreenPage from '@/layouts/components/lockscreen/LockScreenPage.vue'

const systemStore = useSystemStore()

const isMounted = ref(false)

onMounted(() => {
  isMounted.value = true
})
</script>

<template>
  <Transition name="slide-up">
    <LockScreenPage v-if="systemStore.lockScreenState && isMounted" />
  </Transition>
</template>

<style lang="less" scoped>
.slide-up-enter-active {
  animation: slide-up 0.5s;
}
.slide-up-leave-active {
  animation: slide-up 0.5s reverse;
}
@keyframes slide-up {
  0% {
    transform: translateY(-100%);
  }
  100% {
    transform: translateY(0);
  }
}
</style>
