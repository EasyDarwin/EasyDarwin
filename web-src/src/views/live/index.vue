<script setup>
import { ref, createVNode, watch, onBeforeUnmount } from 'vue';
import LiveForm from './form.vue'
import { live } from "@/api";
import { copyText } from "@/utils";
import { notification, Modal } from 'ant-design-vue'
import { EditOutlined, DownOutlined, PlusOutlined, ReloadOutlined, DeleteOutlined, ExclamationCircleOutlined, PlayCircleOutlined, PoweroffOutlined, CopyOutlined } from '@ant-design/icons-vue'
import EasyPlayerPro from '@/components/Player/live.vue'
import ImageBox from '@/components/Image/index.vue'
import UrlModal from './url.vue';
import { debounce } from 'lodash-es'

const columns = [
  { title: 'ID', width: 80, dataIndex: 'id', key: 'id', fixed: 'left', },
  { title: '名称', width: 160, dataIndex: 'name', key: 'name' },
  { title: '快照', align: 'center', width: 80, dataIndex: 'snapURL', key: 'snapURL' },
  { title: '状态', align: 'center', width: 80, dataIndex: 'online', key: 'online' },
  { title: '类型', align: 'center', width: 100, dataIndex: 'liveType', key: 'liveType' },
  { title: '启用', align: 'center', width: 80, dataIndex: 'enable', key: 'enable' },
  { title: '全时直播', align: 'center', width: 100, dataIndex: 'onDemand', key: 'onDemand' },
  // { title: '音频', align: 'center', width: 80, dataIndex: 'audio', key: 'audio' },
  { title: '详情', align: 'center', width: 80, dataIndex: 'info', key: 'info' },
  { title: '地址', align: 'center', width: 100, dataIndex: 'url', key: 'url' },
  { title: '创建时间', align: 'center', width: 160, dataIndex: 'created_at', key: 'created_at' },
  {
    title: '操作',
    key: 'operation',
    fixed: 'right',
    align: 'center',
    width: 160,
  },
];
const dataList = ref([])
const timeInterval = ref(null)
const playeInterval = ref(null)
const playeId = ref(null)
const LiveFormRef = ref(null)
const openVideo = ref(false)
const videoUrl = ref("")
const selectUrl = ref("")
const keyStr = ref("")
const streamInfo = ref("")
const selectUrlItems = ref([])
const objPort = ref({
  rtmp: '',
  rtsp: '',
})

const search = ref("")
const liveType = ref("")
const queryParams = {
  current: 1,
  pageSize: 10,
  total: 0,
  q: undefined,
  type: undefined
}

const urlModalRef = ref();

const onClickUrl = (id) => {
  live.postPlayStart(id).then(res => {
    const info = res.data.info;
    const data = {
      http_flv: info.http_flv,
      http_hls: info.http_hls,
      ws_flv: info.ws_flv,
      webrtc: info.webrtc,
      rtmp: info.rtmp,
      rtsp: info.rtsp
    }
    urlModalRef.value.open(data)
  })
}

const infoMouseOver = (item, type) => {
  live.getLiveInfo(item.id).then(res => {
    console.log(res);
  })
  live.getStreamInfo(item.id).then(res => {
    if (res.status == 200) {
      if (res.data.code == 0) {
        let info = res.data.data
        if (info.pub && info.pub.session_id == "") delete info.pub
        if (info.pull && info.pull.session_id == "") delete info.pull

        if (info.subs && info.subs.length >= 1) {
          let arr = []
          info.subs.forEach(element => {
            if (element.session_id != "") {
              arr.push(element)
            }
          });
          info.subs = [...arr]

        } else {
          delete info.subs
        }
        streamInfo.value = JSON.stringify(info, null, 2)
      } else {
        streamInfo.value = JSON.stringify(res.data, null, 2)
      }
      if (type) notification.success({ description: "刷新成功!" });
    }
  })
}
const onTimeStart = () => {
  timeInterval.value = setInterval(() => {
    queryData()
  }, 6000);
}
const onTimeStop = () => {
  clearInterval(timeInterval.value)
  if (timeInterval.value != null) {
    timeInterval.value = null
  }
}
const onLiveType = () => {
  if (liveType.value == '') {
    queryParams.type = undefined
  } else {
    queryParams.type = liveType.value
  }
  queryData()
}
const onSearch = () => {
  if (search.value == '') {
    queryParams.q = undefined
  } else {
    queryParams.q = search.value
  }
  queryData()
}

// 防抖包装，避免每次输入都触发
const debounceSearch = debounce(() => {
  onSearch();
}, 500);

// 监听搜索词变化，触发防抖搜索
watch(() => search.value, () => {
  debounceSearch();
});

const changeSelectUrl = (item) => {
  selectUrl.value = item.value
  keyStr.value = item.key
  if (item.value.includes('rtmp') || item.value.includes('rtsp')) {
    return
  }
  videoUrl.value = item.value
}
const onPlayStart = (text) => {
  if (text.enable == 0) {
    notification.warning({ description: "通道未启用!" });
    return
  }
  if (text.online == 0) {
    notification.warning({ description: "通道已离线!" });
    return
  }
  playeId.value = text.id
  live.postPlayStart(playeId.value).then(res => {
    if (res.status == 200) {
      openVideo.value = true
      let info = res.data.info || {}
      selectUrlItems.value = []
      let url = ""
      for (const key in info) {
        let v = info[key]
        switch (key) {
          case "http_flv":
            url = v
            keyStr.value = "HTTP-FLV"
            selectUrlItems.value.push({ key: 'HTTP-FLV', value: v })
            break;
          case "http_hls":
            selectUrlItems.value.push({ key: 'HTTP-HLS', value: v })
            break;
          case "ws_flv":
            selectUrlItems.value.push({ key: 'WS-FLV', value: v })
            break;
          case "webrtc":
            selectUrlItems.value.push({ key: 'WEBRTC', value: v })
            break;
          case "rtsp":
            selectUrlItems.value.push({ key: 'RTSP', value: v })
            break;
          default:
            break;
        }

      }
      if (url != '') {
        selectUrl.value = url
        videoUrl.value = url
      }

    }
  })
  if (text.liveType == 'pull' && text.onDemand) {
    onPlayStartTime()
  }
}
const onPlayStartTime = () => {
  playeInterval.value = setInterval(() => {
    live.postPlayStart(playeId.value)
  }, 15000);

}
const onPlayerStopTime = () => {
  clearInterval(playeInterval.value)
  if (playeInterval.value != null) {
    playeInterval.value = null
  }
}
const onPlayStop = (text) => {
  if (text.online == 2) {
    live.postPlayStop(text.id).then(res => {
      if (res.status == 200) {
        notification.success({ description: "停流成功!" });
      }
    })
  } else {
    notification.warning({ description: "通道未在直播中!" });
  }

}
const onAdd = () => {
  if (LiveFormRef.value) {
    LiveFormRef.value.setOpen(null);
  }
}
const onEdit = (text) => {
  if (LiveFormRef.value) {
    LiveFormRef.value.setOpen(text);
  }
}
const onSwitch = (types, text) => {
  let key = ""
  let value = null
  switch (types) {
    case "enable":
      key = "enable"
      value = text.enable ? 1 : 0
      break;
    case "onDemand":
      key = "onDemand"
      value = text.onDemand ? 1 : 0
      break;
    case "audio":
      key = "audio"
      value = text.audio ? 1 : 0
      break;
    default:

      break;
  }
  if (key == "" || value == null) {
    notification.error({ description: "参数异常!" });
    return
  }
  live.putLiveOne(text.liveType, text.id, key, value).then(res => {
    if (res.status == 200) {
      queryData()
      notification.success({ description: "更新成功!" });

    }
  })
}

const onClose = () => {
  onPlayerStopTime()
}
watch(() => openVideo.value, (newValue) => {
  if (newValue == false) {
    onPlayerStopTime()
  }
}, { deep: true })
const onOk = () => {
  queryData()
  if (LiveFormRef.value) {
    LiveFormRef.value.setClose();
  }
}
const onDel = (text) => {
  Modal.confirm({
    title: `确定要删除 “${text.name}” 吗?`,
    icon: createVNode(ExclamationCircleOutlined),
    okText: '确定',
    okType: 'danger',
    cancelText: '取消',
    onOk() {
      live.deleteLive(text.id).then(res => {
        if (res.status == 200) {
          notification.success({ description: "删除成功!" });
          queryData()
        }
      })
    },
    onCancel() {
      // console.log('Cancel');
    },
  });
}

const queryData = () => {
  live.getLiveList({
    page: queryParams.current,
    size: queryParams.pageSize,
    type: queryParams.type,
    q: queryParams.q,
  }).then(res => {
    if (res.status == 200) {
      queryParams.total = res.data.total
      dataList.value = res.data.items
      objPort.value.rtmp = res.data.rtmp
      objPort.value.rtsp = res.data.rtsp

    }
  }).catch(err => {
  })
};

const handlePageChange = (page) => {
  queryParams.current = page
  queryData()
};
queryData()
onTimeStart()
onBeforeUnmount(() => {
  onTimeStop()
  onPlayerStopTime()
})
</script>
<template>
  <div class="bg-white rounded-md cursor-pointer p-2 flex justify-between items-center">
    <div>
      <a-button type="primary" @click="onAdd">
        <template #icon>
          <PlusOutlined />
        </template>
        创建直播
      </a-button>
    </div>
    <a-flex justify="flex-end">
      <a-space>
        <a-select v-model:value="liveType" style="width: 80px" @change="onLiveType">
          <a-select-option value="">全部</a-select-option>
          <a-select-option value="pull">拉流</a-select-option>
          <a-select-option value="push">推流</a-select-option>
        </a-select>
        <a-input-search v-model:value="search" placeholder="请输入关键字..." class="w-68" enter-button @search="onSearch" />
      </a-space>
    </a-flex>
  </div>


  <div class="bg-white rounded-md mt-5">

    <a-table :columns="columns" :data-source="dataList" :pagination="false" :scroll="{ x: 1200 }">
      <template #bodyCell="{ column, text, record }">
        <template v-if="column.key === 'operation'">
          <a-button type="primary" shape="circle" class="mr5px" @click="onPlayStart(text)">
            <PlayCircleOutlined />
          </a-button>
          <a-button type="primary" shape="circle" class="mr5px ml5px" @click="onEdit(text)">
            <EditOutlined />
          </a-button>
          <!-- <a-button type="primary" danger shape="circle" class="mr5px ml5px" @click="onPlayStop(text)">
                        <PoweroffOutlined />
                    </a-button> -->
          <a-button type="primary" danger shape="circle" class="ml5px" @click="onDel(text)">
            <DeleteOutlined />
          </a-button>
        </template>
        <template v-if="column.key === 'liveType'">
          <a-tag class="mr0px" color="success" v-if="record.liveType == 'pull'">拉流</a-tag>
          <a-tag class="mr0px" color="warning" v-else-if="record.liveType == 'push'">推流</a-tag>
        </template>
        <template v-if="column.key === 'online'">
          <a-tag class="mr0px" color="success" v-if="record.online == 1">在线</a-tag>
          <a-tag class="mr0px" color="success" v-else-if="record.online == 2">直播中</a-tag>
          <a-tag class="mr0px" color="default" v-else>离线</a-tag>
        </template>
        <template v-if="column.key === 'url'">
          <a-button size="small" @click="onClickUrl(record.id)">查看流地址</a-button>
        </template>
        <template v-if="column.key === 'enable'">
          <a-switch v-model:checked="record.enable" @change="onSwitch('enable', record)" />
        </template>
        <template v-if="column.key === 'onDemand'">
          <a-switch v-model:checked="record.onDemand" @change="onSwitch('onDemand', record)" />
      </template>
      <!-- <template v-if="column.key === 'audio'">
          <a-switch v-model:checked="record.audio" @change="onSwitch('audio', record)" />
      </template> -->
        <template v-if="column.key === 'snapURL'">
          <a-flex justify="center">
            <a-popover placement="left">
              <template #content>
                <div class="w300px h200px">
                  <ImageBox :img-url="record.snapURL" />
                </div>
              </template>
              <template #title></template>
              <div class="w60px h24px">
                <ImageBox :img-url="record.snapURL" />
              </div>
            </a-popover>
          </a-flex>

        </template>
        <template v-if="column.key === 'info'">
          <a-flex justify="center">
            <a-popover placement="left">
              <template #content>
                <pre class="json-display">{{ streamInfo }}</pre>
              </template>
              <template #title>

                <a-flex justify="space-between">
                  <span>视频流详情</span>
                  <ReloadOutlined class="cp" @click="infoMouseOver(record, true)" />
                </a-flex>
              </template>
              <a-button size="small" class="mr0px cp" @mouseover="infoMouseOver(record, false)"
                @click="infoMouseOver(record, true)">详情</a-button>
            </a-popover>
          </a-flex>

        </template>
      </template>
    </a-table>
    <div class="pagination-box p10px">
      <a-pagination v-model:current="queryParams.current" v-model:pageSize="queryParams.pageSize" :size="small"
        @change="handlePageChange" :total="queryParams.total" :show-total="total => `共 ${total} 项`" show-less-items />
    </div>
    <LiveForm ref="LiveFormRef" @ok="onOk" />

    <a-modal v-model:open="openVideo" title="直播" width="760px" @close="onClose">
      <div class="h400px">
        <EasyPlayerPro :videoUrl="videoUrl" v-if="openVideo" />
      </div>
      <template #footer>
        <a-flex justify="flex-end">

          <a-input v-model:value="selectUrl" disabled>
            <template #addonBefore>
              <a-popover placement="right">
                <template #content>
                  <a-flex justify="space-between" v-for="(item, index) in selectUrlItems" class="cp m14px" :key="index">
                    <span>{{ item.key }}</span>
                    <div class="ml16px">
                      <PlayCircleOutlined v-if="item.key != 'RTMP' && item.key != 'RTSP'" :style="{ color: '#0cbb92' }"
                        @click="changeSelectUrl(item)" />
                      <CopyOutlined class="ml16px" :style="{ color: '#0cbb92' }" @click="copyText(item.value)" />
                    </div>
                  </a-flex>

                </template>
                <template #title></template>
                <span class="cp">{{ keyStr }}
                  <DownOutlined />
                </span>
              </a-popover>

            </template>
            <template #addonAfter>
              <CopyOutlined class="cp" @click="copyText(selectUrl)" />
            </template>
          </a-input>
          <a-button @click="openVideo = false" class="ml16px">关闭</a-button>
        </a-flex>
      </template>
    </a-modal>
    <UrlModal ref="urlModalRef" />
  </div>
</template>
<style scoped lang="less">
.json-display {
  width: 400px;
  height: 500px;
  white-space: pre-wrap;
  word-break: break-all;
  font-family: monospace;

}
</style>
