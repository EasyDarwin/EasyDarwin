<template>
  <a-modal :open="open" title="上传视频" :footer="null" width="60%" @cancel="handleCancel">
    <div class="space-y-4">
      <a-upload-dragger v-if="!progress"  name="file" :file-list="fileList" :before-upload="beforeUpload" :accept="accept" :max-count="1"
        @change="handleFileChange">
        <div class="flex flex-col items-center justify-center py-8">
          <upload-outlined class="text-4xl text-gray-400" />
          <p class="mt-2 text-sm text-gray-500">
            拖放文件到此 或 点击 <span class="text-primary">上传</span>
          </p>
          <p class="text-xs text-gray-400 mt-1">
            支持文件类型：{{ accept || defaultAccept }}
          </p>
        </div>
      </a-upload-dragger>

      <div v-else class="px-2">
        <a-progress :percent="progress" status="active" />
      </div>
    </div>
  </a-modal>
</template>

<script setup>
import { ref, reactive, watch } from "vue";
import { UploadOutlined } from "@ant-design/icons-vue";
import { vodApi } from "@/api";
import { message, notification } from "ant-design-vue";

const props = defineProps({
  open: { type: Boolean, required: true },
});
const emit = defineEmits(["update:open", "refreshList", "callback"]);

// 默认支持格式
const defaultAccept =
  ".mp3,.wav,.mp4,.mpg,.mpeg,.wmv,.avi,.rmvb,.mkv,.flv,.mov,.3gpp,.3gp,.webm,.m4v,.mng,.vob";
const accept = ref("");

// 受控 fileList
const fileList = ref([]);

// 进度 & loading
const progress = ref(0);
const uploading = ref(false);

// 我们只需要 file
const form = reactive({ file: null });

// 当父组件打开弹窗时，拉取 accept；关闭时重置
watch(
  () => props.open,
  (open) => {
    if (open) {
      fetchAccept();
    } else {
      resetForm();
    }
  }
);

const beforeUpload = (file) => {
  // 立即调用上传逻辑
  uploadFile(file);
  // 阻止 a-upload 自带的上传行为
  return false;
};

async function uploadFile(file) {
  const formData = new FormData();
  formData.append("file", file);

  uploading.value = true;
  progress.value = 0;
  emit("callback", true);
  try {
    await vodApi.uploadVod(formData, (e) => {
      progress.value = e;
    });

    notification.success({ message: "上传成功", description: "文件已成功上传！" });
    emit("refreshList");
    handleCancel();
  } catch (err) {
    console.log("上传失败：", err);
    message.error(err.response?.data?.message || "上传失败，请重试");
  } finally {
    uploading.value = false;
    emit("callback", false);
  }
}
// 拉取后端允许的 accept
async function fetchAccept() {
  try {
    const res = await vodApi.getVodUploadAccept();
    accept.value = res.data || defaultAccept;
  } catch (e) {
    console.error(e);
    accept.value = defaultAccept;
  }
}

// 关闭弹窗
const handleCancel = () => {
  emit("update:open", false);
};


// 重置所有状态
function resetForm() {
  fileList.value = [];
  form.file = null;
  progress.value = 0;
  uploading.value = false;
}
</script>
