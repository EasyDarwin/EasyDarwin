<script setup>
import { ref, onBeforeUnmount, watch } from 'vue';
import { live, records } from "@/api";
import { useI18n } from 'vue-i18n'
import { notification, Modal } from 'ant-design-vue'
import { EditOutlined, } from '@ant-design/icons-vue'
import { useUserStore } from '@/store/business/user.js'
import ImageBox from '@/components/Image/index.vue'
const { t } = useI18n()
const props = defineProps({
    planId: {
        type: Number,
        default: 0,
    },
})
const userStore = useUserStore()
const columns = [
    { title: 'ID', width: 60, dataIndex: 'id', key: 'id' },
    { title: '名称', width: 120, dataIndex: 'name', key: 'name' },
    { title: '状态', align: 'center', width: 40, dataIndex: 'online', key: 'online' },
    { title: '类型', align: 'center', width: 40, dataIndex: 'liveType', key: 'liveType' },
    { title: '启用', align: 'center', width: 40, dataIndex: 'enable', key: 'enable' },
    { title: '快照', align: 'center', width: 40, dataIndex: 'snapURL', key: 'snapURL' },
];
const dataList = ref([])
const search = ref("")
const liveType = ref("")
const queryParams = {
    current: 1,
    pageSize: 10,
    total: 0,
    q: undefined,
    type: undefined
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
const delPlanChannel = (ids) => {
    records.delRecordsPlansChannels(props.planId, {
        channel_ids: ids
    }).then(res => {
        if (res.status == 200) {
            queryData()
        }
    })
}
const addPlanChannel = (ids) => {
    records.postRecordsPlansChannels(props.planId, {
        channel_ids: ids
    }).then(res => {
        if (res.status == 200) {
            queryData()
        }
    })
}
const selectedRowKeys = ref([])
const queryData = () => {
    live.getLiveList({
        page: queryParams.current,
        size: queryParams.pageSize,
        type: queryParams.type,
        plan_id: props.planId,
        q: queryParams.q,
    }).then(res => {
        if (res.status == 200) {
            selectedRowKeys.value = []
            queryParams.total = res.data.total
            dataList.value = res.data.items
            dataList.value.forEach(element => {
                if (element.RecordPlanEnabled) {
                    selectedRowKeys.value.push(element.id)
                }
            });
        }

    }).catch(err => {
    })
};

const handlePageChange = (page) => {
    queryParams.current = page
    queryData()
};

const rowSelection = {
    selectedRowKeys: selectedRowKeys,
    onSelect: (record, selected, selectedRows, nativeEvent) => {
        let ids = [`${record.id}`]
        if (selected) {
            addPlanChannel(ids)
        } else {
            delPlanChannel(ids)
        }
    },
    onSelectAll: (selected, record, selectedRows, nativeEvent) => {
        let ids = []
        selectedRows.forEach(element => {
            ids.push(`${element.id}`)
        });
        if (selected) {
            addPlanChannel(ids)
        } else {
            delPlanChannel(ids)
        }

    }
};
watch(() => props.planId,  () => {
    queryData()
},{ deep: true }) 
queryData()
onBeforeUnmount(() => {

})
</script>
<template>
    <br>
    <a-flex justify="flex-end">
        <a-flex justify="flex-end">
            <a-select v-model:value="liveType" style="width: 80px" @change="onLiveType">
                <a-select-option value="">全部</a-select-option>
                <a-select-option value="pull">拉流</a-select-option>
                <a-select-option value="push">推流</a-select-option>
            </a-select>
            <a-input-search class="ml16px" v-model:value="search" placeholder="请输入关键字..." style="width: 200px"
                @change="onSearch" />
        </a-flex>
    </a-flex>

    <br>
    <a-table :columns="columns" :data-source="dataList" :pagination="false" rowKey="id" :row-selection="rowSelection"
        :scroll="{ x: 700 }">
        <template #bodyCell="{ column, text, record }">
            <template v-if="column.key === 'liveType'">
                <a-tag class="mr0px" color="success" v-if="record.liveType == 'pull'">拉流</a-tag>
                <a-tag class="mr0px" color="warning" v-else-if="record.liveType == 'push'">推流</a-tag>
            </template>
            <template v-if="column.key === 'online'">
                <a-tag class="mr0px" color="success" v-if="record.online == 1">在线</a-tag>
                <a-tag class="mr0px" color="success" v-else-if="record.online == 2">直播中</a-tag>
                <a-tag class="mr0px" color="default" v-else>离线</a-tag>
            </template>
            <template v-if="column.key === 'enable'">
                <a-tag class="mr0px" color="success" v-if="record.enable">启用</a-tag>
                <a-tag class="mr0px" color="default" v-else>未启用</a-tag>
            </template>
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
        </template>
    </a-table>
    <div class="pagination-box p10px">
        <a-pagination v-model:current="queryParams.current" v-model:pageSize="queryParams.pageSize" :size="small"
            @change="handlePageChange" :total="queryParams.total" show-less-items />
    </div>
</template>