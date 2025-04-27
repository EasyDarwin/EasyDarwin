<script setup>
import { ref, computed, createVNode, watch, onBeforeUnmount } from 'vue';
import { records } from "@/api";
import { copyText } from "@/utils";
import { notification, Modal } from 'ant-design-vue'
import { EditOutlined, DeleteOutlined,LinkOutlined, ExclamationCircleOutlined, PlayCircleOutlined, PoweroffOutlined, CopyOutlined } from '@ant-design/icons-vue'
import { useUserStore } from '@/store/business/user.js'
import PlansChannelTable from './table.vue'
const userStore = useUserStore()
const columns = [
    { title: 'ID', dataIndex: 'id', key: 'id'},
    { title: '名称',  dataIndex: 'name', key: 'name' },
    { title: '启用', align: 'center',  dataIndex: 'enabled', key: 'enabled' },
    // { title: '更新时间', align: 'center', width: 100, dataIndex: 'updated_at', key: 'updated_at' },
    { title: '创建时间', align: 'center', dataIndex: 'created_at', key: 'created_at' },
    {
        title: '关联',
        key: 'operation',
        fixed: 'right',
        align: 'center',
        width: 120,
    },
];
const dataList = ref([])
const openModal = ref(false)
const planId = ref(0)

const search = ref("")
const modalTitle = ref("")
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

const onAdd = () => {}
const onEdit = (value) => {}
const onClose = () => {}
const onLink = (value) => {
    openModal.value = true
    planId.value = value.id
    modalTitle.value = `关联通道(${value.name})`
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
        name: queryParams.q,
        fields:"FULL"
    }).then(res => {
        if (res.status == 200) {
            queryParams.total = res.data.total
            dataList.value = res.data.items
        }
    }).catch(err => {
    })
};
const onSwitch = (types, text) => {
    if (types=="enabled") {
        records.postRecordsPlansList({
            "id": text.id,
            "name": text.name,
            "enabled": text.enabled,
            "plans": text.plans,
            "storage_days": text.storage_days
        }).then(res => {
            if (res.status == 200) {
                queryData()
                notification.success({ description: "更新成功!" });
            }
        })
    }
}
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
            <a-flex justify="flex-end" style="width: 100%;">
                <a-input-search class="ml16px" v-model:value="search" placeholder="请输入关键字..." style="width: 200px"
                    @change="onSearch" />
            </a-flex>
        </a-flex>
        <a-table :columns="columns" :data-source="dataList" :pagination="false" :scroll="{ x: 1200 }">
            <template #bodyCell="{ column, text, record }">
                <template v-if="column.key === 'operation'">
                    <!-- <a-button type="primary" shape="circle" class="mr5px ml5px" @click="onEdit(record)">
                        <EditOutlined />
                    </a-button> -->
                    <!-- <a-button type="primary" danger shape="circle" class="ml5px" @click="onDel(record)">
                        <DeleteOutlined />
                    </a-button> -->
                    <a-button type="primary" shape="circle" class="mr5px ml5px" @click="onLink(record)">
                        <LinkOutlined/>
                    </a-button>
                </template>
                <template v-if="column.key === 'enabled'">
                    <a-switch v-model:checked="record.enabled" @change="onSwitch('enabled', record)" />
                </template>
            </template>
        </a-table>
        <div class="pagination-box p10px">
            <a-pagination v-model:current="queryParams.current" v-model:pageSize="queryParams.pageSize" :size="small"
                @change="handlePageChange" :total="queryParams.total" show-less-items />
        </div>

        <a-modal v-model:open="openModal" :title="modalTitle" width="760px" @close="onClose">
            <div class="h400px">
                <PlansChannelTable :planId="planId"/>
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