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
    "rtmps_enable": false,
    "addr": 0,
    "rtmps_addr": 0,
    "rtmp_over_quic_enable": false,
    "rtmp_over_quic_addr": 0,
    "merge_write_size": 0,
    "pub_timeout_sec": 0,
    "pull_timeout_sec": 0
});

const onSubmit = () => {
    base.postConfigRtmp(formState.value, true).then(res => {
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
            <a-form-item label="数据合并(Byte)">
                <span class="info" style="display: block;">将小包数据合并发送，提高服务器性能，为0则不合并发送。</span>
                <a-input-number style="width: 120px" v-model:value="formState.merge_write_size" />
            </a-form-item>
            <a-form-item label="推流超时(s)">
                <a-input-number style="width: 120px" v-model:value="formState.pub_timeout_sec" />
            </a-form-item>
            <a-form-item label="拉流超时(s)">
                <a-input-number style="width: 120px" v-model:value="formState.pull_timeout_sec" />
            </a-form-item>
            <a-form-item label="启用RTMP">
                <a-radio-group v-model:value="formState.enable">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="RTMP 端口">
                <a-input-number style="width: 120px" v-model:value="formState.addr" :min="1"
                    :max="65535" />
            </a-form-item>
            <a-form-item label="启用RTMPS">
                <a-radio-group v-model:value="formState.rtmps_enable">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="RTMPS 端口">
                <a-input-number style="width: 120px" v-model:value="formState.rtmps_addr" :min="1"
                    :max="65535" />
            </a-form-item>
        
            <a-form-item label="启用RTMP_OVER_QUIC">
                <a-radio-group v-model:value="formState.rtmp_over_quic_enable">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="RTMP_OVER_QUIC 端口">
                <a-input-number style="width: 120px" v-model:value="formState.rtmp_over_quic_addr" :min="1"
                    :max="65535" />
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
