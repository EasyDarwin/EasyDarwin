<template>
  <a-modal v-model:open="visible" title="点播上传" :footer="null" centered width="500px">
    <div class="space-y-4">
      <a-upload-dragger
        name="file"
        :multiple="false"
        :max-count="1"
        :before-upload="() => false"
        :accept="accept"
        @change="handleFileChange"
      >
        <div class="flex flex-col items-center justify-center py-8">
          <upload-outlined class="text-4xl text-gray-400" />
          <p class="mt-2 text-sm text-gray-500">
            拖放文件到此 或 点击 <span class="text-primary">上传</span>
          </p>
          <p class="text-xs text-gray-400 mt-1">支持文件类型：{{ accept }}</p>
        </div>
      </a-upload-dragger>

      <div v-if="progress > 0" class="px-2">
        <a-progress :percent="progress" status="active" />
      </div>

      <div>
        <p class="font-semibold mb-1">视频描述（可选）</p>
        <a-textarea
          v-model:value="form.describe"
          placeholder="请输入视频描述（最多100字）"
          :maxlength="100"
          auto-size
          allow-clear
        />
      </div>

      <div class="flex justify-end space-x-2">
        <a-button @click="close">取消</a-button>
        <a-button type="primary" :loading="uploading" @click="handleSubmit">上传</a-button>
      </div>
    </div>
  </a-modal>
</template>

<script setup>
import { ref, reactive } from "vue";
import { UploadOutlined } from "@ant-design/icons-vue";
import { UploadVod, FindUploadAccept } from "@/api/vod";
import { message, notification } from "ant-design-vue";
const UPLOAD_ACCEPT = ".mp3,.wav,.mp4,.mpg,.mpeg,.wmv,.avi,.rmvb,.mkv,.flv,.mov,.3gpp,.3gp,.webm,.m4v,.mng,.vob"

const emit = defineEmits(["refreshList", "callback"]);

const visible = ref(false);
const accept = ref("");
const progress = ref(0);
const uploading = ref(false);

const form = reactive({
  file: null,
  describe: ""
});

// 打开弹窗
const open = async () => {
  await fetchAccept();
  visible.value = true;
};

// 关闭弹窗
const close = () => {
  resetForm();
  visible.value = false;
};

// 选择文件
const handleFileChange = (info) => {
  const file = info.file;
  if (file.status !== "removed") {
    form.file = file.originFileObj;
  }
};

// 表单提交
const handleSubmit = async () => {
  if (!form.file) {
    return message.error("请选择一个视频文件！");
  }

  const formData = new FormData();
  formData.append("file", form.file);
  formData.append("describe", form.describe);

  emit("callback", true);
  uploading.value = true;
  progress.value = 0;

  try {
    await uploadVod(formData);
    notification.success({ message: "上传成功", description: "文件已成功上传！" });
    emit("refreshList");
    close();
  } catch (error) {
    console.log(error);
  } finally {
    emit("callback", false);
    uploading.value = false;
  }
};

// 上传文件
const uploadVod = async (formData) => {
    const res = await UploadVod(formData, {
      onUploadProgress: (e) => {
        if (e.total > 0) {
          progress.value = Math.round((e.loaded / e.total) * 100);
        }
      }
    });
    return res;
};

// 请求上传类型
const fetchAccept = async () => {
  try {
    const res = await FindUploadAccept();
    accept.value = res.data || UPLOAD_ACCEPT;
  } catch (error) {
    console.log(error);
  }
};

// 重置表单
const resetForm = () => {
  form.file = null;
  form.describe = "";
  progress.value = 0;
};

defineExpose({
  open
});
</script>
