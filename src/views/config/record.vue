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
    "enable_flv": false,
    "flv_out_path": "",
    "enable_mpegts": false,
    "mpegts_out_path": "",
    "enable_fmp4": false,
    "fmp4_out_path": ""
});
const onSubmit = () => {
    base.postConfigRecord(formState.value, true).then(res => {
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
            <a-form-item label="启用FLV录制">
                <a-radio-group v-model:value="formState.enable_flv">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="FLV录制路径">
                <a-input  v-model:value="formState.flv_out_path" />
            </a-form-item>
            <a-form-item label="启用FMP4录制">
                <a-radio-group v-model:value="formState.enable_fmp4">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="FMP4录制路径">
                <a-input  v-model:value="formState.fmp4_out_path" />
            </a-form-item>
            <a-form-item label="启用MPEGTS录制">
                <a-radio-group v-model:value="formState.enable_mpegts">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="MPEGTS录制路径">
                <a-input  v-model:value="formState.mpegts_out_path" />
            </a-form-item>
            <a-form-item>
                <br>
                <a-popconfirm title="保存后系统将会重启?" ok-text="确认" cancel-text="取消" @confirm="onSubmit">
                    <a-button type="primary">保存</a-button>
                </a-popconfirm>
            </a-form-item>
        </a-form>
</template>
