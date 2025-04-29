<template>
  <a-modal v-model:open="open" centered title="分发流地址" @cancel="handelCancel">
    <a-form layout="vertical" :model="url" class="pt-2">

      <a-form-item label="HTTP-FLV">
        <div class="flex items-center space-x-2">
          <a-input v-model:value="url.http_flv" readonly disabled />
          <a-button @click="copy(url.http_flv)">复制</a-button>
        </div>
      </a-form-item>

      <a-form-item label="HTTP-HLS">
        <div class="flex items-center space-x-2">
          <a-input v-model:value="url.http_hls" readonly disabled />
          <a-button @click="copy(url.http_hls)">复制</a-button>
        </div>
      </a-form-item>

      <a-form-item label="WS-FLV">
        <div class="flex items-center space-x-2">
          <a-input v-model:value="url.ws_flv" readonly disabled />
          <a-button @click="copy(url.ws_flv)">复制</a-button>
        </div>
      </a-form-item>

      <a-form-item label="WebRTC">
        <div class="flex items-center space-x-2">
          <a-input v-model:value="url.webrtc" readonly disabled />
          <a-button @click="copy(url.webrtc)">复制</a-button>
        </div>
      </a-form-item>

      <a-form-item label="RTMP">
        <div class="flex items-center space-x-2">
          <a-input v-model:value="url.rtmp" readonly disabled />
          <a-button @click="copy(url.rtmp)">复制</a-button>
        </div>
      </a-form-item>

      <a-form-item label="RTSP">
        <div class="flex items-center space-x-2">
          <a-input v-model:value="url.rtsp" readonly disabled />
          <a-button @click="copy(url.rtsp)">复制</a-button>
        </div>
      </a-form-item>

    </a-form>
  </a-modal>
</template>

<script setup>
import { ref } from 'vue';
import { copyText } from "@/utils";

const open = ref(false);
const url = ref({
  http_flv: '',
  http_hls: '',
  ws_flv: '',
  webrtc: '',
  rtmp: '',
  rtsp: ''
})

const openModal = (data) => {
  url.value = data
  open.value = true;
}

const handelCancel = () => {
  open.value = false;
}

const copy = (value) => {
  copyText(value);
}

defineExpose({
  open: openModal
})
</script>
