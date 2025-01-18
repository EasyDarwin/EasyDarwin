<script setup>
import { ref, computed } from 'vue';
import { base } from "@/api";
// import { useRoute, useRouter } from 'vue-router'
import BaseConfig from './base.vue'
import StreamConfig from './stream.vue'
import DataConfig from './data.vue'
import HlsConfig from './hls.vue'
import RtcConfig from './rtc.vue'
import RtmpConfig from './rtmp.vue'
import RtspConfig from './rtsp.vue'
import RecordConfig from './record.vue'
import BaseLogConfig from './baseLog.vue'
import StreamLogConfig from './streamLog.vue'

// const route = useRoute()
// const router = useRouter()
const titleInfo = ref({})
const index = ref('1')
const selectedKeys = ref([index.value])
const openKeys = ref([])
const items = ref([
    { key: '1', label: '基础配置', title: '基础配置', config:{} },
    { key: '2', label: '流媒体配置', title: '流媒体配置', config:{} },
    { key: '3', label: '数据库配置', title: '数据库配置', config:{} },
    { key: '4', label: 'HLS配置', title: 'HLS配置', config:{} },
    { key: '5', label: 'RTC配置', title: 'RTC配置', config:{} },
    { key: '6', label: 'RTMP配置', title: 'RTMP配置', config:{} },
    { key: '7', label: 'RTSP配置', title: 'RTSP配置', config:{} },
    { key: '8', label: '录像配置', title: '录像配置', config:{} },
    { key: '9', label: '系统日志', title: '系统日志', config:{} },
    { key: '10', label: '流媒体日志', title: '流媒体日志', config:{} },
])

const getInfo = (key) => {
    return items.value.find(item => item.key == key)
}
titleInfo.value = getInfo(index.value)
const handleClick = (val) => {
    titleInfo.value = getInfo(val.key)
    // router.push({ params: { index: val.key } })
}
const getBaseInfo = () => {
    base.getConfigBase().then(res => {
        if (res.status == 200) {
            items.value[0].config = res.data?.base;
            items.value[1].config = res.data?.stream;
            items.value[2].config = res.data?.data;
            items.value[3].config = res.data?.hls;
            items.value[4].config = res.data?.rtc;
            items.value[5].config = res.data?.rtmp;
            items.value[6].config = res.data?.rtsp;
            items.value[7].config = res.data?.record;
            items.value[8].config = res.data?.base_log;
            items.value[9].config = res.data?.stream_log;
        }
    })
};
getBaseInfo()
</script>
<template>
    <a-row class="config-box">
        <a-col flex="240px">
            <a-menu style="border-radius: 8px;" id="dddddd" v-model:openKeys="openKeys"
                v-model:selectedKeys="selectedKeys" mode="inline" :items="items" @click="handleClick"></a-menu>
        </a-col>
        <a-col flex="auto" style="overflow: hidden;height: 100%;">
            <a-card>
                <BaseConfig :info="titleInfo" v-if="selectedKeys[0] == '1'" />
                <StreamConfig :info="titleInfo" v-if="selectedKeys[0] == '2'" />
                <DataConfig :info="titleInfo" v-if="selectedKeys[0]=='3'"/>
                <HlsConfig :info="titleInfo" v-if="selectedKeys[0]=='4'"/>
                <RtcConfig :info="titleInfo" v-if="selectedKeys[0]=='5'"/>
                <RtmpConfig :info="titleInfo" v-if="selectedKeys[0]=='6'"/>
                <RtspConfig :info="titleInfo" v-if="selectedKeys[0]=='7'"/>
                <RecordConfig :info="titleInfo" v-if="selectedKeys[0]=='8'"/>
                <BaseLogConfig :info="titleInfo" v-if="selectedKeys[0]=='9'"/>
                <StreamLogConfig :info="titleInfo" v-if="selectedKeys[0]=='10'"/>
            </a-card>
        </a-col>
    </a-row>

</template>
<style lang="less">
.config-box {
    height: 100%;

    .ant-form-item-control-input-content {
        .info {
            display: block;
            font-size: 14px;
            color: #aaa;
            padding: 4px 0;
        }
    }

    .brtl {
        border-radius: 0 6px 6px 0;
    }

    .ant-input,
    .ant-form-item-control-input {
        max-width: 460px;
    }

    .ant-menu {
        height: 100%;
        padding: 10px 0;
        width: 100%;
        border-radius: 8px 0 0 8px !important;
    }

    .ant-card {
        max-height: 100%;
        height: 100%;
        border: 0;
        overflow: auto;
        border-radius: 0 8px 8px 0 !important;
        position: relative;
        .ant-card-body {
            // height: 100%;
        }
    }
}
</style>