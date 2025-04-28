<template>
  <a-modal v-model:open="open" title="编辑视频" :confirm-loading="loading" @cancel="handelCancel"
    :ok-button-props="{ htmlType: 'submit', form: 'editForm' }" centered>
    <a-form id="editForm" :model="formData" layout="vertical" @finish="onFinish" class="pt-2">
      <a-form-item name="name" label="视频名称" :rules="[{ required: true, message: '请输入视频名称' }]">
        <a-input v-model:value="formData.name" placeholder="请输入视频名称" />
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

const open = ref(false);

const emit = defineEmits(['update:open', 'refresh']);


const formData = reactive({
  id: '',
  name: '',
  shared: false,
  sharedLink: ''
})


// 提交
async function onFinish(values) {
  vodApi.vodEdit(values).then(res => {
    if (res.data.code == 200) {
      message.info('操作成功');
      emit('refresh');
      handelCancel();
    }
  }).catch(err => {
    message.error('操作失败')
  })
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
}

defineExpose({
  open: openModal
})
</script>
