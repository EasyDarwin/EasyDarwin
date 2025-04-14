<script setup>
import { ref, computed, createVNode, watch, onBeforeUnmount } from 'vue';
import { records } from "@/api";
import { copyText, downLoad } from "@/utils";
import { notification, Modal } from 'ant-design-vue'
import { useRouter, useRoute } from 'vue-router'
import dayjs from 'dayjs'
import { LeftOutlined, DeleteOutlined, ExclamationCircleOutlined, PlayCircleOutlined, CopyOutlined, DownloadOutlined } from '@ant-design/icons-vue'
import { useUserStore } from '@/store/business/user.js'
import RecordsDate from './date.vue'
import EasyPlayerPro from '@/components/Player/vod.vue'
const userStore = useUserStore()
const router = useRouter()
const route = useRoute()

const columns = [
    { title: '序号', width: 40, dataIndex: 'index', key: 'index' },
    // { title: 'ID', width: 40, dataIndex: 'id', key: 'id' },
    // { title: '名称', width: 100, dataIndex: 'name', key: 'name', ellipsis: true },
    { title: '文件名称', width: 100, dataIndex: 'file_name', key: 'file_name', ellipsis: true },
    // { title: '地址', width: 120, dataIndex: 'url', key: 'url' },
    { title: '时长',  align: 'center',width: 60, dataIndex: 'duration', key: 'duration' },
    { title: '创建时间', align: 'center', width: 100, dataIndex: 'start', key: 'start' },
    {
        title: '操作',
        key: 'operation',
        fixed: 'right',
        align: 'center',
        width: 100,
    },
];

const dateYear = ref(0)
const dateMonth = ref(0)
const dateDay = ref(0)

const openModal = ref(false)
const videoUrl = ref("")
const channelName = ref("")
const dataList = ref([])
const dataListAll = ref([])
const dateDayList = ref([])
const search = ref("")
const queryParams = {
    current: 1,
    pageSize: 10,
    total: 0,
    name: undefined,
    type: undefined
}
const onSearch = () => {
    if (search.value == '') {
        queryParams.q = undefined
    } else {
        queryParams.q = search.value
    }
    queryData()

}

const onClose = () => {
    openModal.value = false
}
const onReturn = () => {
    router.push({ name: "Records" })
}


watch(() => openModal.value, (newValue) => {

}, { deep: true })

const onTime = (s, e) => {
    console.log(s, e);
}
const onDay = (y, m, d) => {
    router.push({ params: { date: `${dayjs(`${y}-${m}-${d}`).format('YYYYMMDD')}` } })

}
const onDate = (y, m) => {
    router.push({ params: { date: `${dayjs(`${y}-${m}-${dateDay.value}`).format('YYYYMMDD')}` } })

}
const initDate = () => {
    let v = dayjs(route.params.date)
    if (Number.isNaN(v.$y) || Number.isNaN(v.$M) || Number.isNaN(v.$D)) {
        date = dayjs()
        dateYear.value = v.$y
        dateMonth.value = v.$M + 1
        dateDay.value = v.$D
        router.push({ params: { date: `${dayjs(`${v.$y}-${v.$M + 1}-${v.$D}`).format('YYYYMMDD')}` } })
    } else {
        dateYear.value = v.$y
        dateMonth.value = v.$M + 1
        dateDay.value = v.$D
    }
}
const getMp4Url = (text) => {
    let t = dayjs(route.params.date).format("YYYYMMDD")
    let id = route.params.id
    // return `${location.origin}/api/records/hls/stream/${id}/${t}/${text}`
    return `http://127.0.0.1:10086/api/records/hls/stream/${id}/${t}/${text}`
}
const onPlayStart = (text) => {
    openModal.value = true
    // videoUrl.value = "http://127.0.0.1:3001/test2.mp4"
    videoUrl.value = getMp4Url(text.file_name)
    
}
const onCopy = (text) => {
    copyText(getMp4Url(text.file_name))
}
const onDow = (text) => {
    // downLoad("http://127.0.0.1:3001/test2.mp4","test")
    downLoad(getMp4Url(text.file_name),text.file_name)
}
const onDel = (text) => {
    Modal.confirm({
        title: `确定要删除 “${text.name}” 吗?`,
        icon: createVNode(ExclamationCircleOutlined),
        okText: '确定',
        okType: 'danger',
        cancelText: '取消',
        onOk() {
            // xx.xx(text.id).then(res => {
            //     if (res.status == 200) {
            //         notification.success({ description: "删除成功!" });
            //         queryData()
            //     }
            // })
        },
        onCancel() {
        },
    });
}

const queryData = () => {
    const start = dayjs(route.params.date).startOf('day'); // 当天开始
    const end = dayjs(route.params.date).endOf('day');     // 当天结束
    records.getRecordsCloudInfo(route.params.id, { start_ms: start.valueOf(), end_ms: end.valueOf() }).then(res => {
        if (res.status == 200) {
            dataListAll.value = res.data.data || []
            channelName.value = res.data.name || []
            queryParams.total = dataListAll.value.length
            querySliceData()
        }
    }).catch(err => {
    })
};

const handlePageChange = (page, pageSize) => {
    queryParams.current = page
    queryParams.pageSize = pageSize
    querySliceData()
};
const querySliceData = () => {
    let start = (queryParams.current-1)*queryParams.pageSize
    let end = queryParams.pageSize*queryParams.current
    let len = dataListAll.value.length
    if (end>len)end = len
    let list = dataListAll.value.slice(start, end);
    dataList.value = [...list]
}

const queryDataMonth = () => {
    records.getRecordsCloudMonth(route.params.id, { dates: dayjs(route.params.date).format('YYYYMM') }).then(res => {
        if (res.status == 200) {
            let str = res.data.data || ""
            let array = str.split("")
            let list = []
            if (array.length > 0) {
                for (let index = 0; index < array.length; index++) {
                    const element = array[index];
                    if (element == '1') {
                        list.push(index + 1)
                    }

                }
            }
            dateDayList.value = [...list]
        }
    }).catch(err => {
    })

};

initDate()
queryDataMonth()
queryData()
onBeforeUnmount(() => {
})
</script>
<template>
    <div class="table-box">
        <a-flex justify="space-between" class="p20px">
            <div>
                <a-button type="primary" @click="onReturn">
                    <LeftOutlined />返回
                </a-button>
            </div>
            <span>（{{channelName}}）通道</span>
            <a-flex justify="flex-end">
                <RecordsDate :data-day="dateDayList" :year="dateYear" :month="dateMonth" :day="dateDay" @day="onDay"
                    @date="onDate" @time="onTime" />
                <!-- <a-input-search class="ml16px" v-model:value="search" placeholder="请输入关键字..." style="width: 200px"
                    @change="onSearch" /> -->
            </a-flex>
        </a-flex>
        <a-table :columns="columns" :data-source="dataList" :pagination="false" :scroll="{ x: 1000 }">
            <template #bodyCell="{ column, text, record }">
                <template v-if="column.key === 'operation'">
                    <a-button type="primary" shape="circle" class="mr5px" @click="onPlayStart(record)">
                        <PlayCircleOutlined />
                    </a-button>
                    <a-button type="primary" shape="circle" class="mr5px" @click="onDow(record)">
                        <DownloadOutlined />
                    </a-button>
                    <a-button type="primary" shape="circle" class="mr5px" @click="onCopy(record)">
                        <CopyOutlined />
                    </a-button>
                    <a-button type="primary" danger shape="circle" class="ml5px" @click="onDel(record)">
                        <DeleteOutlined />
                    </a-button>
                </template>
                <template v-if="column.key === 'duration'">
                    <a-button type="info">
                        {{ parseInt(record.duration) }}秒
                    </a-button>
                </template>
                <template v-if="column.key === 'start'">
                    <a-button type="info">
                        {{ dayjs(record.start).format('YYYY-MM-DD HH:mm:ss') }}
                    </a-button>
                </template>
            </template>
        </a-table>
        <div class="pagination-box p10px">
            <a-pagination v-model:current="queryParams.current" :size="small"
                @change="handlePageChange" :total="queryParams.total" show-less-items />
        </div>

        <a-modal v-model:open="openModal" title="直播" width="760px" @close="onClose">
            <div class="h400px">
                <EasyPlayerPro :videoUrl="videoUrl" v-if="openModal" />
            </div>
            <template #footer>
                <a-flex justify="flex-end">

                    <a-input v-model:value="videoUrl" disabled>
                        <template #addonAfter>
                            <CopyOutlined class="cp" @click="copyText(videoUrl)" />
                        </template>
                    </a-input>
                    <a-button @click="openModal = false" class="ml16px">关闭</a-button>
                </a-flex>
            </template>
        </a-modal>

    </div>
</template>
<style scoped lang="less"></style>