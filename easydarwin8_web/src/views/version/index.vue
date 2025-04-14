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
        <a-popconfirm title="确认需要重启吗?" ok-text="确认" cancel-text="取消" @confirm="onReboot">
            <a-button type="link" danger class="mt12px " style="margin-left: -16px;">重启服务</a-button>
        </a-popconfirm>

    </a-card>
    <br>
    <a-card class="p20px">
        <a-flex>
            <a-flex flex="auto">
                <div class="version-log">
                    <h2><strong>版本更新日志</strong></h2>
                    <a-divider />
                    <div class="version-log-box">
                        <div>
                            <h1><strong>Next Version</strong></h1>
                            <p>1.[功能] 视频录像</p>
                            <p>2.[功能] 直播拉转推</p>
                        </div>
                        <br>
                        <br>
                        <div id="8-3-2">
                            <h1><strong>EasyDarwin-8.3.2</strong></h1>
                            <span>v8.3.2-20250403</span>
                            <h5><strong>更新</strong></h5>
                            <p>1.[新增] 流列表条目展示各种流地址(FLV/HLS/WebRTC/RTSP/RTMP)输出信息；</p>
                            <p>2.[优化] 启动脚本；</p>
                            <p>3.[修改] 默认按需不勾选；</p>
                            <p>4.[修改] 新增音频开关，默认不开启音频；</p>
                        </div>
                        <div id="8-3-1">
                            <h1><strong>EasyDarwin-8.3.1</strong></h1>
                            <span>v8.3.1-20250120</span>
                            <h5><strong>更新</strong></h5>
                            <p>1.[新增] 配置修改</p>
                            <p>2.[新增] 软重启</p>
                            <p>3.[新增] 点播文件拉流倍速</p>
                            <p>4.[新增] 接口文档</p>
                            <p>5.[修复] H.265直播拉流播放</p>
                            <p>6.[优化] 优化拉流在线检测</p>
                        </div>
                        <br>
                        <br>
                        <div id="8-3-0">
                            <h1><strong>EasyDarwin-8.3.0</strong></h1>
                            <span>v8.3.0-20241221</span>
                            <h5><strong>更新</strong></h5>
                            <p>1.[新增] 拉流直播</p>
                            <p>2.[新增] 推流直播</p>
                        </div>
                    </div>
                </div>
            </a-flex>
            <!-- <div style="width: 200px;margin-left: 20px;margin-top: 6px;">
                <br>
                <br>
                <a-anchor :affix="false" :items="[
                    {
                        key: '1',
                        href: '#8-2-1',
                        title: 'EasyDarwin-8.3.1',
                    },
                    {
                        key: '2',
                        href: '#8-2-0',
                        title: 'EasyDarwin-8.3.0',
                    },

                ]" @click="handleClick"></a-anchor>
            </div> -->
        </a-flex>
    </a-card>
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