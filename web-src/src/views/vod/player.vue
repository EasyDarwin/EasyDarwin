<template>
  <a-modal width="60%" :title="title" :open="open" :footer="null" @cancel="handleCancel">
    <div class="w-full aspect-video">
      <VodVideo :videoUrl="url" />
    </div>
  </a-modal>
</template>

<script setup>
import VodVideo from '@/components/Player/vod.vue';

const props = defineProps({
  open: {
    type: Boolean,
    default: false
  },
  title: {
    type: String,
    default: "点播预览"
  },
  url: {
    type: String,
    default: ""
  },
});
const emit = defineEmits(["update:open",]);

const handleCancel = () => {
  emit("update:open", false);
}
</script>
