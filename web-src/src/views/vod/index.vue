<template>
  <div>
    <div class="bg-white rounded-md cursor-pointer p-2 flex justify-between items-center">
      <a-button type="primary" @click="onClickUpload">
        <template #icon>
          <PlusOutlined />
        </template>
        上传视频
      </a-button>

      <a-input-search class="w-68" v-model:value="vodParams.q" placeholder="请输入视频名称" enter-button @search="onSearch" />
    </div>

    <div class="mt-5">
      <template v-if="vodData.items.length > 0">
        <a-row :gutter="[16, 16]">
          <a-col :xs="24" :sm="24" :md="12" :lg="8" :xl="6" :xxl="4" v-for="(item, index) in vodData.items"
            :key="item.id">
            <VodCard :data="item" @on-click="onPlayVod" @on-delect="onDeleteVod" @on-retran="onRetran" @on-edit="onEidt"
              @refresh="getVodDataList" />
          </a-col>
        </a-row>
      </template>
      <template v-else>
        <div class="p-2 bg-white rounded-md">
          <a-empty :image="simpleImage" />
        </div>
      </template>
    </div>

    <VodPlayer :open="playerVisible" :url="playerUrl" @update:open="onPlayerCancel" />
    <VodEdit ref="editRef" @refresh="getVodDataList" />
    <UploadModal :open="uploadModalVisible" @refreshList="getVodDataList" @update:open="uploadModalVisible = false"
      @callback="onCallback" />
  </div>
</template>

<script setup>
import { onMounted, reactive, ref, watch } from 'vue';
import { vodApi } from '@/api';
import { PlusOutlined } from '@ant-design/icons-vue';
import VodCard from './card.vue';
import UploadModal from './upload.vue';
import VodPlayer from './player.vue';
import VodEdit from './edit.vue';
import { message } from 'ant-design-vue';
import { debounce } from 'lodash-es'
import { Empty } from 'ant-design-vue';
const simpleImage = Empty.PRESENTED_IMAGE_SIMPLE;

const editRef = ref();

const uploadModalVisible = ref(false);
const playerVisible = ref(false);
const playerUrl = ref('');

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
  }).catch(err => {
    console.log(err);
  })
}

// 搜索
const onSearch = (e) => {
  getVodDataList();
}

// 防抖包装，避免每次输入都触发
const debounceSearch = debounce(() => {
  getVodDataList();
}, 500);

// 监听搜索词变化，触发防抖搜索
watch(() => vodParams.q, () => {
  debounceSearch();
});

const onClickUpload = () => {
  uploadModalVisible.value = true
}

const onCallback = () => {
  // getVodDataList();
}

// 点击 vod
const onPlayVod = (item) => {
  playerUrl.value = item.videoUrl
  playerVisible.value = true
}

// 关闭播放
const onPlayerCancel = () => {
  playerVisible.value = false;
  playerUrl.value = ''
}

// 点击编辑
const onEidt = (item) => {
  const data = {
    id: item.id,
    name: item.name,
    shared: item.shared,
    sharedLink: item.sharedLink,
    snapUrl: item.snapUrl
  }
  editRef.value.open(data)
}

// 点击删除
const onDeleteVod = (id) => {
  vodApi.deleteVod(id).then(res => {
    if (res.data.code == 200) {
      getVodDataList()
    }
  }).catch(err => {
    message.error('删除失败')
  })
}

const onRetran = (id) => {
  vodApi.vodRetran(id).then(res => {
    getVodDataList()
  })
}
</script>
