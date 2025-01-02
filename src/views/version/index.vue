
<script setup>
import { computed } from 'vue';
import { base } from "@/api";
import { notification } from 'ant-design-vue'
import { useBaseStore } from '@/store/business/base.js'
const baseStore = useBaseStore()
const onReboot = (text) => {
    base.setReboot().then(res => {
        if (res.status == 200) {
            notification.success({ description: "正在重启，请稍后访问!" });
        }
    })
}
const info = computed(() => baseStore.serverInfo)
</script>
<template>
    <a-card class="p20px version-box">
        <a-flex gap="4">
            <div>系统平台: </div>
            <div> {{ info.server }}</div>
        </a-flex >
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
        <a-popconfirm
            title="确认需要重启吗?"
            ok-text="确认"
            cancel-text="取消"
            @confirm="onReboot"
        >
            <a-button type="link" danger class="mt12px " style="margin-left: -16px;">重启服务</a-button>
        </a-popconfirm>
        
    </a-card>
</template>
<style scoped lang="less">
.version-box {
    font-size: 18px;
    line-height: 42px;
}

</style>