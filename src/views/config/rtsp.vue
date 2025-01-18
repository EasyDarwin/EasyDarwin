<script setup>
import { ref, watch, computed} from 'vue';
import { base } from "@/api";
import { notification } from 'ant-design-vue'
const props =defineProps({
  info: {
    type: Object,
    default: () => ({}),
  },
})

const formState = ref({
    "enable": false,
    "addr": 0,
    "rtsps_enable": false,
    "rtsps_addr": 0,
    "ws_rtsp_enable": false,
    "ws_rtsp_addr": 0,
    "out_wait_key_frame_flag": false,
    "pub_timeout_sec": 0,
    "pull_timeout_sec": 0,

    "auth_enable": false,
    "auth_method": 0,
    "username": "",
    "password": ""
});

const onSubmit = () => {
    base.postConfigRtsp(formState.value, true).then(res => {
        if (res.status == 200) {
            notification.success({ description: "修改成功!" });
        }
    })
};

formState.value = props.info.config
const labelName = computed(() => props.info.label)
watch(() => props.info,  () => {
    formState.value = props.info.config
  },{ deep: true }) 
</script>
<template>

    <a-form :model="formState" layout="vertical">
            <h3 class="fw600">{{ labelName }}</h3>
            <a-divider />
            <a-form-item label="推流超时(s)">
                <a-input-number style="width: 120px" v-model:value="formState.pub_timeout_sec" />
            </a-form-item>
            <a-form-item label="拉流超时(s)">
                <a-input-number style="width: 120px" v-model:value="formState.pull_timeout_sec" />
            </a-form-item>
            <a-form-item label="等待关键帧数据">
                <span class="info" style="display: block;">发送数据时，是否等待视频关键帧数据再发送</span>
                <a-radio-group v-model:value="formState.enable">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="启用RTSP">
                <a-radio-group v-model:value="formState.enable">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="RTSP 端口">
                <a-input-number style="width: 120px" v-model:value="formState.addr" :min="1"
                    :max="65535" />
            </a-form-item>
            <a-form-item label="启用RTSPS">
                <a-radio-group v-model:value="formState.rtsps_enable">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="RTSPS 端口">
                <a-input-number style="width: 120px" v-model:value="formState.rtsps_addr" :min="1"
                    :max="65535" />
            </a-form-item>
        
            <a-form-item label="启用WS_RTSP">
                <a-radio-group v-model:value="formState.ws_rtsp_enable">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="WS_RTSP 端口">
                <a-input-number style="width: 120px" v-model:value="formState.ws_rtsp_addr" :min="1"
                    :max="65535" />
            </a-form-item>
            <a-form-item label="RTSP鉴权">
                <a-radio-group v-model:value="formState.auth_enable">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="RTSP鉴权方式">
                <a-radio-group v-model:value="formState.auth_method">
                    <a-radio-button :value="0">Basic</a-radio-button>
                    <a-radio-button :value="1">Digest</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="账号">
                <a-input  v-model:value="formState.username" />
            </a-form-item>
            <a-form-item label="密码">
                <a-input  v-model:value="formState.password" />
            </a-form-item>
            <a-form-item>
                <br>
                <a-popconfirm title="保存后系统将会重启?" ok-text="确认" cancel-text="取消" @confirm="onSubmit">
                    <a-button type="primary">保存</a-button>
                </a-popconfirm>
            </a-form-item>
            <a-modal v-model:open="open" width="260px" style="top: 34%" title="添加IP白名单" @ok="handleOk">
                <a-input  v-model:value="inputIp"/>
                <span  style="color: red;" v-if="inputIpErr">IP地址格式不正确</span>
                <br>
            </a-modal>
    </a-form>
</template>
