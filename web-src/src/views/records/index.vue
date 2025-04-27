<script setup>
import { ref, computed, createVNode, watch, onBeforeUnmount } from 'vue';
import { records } from "@/api";
import { copyText } from "@/utils";
import { notification, Modal } from 'ant-design-vue'
import { useRouter } from 'vue-router'
import dayjs from 'dayjs'
import { SwitcherOutlined, DeleteOutlined, ExclamationCircleOutlined, PlayCircleOutlined, PoweroffOutlined, CopyOutlined } from '@ant-design/icons-vue'
import { useUserStore } from '@/store/business/user.js'
const userStore = useUserStore()
const router = useRouter()
const columns = [
    { title: 'ID', width: 80, dataIndex: 'id', key: 'id' },
    { title: '名称', width: 160, dataIndex: 'name', key: 'name' },
    { title: '状态', align: 'center', width: 80, dataIndex: 'online', key: 'online' },
    { title: '更新时间', align: 'center', width: 100, dataIndex: 'updated_at', key: 'updated_at' },
    {
        title: '操作',
        key: 'operation',
        fixed: 'right',
        align: 'center',
        width: 80,
    },
];
const dataList = ref([])
const dataListAll = ref([])
const openModal = ref(false)

const search = ref("")
const queryParams = {
    current: 1,
    pageSize: 10,
    total: 0,
    name: undefined,
    type: undefined,
    q:undefined
}
const onSearch = () => {
    if (search.value=='') {
        querySliceData()
        queryParams.q = undefined
    } else {
        queryParams.q = search.value
        onSearchAll()
    }
}

const onSearchAll = () => {
    let list = []
    dataListAll.value.forEach(item => {
        if ( item.name.indexOf(queryParams.q) !== -1) {
            list.push(item)
        }
    });
    dataList.value = [...list]
}

const onSeeStart = (value) => {
    router.push({ name: "RecordsChannel",params:{id:value.id,date:`${dayjs().format('YYYYMMDD')}`} })
}

const onClose = () => {
}
watch(() => openModal.value, (newValue) => {

}, { deep: true })

const onDel = (text) => {
    Modal.confirm({
        title: `确定要删除 “${text.name}” 吗?`,
        icon: createVNode(ExclamationCircleOutlined),
        okText: '确定',
        okType: 'danger',
        cancelText: '取消',
        onOk() {
        },
        onCancel() {
        },
    });
}

const queryData = () => {
    records.getRecordsCloudList({
        page: queryParams.current,
        size: queryParams.pageSize,
        name: queryParams.q,
    }).then(res => {
        if (res.status == 200) {
            dataListAll.value = res.data.data||[]
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
queryData()
onBeforeUnmount(() => {
})
</script>
<template>
    <div class="table-box">
        <a-flex justify="space-between" class="p20px">
            <a-flex justify="flex-end" style="width: 100%;">
                <a-input-search class="ml16px" v-model:value="search" placeholder="请输入关键字..." style="width: 200px"
                    @change="onSearch" />
            </a-flex>
        </a-flex>
        <a-table :columns="columns" :data-source="dataList" :pagination="false" :scroll="{ x: 900 }">
            <template #bodyCell="{ column, text, record }">
                <template v-if="column.key === 'operation'">
                    <a-button type="primary" shape="circle" class="mr5px" @click="onSeeStart(record)">
                        <!-- <PlayCircleOutlined /> -->
                        <SwitcherOutlined />
                    </a-button>
                    <a-button type="primary" danger shape="circle" class="ml5px" @click="onDel(record)">
                        <DeleteOutlined />
                    </a-button>
                </template>
                <template v-if="column.key === 'online'">
                    <a-tag class="mr0px" color="success" v-if="record.online ">录制中</a-tag>
                    <a-tag class="mr0px" color="error" v-else>暂停</a-tag>
                </template>

            </template>
        </a-table>
        <div class="pagination-box p10px" v-if="search == ''">
            <a-pagination v-model:current="queryParams.current" v-model:pageSize="queryParams.pageSize" :size="small"
                @change="handlePageChange" :total="queryParams.total" show-less-items />
        </div>

        <a-modal v-model:open="openModal" title="回放" width="760px" @close="onClose">
            <div class="h400px">
            </div>
            <template #footer>
                <a-flex justify="flex-end">
                    <a-button @click="openModal = false" class="ml16px">关闭</a-button>
                </a-flex>
            </template>
        </a-modal>

    </div>
</template>
<style scoped lang="less">

</style>
