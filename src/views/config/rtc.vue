<script setup>
import { ref, watch, computed} from 'vue';
import { base } from "@/api";
import { isValidIP } from "@/utils";
import { notification } from 'ant-design-vue'
import { PlusOutlined, DeleteOutlined } from '@ant-design/icons-vue'
const props =defineProps({
  info: {
    type: Object,
    default: () => ({}),
  },
})
const iceHostNatToIps = ref([])
const inputIp = ref("")
const inputIpErr = ref(false)
const formState = ref({
    "iceHostNatToIps": [],
    "iceUdpMuxPort": 0,
    "iceTcpMuxPort": 0,
    "pub_timeout_sec": 0,
    
    "enable": false,
    "enable_https": false,
    "url_pattern": ""
});

const onSubmit = () => {
    formState.value.iceHostNatToIps = iceHostNatToIps.value
    base.postConfigRtc(formState.value, true).then(res => {
        if (res.status == 200) {
            notification.success({ description: "修改成功!" });
        }
    })
};
const open = ref(false)

const onAdd = () => {
    open.value = true
    inputIpErr.value = false
    inputIp.value = ""
}
const onDel = (index) => {
    iceHostNatToIps.value.splice(index, 1)
}
const handleOk = () => {
    if (isValidIP(inputIp.value)) {
        iceHostNatToIps.value.push(inputIp.value)
        open.value = false
    } else{
        inputIpErr.value = true
    }
}



formState.value = props.info.config
iceHostNatToIps.value = props.info?.config?.iceHostNatToIps||[]
const labelName = computed(() => props.info.label)
watch(() => props.info,  () => {
    formState.value = props.info.config
    iceHostNatToIps.value = props.info?.config?.iceHostNatToIps||[]
  },{ deep: true }) 
</script>
<template>

    <a-form :model="formState" layout="vertical">
            <h3 class="fw600">{{ labelName }}</h3>
            <a-divider />
            <a-form-item label="超时时间(s)">
                <span class="info">发布流，没有流超时时间，自动销毁</span>
                <a-input-number style="width: 120px" v-model:value="formState.pub_timeout_sec" />
            </a-form-item>
            <a-form-item label="UDP 端口">
                <a-input-number style="width: 120px" v-model:value="formState.iceUdpMuxPort" :min="1"
                    :max="65535" />
            </a-form-item>
            <a-form-item label="TCP 端口">
                <a-input-number style="width: 120px" v-model:value="formState.iceTcpMuxPort" />
            </a-form-item>
            <a-form-item label="IP 白名单">
                <a-button  class="ml5px"  v-for="(item, index) in iceHostNatToIps" :key="index" size="small">
                {{ item }}  <DeleteOutlined @click="onDel(index)"/>
                </a-button>
                <a-button type="primary" class="ml5px" shape="circle" @click="onAdd" size="small">
                    <PlusOutlined />
                </a-button>
            </a-form-item>
            <a-form-item label="启用HTTP">
                <a-radio-group v-model:value="formState.enable">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="启用HTTPS">
                <a-radio-group v-model:value="formState.enable_https">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="地址格式">
                <a-input v-model:value="formState.url_pattern" />
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

