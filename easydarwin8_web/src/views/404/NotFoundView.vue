<script setup>
import { useRouter } from 'vue-router'
import { useI18n } from 'vue-i18n'

const router = useRouter()
const { t } = useI18n()
</script>

<template>
  <a-result
    status="404"
    title="404"
    :sub-title="t('message.sorryPageNotFound')"
  >
    <template #extra>
      <a-button type="primary" @click="() => router.push('/')">
        {{ t('setting.backToHome') }}
      </a-button>
    </template>
  </a-result>
</template>

<style lang="less" scoped></style>
