<script setup>
import { ref, computed, createVNode, watch, onBeforeUnmount } from 'vue';
import { records } from "@/api";
import { copyText } from "@/utils";
import { notification, Modal } from 'ant-design-vue'
import { EditOutlined, DeleteOutlined, ExclamationCircleOutlined, PlayCircleOutlined, PoweroffOutlined, CopyOutlined } from '@ant-design/icons-vue'
import { useUserStore } from '@/store/business/user.js'
const userStore = useUserStore()
const columns = [
    { title: 'ID', width: 80, dataIndex: 'id', key: 'id', fixed: 'left', },
    { title: '名称', width: 160, dataIndex: 'name', key: 'name' },
    { title: '快照', align: 'center', width: 80, dataIndex: 'snapURL', key: 'snapURL' },
    { title: '状态', align: 'center', width: 80, dataIndex: 'online', key: 'online' },
    { title: '类型', align: 'center', width: 100, dataIndex: 'liveType', key: 'liveType' },
    { title: '启用', align: 'center', width: 80, dataIndex: 'enable', key: 'enable' },
    // { title: '按需', align: 'center', width: 80, dataIndex: 'onDemand', key: 'onDemand' },
    // { title: '音频', align: 'center', width: 80, dataIndex: 'audio', key: 'audio' },
    { title: '详情', align: 'center', width: 80, dataIndex: 'info', key: 'info' },
    { title: '创建时间', align: 'center', width: 100, dataIndex: 'created_at', key: 'created_at' },
    {
        title: '操作',
        key: 'operation',
        fixed: 'right',
        align: 'center',
        width: 160,
    },
];
const dataList = ref([])
const openModal = ref(false)

const search = ref("")
const queryParams = {
    current: 1,
    pageSize: 10,
    total: 0,
    name: undefined,
    type: undefined
}
const onSearch = () => {
    if (search.value=='') {
        queryParams.q = undefined
    } else {
        queryParams.q = search.value
    }
    queryData()

}

const onAdd = () => {
   
}
const onEdit = (value) => {
   
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
    records.getRecordsPlansList({
        page: queryParams.current,
        size:999,
        name: "",
        ields:"FULL"
    }).then(res => {
        if (res.status == 200) {
            queryParams.total = res.data.total
            dataList.value = res.data.items
            console.log( dataList.value);
        }
    }).catch(err => {
    })
};

const handlePageChange = (page) => {
    queryParams.current = page
    queryData()
};
queryData()
onBeforeUnmount(() => {
})
</script>
<template>
    <div class="table-box">
        <a-flex justify="space-between" class="p20px">
            <a-flex justify="flex-end">
                <a-input-search class="ml16px" v-model:value="search" placeholder="请输入关键字..." style="width: 200px"
                    @change="onSearch" />
            </a-flex>
        </a-flex>
        <a-table :columns="columns" :data-source="dataList" :pagination="false" :scroll="{ x: 1200 }">
            <template #bodyCell="{ column, text, record }">
                <template v-if="column.key === 'operation'">
                    <a-button type="primary" shape="circle" class="mr5px" @click="onPlayStart(record)">
                        <PlayCircleOutlined />
                    </a-button>
                    <a-button type="primary" shape="circle" class="mr5px ml5px" @click="onEdit(record)">
                        <EditOutlined />
                    </a-button>
                    <a-button type="primary" danger shape="circle" class="ml5px" @click="onDel(record)">
                        <DeleteOutlined />
                    </a-button>
                </template>
               
            </template>
        </a-table>
        <div class="pagination-box p10px">
            <a-pagination v-model:current="queryParams.current" v-model:pageSize="queryParams.pageSize" :size="small"
                @change="handlePageChange" :total="queryParams.total" show-less-items />
        </div>

        <a-modal v-model:open="openModal" title="直播" width="760px" @close="onClose">
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