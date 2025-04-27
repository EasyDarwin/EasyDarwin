<template>
  <div class="bg-white rounded-md cursor-pointer p-2" @click="onclick">
    <div v-if="data.status === 'done'">
      <div class="h-28 relative">
        <img class="h-full w-full object-cover rounded-md" :src="data.snapUrl" />
        <button
          class="absolute left-1/2 top-1/2 text-4xl -translate-x-1/2 -translate-y-1/2 text-primary bg-white/80 rounded-full p-2"
          @click.stop
        >
          ▶️
        </button>
      </div>

      <div class="text-left px-1 pt-3 space-y-2">
        <div class="flex items-center gap-1 text-sm text-gray-500 truncate">
          🎬 {{ data.name }}
        </div>

        <div class="flex items-center gap-1 text-sm text-gray-500 truncate">
          📅 {{ data.createAt }}
        </div>

        <div class="flex items-center gap-1 text-sm text-gray-500 truncate">
          描述：{{ data.describe }}
        </div>

        <div class="flex justify-end items-center space-x-2">
          <button class="text-gray-500 hover:text-primary" @click.stop="download">⬇️</button>
          <button class="text-gray-400 cursor-not-allowed" disabled>🔗</button>
          <button class="text-red-500 hover:text-red-700" @click.stop="confirmDelete">🗑️</button>
        </div>
      </div>
    </div>

    <div v-else class="flex justify-center items-center h-28">
      <a-progress
        :percent="data.progress"
        type="circle"
        :width="64"
        stroke-color="#409eff"
      />
    </div>
  </div>
</template>

<script setup>
import { saveFile } from "@/utils/down";
import { Progress as AProgress } from "ant-design-vue";

const props = defineProps(["data"]);
const emit = defineEmits(["onClick", "onDelect"]);

// 点击盒子
const onclick = () => {
  emit("onClick", props.data.id);
};

// 确认删除
const confirmDelete = () => {
  if (confirm('您确定要删除吗？')) {
    emit("onDelect", props.data.id);
  }
};

// 下载文件
const download = () => {
  const url = `/vod/download/${props.data.id}`;
  saveFile(url);
};
</script>

<style scoped>
/* 不需要额外样式了，unocss + antd-vue足够 */
</style>
