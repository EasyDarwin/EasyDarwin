<template>
  <a-modal v-model:open="open" title="编辑视频" :confirm-loading="submitLoading" @cancel="handelCancel"
    :ok-button-props="{ htmlType: 'submit', form: 'editForm' }" centered>
    <a-form id="editForm" :model="formData" layout="vertical" @finish="onFinish" class="pt-2">
      <a-form-item name="name" label="视频名称" :rules="[{ required: true, message: '请输入视频名称' }]">
        <a-input v-model:value="formData.name" placeholder="请输入视频名称" />
      </a-form-item>



      <a-form-item label="视频封面">
        <a-space>
          <a-upload :max-count="1" :showUploadList="false" :beforeUpload="() => false" :customRequest="() => { }"
            name="file" accept=".png,.jpg,.jpeg" @change="handleImageChange">
            <a-button>
              <UploadOutlined />
              上传封面
            </a-button>
          </a-upload>

          <a-button @click="resetCover">重置封面</a-button>
        </a-space>

        <div v-if="previewImage" class="mt-2">
          <img :src="previewImage" alt="封面预览" class="max-w-[200px] border border-gray-200 rounded" />
        </div>
      </a-form-item>


      <a-form-item name="shared" label="是否共享" value-prop-name="checked">
        <a-switch v-model:checked="formData.shared" />
      </a-form-item>

      <a-form-item v-if="formData.sharedLink != '' && formData.shared" label="分享链接">
        <a-input :value="formData.sharedLink" placeholder="-" class="mb-4" />
        <a-qrcode :value="formData.sharedLink" />
      </a-form-item>

      <a-form-item name="id" hidden>
        <a-input v-model:value="formData.id" />
      </a-form-item>
    </a-form>
  </a-modal>
</template>

<script setup>
import { ref, reactive } from 'vue';
import { Form, message } from 'ant-design-vue';
import { vodApi } from '@/api';
import { UploadOutlined, CloseCircleOutlined } from '@ant-design/icons-vue';

const open = ref(false);

const emit = defineEmits(['update:open', 'refresh']);


const formData = reactive({
  id: '',
  name: '',
  shared: false,
  sharedLink: ''
})

const fileImage = ref(undefined)
const previewImage = ref(undefined)

// 上传封面
const handleImageChange = (info) => {
  fileImage.value = info.file

  const reader = new FileReader()
  reader.onload = (e) => {
    previewImage.value = e.target.result
  }
  reader.readAsDataURL(fileImage.value)
}

// 提交
const submitLoading = ref(false);
async function onFinish(values) {
  if (submitLoading.value) return;
  submitLoading.value = true;

  try {
    const vodPromise = vodApi.vodEdit(values);
    let uploadPromise = Promise.resolve();
    if (fileImage.value) {
      const formData = new FormData();
      formData.append('id', values.id);
      formData.append('cover', fileImage.value);
      uploadPromise = vodApi.uploadVodSnap(formData, () => { });
    }

    const [vodRes, snapRes] = await Promise.all([vodPromise, uploadPromise]);

    message.success('操作成功');

    emit('refresh');
    handelCancel();

  } catch (err) {
    console.error(err);
    message.error('操作失败，请重试');
  } finally {
    submitLoading.value = false;
  }
}

// 重置封面
const resetCover = async () => {
  const form = new FormData();
  form.append('id', formData.id);
  form.append('time', '00:00:01');
  await vodApi.uploadVodSnap(form, () => { })
  const { data: vodRes } = await vodApi.getVodItemInfo(formData.id);
  previewImage.value = vodRes.snapUrl + `?t=${Date.now()}`;
  fileImage.value = undefined;
  message.success('重置成功');
  emit('refresh');
}

const openModal = (item) => {
  formData.id = item.id;
  formData.name = item.name;
  formData.shared = item.shared;
  formData.sharedLink = item.sharedLink;
  previewImage.value = item.snapUrl + `?t=${Date.now()}`;
  open.value = true;
}

const handelCancel = () => {
  open.value = false;
  formData.id = '';
  formData.name = '';
  formData.shared = false;
  formData.sharedLink = '';
  fileImage.value = undefined;
  previewImage.value = undefined;
}

defineExpose({
  open: openModal
})
</script>
