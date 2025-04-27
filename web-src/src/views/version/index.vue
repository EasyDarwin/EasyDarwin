<script setup>
import { computed } from 'vue';
import { base } from "@/api";
import { notification } from 'ant-design-vue'
import { useBaseStore } from '@/store/business/base.js'
const baseStore = useBaseStore()
// const info = ref({
//     "name": "",
//     "buildTime": "",
//     "hardware": "",
//     "runtime": "",
//     "server": "",
//     "startTime": "",
//     "version": ""
// })
const onReboot = (text) => {
    base.setReboot().then(res => {
        if (res.status == 200) {
            notification.success({ description: "正在重启，请稍后访问!" });
        }
    })
}
const info = computed(() => baseStore.serverInfo)
const handleClick = (e, link) => {
    e.preventDefault();
    console.log(link);
};
</script>
<template>
    <a-card class="p20px version-box">
        <a-flex gap="4">
            <div>系统平台: </div>
            <div> {{ info.server }}</div>
        </a-flex>
        <a-flex gap="4">
            <div>硬件信息: </div>
            <div> {{ info.hardware }}</div>
        </a-flex>
        <a-flex gap="4">
            <div>服务器时间: </div>
            <div> {{ info.serverTime }}</div>
        </a-flex>
        <a-flex gap="4">
            <div>启动时间: </div>
            <div> {{ info.startTime }}</div>
        </a-flex>
        <a-flex gap="4">
            <div>运行时长: </div>
            <div> {{ info.runtime }}</div>
        </a-flex>
        <a-flex gap="4">
            <div>构建信息: </div>
            <div> {{ info.name }}/v{{ info.version }} (build/{{ info.buildTime }})</div>
        </a-flex>
    </a-card>
    <br>
</template>
<style scoped lang="less">
.version-box {
    font-size: 18px;
    line-height: 42px;
}

.version-log {
    width: 100%;
    .version-log-box {
        // max-height: 580px;
        overflow: auto;
        color: #000;
    }
    >div {
        color: #000;
    }
    h1 {
        font-size: 32px;
        margin-bottom: 12px;
    }

    span {
        display: block;
        color: #000;
        font-weight: 600;
        font-size: 14px;
        margin: 16px 0;
        margin-top: 10px;
    }

    p {
        font-size: 14px;

        margin-left: 10px;
    }

}
</style>
