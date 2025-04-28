<template>
  <div>
    <div class="bg-white rounded-md cursor-pointer p-2">
      <a-button type="primary" @click="onClickUpload">
        <template #icon>
          <PlusOutlined />
        </template>
        上传视频
      </a-button>
    </div>

    <div class="mt-5">
      <a-row :gutter="[16, 16]">
        <a-col :xs="24" :sm="24" :md="12" :lg="8" :xl="6" :xxl="4" v-for="(item, index) in vodData.items"
          :key="item.id">
          <VodCard :data="item" @on-click="onPlayVod(item)" @on-delect="onDeleteVod" />
        </a-col>
      </a-row>
    </div>

    <VodPlayer :open="playerVisible" :url="playerData.url" :title="playerData.title"
      @update:open="playerVisible = false" />
    <UploadModal :open="uploadModalVisible" @refreshList="getVodDataList" @update:open="uploadModalVisible = false"
      @callback="onCallback" />
  </div>
</template>

<script setup>
import { onMounted, reactive, ref } from 'vue';
import { vodApi } from '@/api';
import { PlusOutlined } from '@ant-design/icons-vue';
import VodCard from './card.vue';
import UploadModal from './upload.vue';
import VodPlayer from './player.vue'

const uploadModalVisible = ref(false);
const playerVisible = ref(false);
const playerData = reactive({
  url: '',
  title: '',
});

//获取点播数据请求参数
const vodParams = reactive({
  start: 0,
  limit: 10,
  sort: "", //排序字段
  order: "", //排序顺序 允许值: ascending, descending
  q: "", //查询参数
});

const vodData = reactive({
  items: [],
  total: 0,
});

onMounted(() => {
  getVodDataList();
})

const getVodDataList = () => {
  vodApi.getVodList(vodParams).then(res => {
    vodData.items = res.data.rows
    vodData.total = res.data.total
    console.log(vodData);
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

// 点击 vod
const onPlayVod = (item) => {
  playerData.url = item.videoUrl
  playerData.title = item.name
  playerVisible.value = true
}

const onDeleteVod = (id) => {
  console.log('删除 vod ', id);
}

</script>
