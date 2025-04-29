<template>
  <a-modal v-model:open="open" title="编辑视频" :confirm-loading="loading" @cancel="handelCancel"
    :ok-button-props="{ htmlType: 'submit', form: 'editForm' }" centered>
    <a-form id="editForm" :model="formData" layout="vertical" @finish="onFinish" class="pt-2">
      <a-form-item name="name" label="视频名称" :rules="[{ required: true, message: '请输入视频名称' }]">
        <a-input v-model:value="formData.name" placeholder="请输入视频名称" />
      </a-form-item>


      <!--
      <a-form-item label="视频封面">
        <a-upload v-model:file-list="fileList" :max-count="1" :beforeUpload="() => false" :customRequest="() => { }"
          name="file" accept=".png,.jpg,.jpeg" @change="handleImageChange">
          <a-button>
            <UploadOutlined />
            上传封面
          </a-button>
        </a-upload>
      </a-form-item> -->


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
import { UploadOutlined } from '@ant-design/icons-vue';

const open = ref(false);

const emit = defineEmits(['update:open', 'refresh']);


const formData = reactive({
  id: '',
  name: '',
  shared: false,
  sharedLink: ''
})

// const fileImage = ref()

// // 上传封面
// const handleImageChange = (file, files) => {
//   console.log('>>>', file, files);
//   fileImage.value = file;
// }

// 提交
async function onFinish(values) {
  const vodRes = await vodApi.vodEdit(values).catch(err => {
    message.error('操作失败')
  })
  if (vodRes.data.code == 200) {
    message.info('操作成功');
    emit('refresh');
    handelCancel();
  }

  // const formData = new FormData()
  // formData.append("id", values.id)
  // formData.append("file", fileImage.value)
  // const snapRes = await vodApi.uploadVodSnap()
  // console.log('>>>', snapRes);
}

const openModal = (item) => {
  formData.id = item.id;
  formData.name = item.name;
  formData.shared = item.shared;
  formData.sharedLink = item.sharedLink;
  open.value = true;
}

const handelCancel = () => {
  open.value = false;
  formData.id = '';
  formData.name = '';
  formData.shared = false;
  formData.sharedLink = '';
  // fileImage.value = undefined;
}

defineExpose({
  open: openModal
})
</script>
