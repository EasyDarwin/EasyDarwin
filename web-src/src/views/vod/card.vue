<template>
  <div class="cursor-pointer rounded-md overflow-hidden bg-white h-full relative flex flex-col">
    <div v-if="data.status === 'done'" class="flex-1 flex flex-col">
      <div class="relative" @click="onclick">
        <a-image class="aspect-video w-full object-cover" :preview="false" :src="data.snapUrl"
          fallback="/assets/images/noImg.png" />

        <PlayCircleOutlined
          class="text-white absolute left-1/2 top-1/2 text-4xl -translate-x-1/2 -translate-y-1/2  hover:text-green transition-all" />
      </div>

      <div class="text-left  space-y-2 flex-1 flex flex-col justify-between">
        <div class="text-left py-2 px-2 space-y-2">
          <div class="text-sm text-gray-600 truncate font-semibold">
            {{ data.name }}
          </div>

          <a-space wrap size="small">
            <a-tag v-if="data.aspect" :bordered="false">{{ data.aspect }}</a-tag>
            <a-tag v-if="data.audioCodec" :bordered="false"> {{ data.audioCodec }}</a-tag>
            <a-tag v-if="data.videoCodec" :bordered="false"> {{ data.videoCodec }}</a-tag>
            <a-tag v-if="data.size" :bordered="false"> {{ formatFileSize(data.size) }}</a-tag>
            <a-tag v-if="data.duration" :bordered="false">{{ formatDuration(data.duration) }}</a-tag>
            <a-tag v-if="data.shared" color="green" :bordered="false">分享中</a-tag>
          </a-space>
        </div>

        <div class="flex justify-end items-center space-x-2 p-2">
          <a-tooltip title="播放">
            <a-button type="text" @click.stop="onclick">
              <template #icon>
                <PlayCircleOutlined />
              </template>
            </a-button>
          </a-tooltip>

          <a-tooltip title="编辑">
            <a-button type="text" @click.stop="onClickEdit">
              <template #icon>
                <EditOutlined />
              </template>
            </a-button>
          </a-tooltip>

          <a-tooltip title="下载">
            <a-button type="text" @click.stop="download">
              <template #icon>
                <DownloadOutlined />
              </template>
            </a-button>
          </a-tooltip>

          <a-popconfirm placement="topRight" title="您确定要重新转码这条视频吗？" ok-text="是" cancel-text="否"
            @confirm="onClickRetran">
            <a-tooltip title="重新转码">
              <a-button type="text">
                <template #icon>
                  <RedoOutlined />
                </template>
              </a-button>
            </a-tooltip>
          </a-popconfirm>

          <a-popconfirm placement="topRight" title="您确定要删除这条视频吗？" ok-text="是" cancel-text="否" @confirm="onClickDelete">
            <a-button type="text" danger>
              <template #icon>
                <DeleteOutlined />
              </template>
            </a-button>
          </a-popconfirm>
        </div>

      </div>
    </div>

    <div v-else class="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 ">
      <a-progress :percent="progress" type="circle" :width="64" stroke-color="#409eff" />
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, watch } from "vue";
import { formatFileSize, formatDuration } from "@/utils/size";
import { Progress as AProgress } from "ant-design-vue";
import { DownloadOutlined, PlayCircleOutlined, DeleteOutlined, RedoOutlined, EditOutlined } from '@ant-design/icons-vue';
import { vodApi } from '@/api'

const props = defineProps(["data"]);
const emit = defineEmits(["onClick", "onEdit", "onDelect", "onRetran", "refresh"]);

// 点击盒子
const onclick = () => {
  emit("onClick", props.data);
};

// 点击编辑
const onClickEdit = () => {
  emit("onEdit", props.data)
}

// 确认删除
const onClickDelete = () => {
  emit("onDelect", props.data.id);
};

// 重新转码
const onClickRetran = () => {
  emit("onRetran", props.data.id)
}

// 下载文件
const download = async () => {
  const res = await vodApi.downloadVod(props.data.id);
  const blob = new Blob([res.data]);
  const url = window.URL.createObjectURL(blob);

  const a = document.createElement('a');
  a.style.display = 'none';
  a.href = url;
  a.download = props.data.name + '.mp4'; // 如果你传了 filename 就用，否则叫 file
  document.body.appendChild(a);
  a.click();

  // 释放 URL 对象，清理内存
  window.URL.revokeObjectURL(url);
  document.body.removeChild(a);
};

// 获取转码进度
const intervalId = ref(null); // 保存定时器 id
const progress = ref(0);

// 自动轮询更新数据
const startPolling = () => {
  if (intervalId.value) return; // 防止重复开定时器
  intervalId.value = setInterval(async () => {
    try {
      const res = await vodApi.getVodProgress(props.data.id);
      if (res.data) {
        progress.value = res.data.reason;
      }
      if (props.data.status === 'done' || res.data.reason == 100) {
        clearInterval(intervalId.value);
        intervalId.value = null;
      }
      emit('refresh')
    } catch (err) {
      console.error('拉取进度失败', err);
    }
  }, 3000); // 每 3 秒拉一次
};

watch(() => props.data, (newData) => {
  if (newData.status !== "done") {
    startPolling();
  } else {
    clearInterval(intervalId.value);
    intervalId.value = null;
  }
}, { deep: true });


// 组件挂载后判断是否需要轮询
onMounted(() => {
  if (props.data.status !== 'done') {
    startPolling();
  }
});

// 组件销毁时清除定时器
onUnmounted(() => {
  if (intervalId.value) {
    clearInterval(intervalId.value);
    intervalId.value = null;
  }
});
</script>
