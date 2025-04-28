<template>
  <div>
    <a-button type="primary" @click="onClickUpload">
      <template #icon>
        <PlusOutlined />
      </template>
      上传视频
    </a-button>

    <UploadModal :open="uploadModalVisible" @refreshList="getVodDataList" @update:open="uploadModalVisible = false"
      @callback="onCallback" />
  </div>
</template>

<script setup>
import { onMounted, reactive, ref } from 'vue';
import { vodApi } from '@/api';
import { PlusOutlined } from '@ant-design/icons-vue';
import UploadModal from './upload.vue';

const uploadModalVisible = ref(false);

//获取点播数据请求参数
const vodParams = reactive({
  start: 0,
  limit: 10,
  sort: "", //排序字段
  order: "", //排序顺序 允许值: ascending, descending
  q: "", //查询参数
});

onMounted(() => {
  getVodDataList();
})

const getVodDataList = () => {
  vodApi.getVodList(vodParams).then(res => {
    console.log(res);
  }).catch(err => {
    console.log(err);
  })
}

const onClickUpload = () => {
  console.log('onClickUpload2');
  uploadModalVisible.value = true
}

const onCallback = () => {
  // getVodDataList();
}

</script>
