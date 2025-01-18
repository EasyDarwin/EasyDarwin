<script setup>
import { ref, watch, computed} from 'vue';
import { base } from "@/api";
import { getTokenStorage } from '@/utils/storage'
import { notification } from 'ant-design-vue'
import {  UploadOutlined } from '@ant-design/icons-vue';
const  props =defineProps({
  info: {
    type: Object,
    default: () => ({}),
  },
})
const headers = ref({
    "authorization": `Bearer ${getTokenStorage()}`
})
const formState = ref({
    disabled_captcha: true,
    https_listen_addr: "",
    https_cert_file: "",
    https_key_file: "",
    timeout: 60,
    jwt_secret: "",
});
const handleChange = (v) => {
    if (v.file && v.file.xhr && v.file.xhr.status == 200) {
        notification.success({ description: "上传成功!" });
    }
}

const onSubmit = () => {
    base.postConfigBase(formState.value, true).then(res => {
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
            <h3 class="fw600">{{ labelName }}<a-divider /></h3>
            
            <a-form-item label="HTTPS 端口">
                <a-input-number style="width: 120px" v-model:value="formState.https_listen_addr" :min="1"
                    :max="65535" />
            </a-form-item>
     
            <a-form-item label="HTTPS Cert 文件">
                <a-input-group compact>
                    <a-input v-model:value="formState.https_cert_file" disabled style="width: calc(100% - 46px)" />
                    <a-upload name="file" action="/api/v1/configs/upload/cert" :headers="headers"
                        :showUploadList="false" @change="handleChange">
                        <a-button class="brtl">
                            <UploadOutlined />
                        </a-button>
                    </a-upload>
                </a-input-group>
            </a-form-item>
            <a-form-item label="HTTPS key 文件">
                <a-input-group compact>
                    <a-input v-model:value="formState.https_key_file" disabled style="width: calc(100% - 46px)" />
                    <a-upload name="file" action="/api/v1/configs/upload/key" :headers="headers" :showUploadList="false"
                        @change="handleChange">
                        <a-button class="brtl">
                            <UploadOutlined />
                        </a-button>
                    </a-upload>
                </a-input-group>
            </a-form-item>
            <a-form-item label="JWT 鉴权密钥">
                <span class="info">jwt 秘钥，空串时，每次启动程序将随机赋值</span>
                <a-input v-model:value="formState.jwt_secret"  placeholder="jwt secret"/>
            </a-form-item>
            <a-form-item label="请求超时(s)">
                <a-input-number style="width: 120px" v-model:value="formState.timeout"  />
            </a-form-item>
            <a-form-item label="登录验证码">
                <a-radio-group v-model:value="formState.disabled_captcha">
                    <a-radio-button :value="false">开启</a-radio-button>
                    <a-radio-button :value="true">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item>
                <br>
                <a-popconfirm title="保存后系统将会重启?" ok-text="确认" cancel-text="取消" @confirm="onSubmit">
                    <a-button type="primary">保存</a-button>
                </a-popconfirm>
            </a-form-item>
        </a-form>
</template>
