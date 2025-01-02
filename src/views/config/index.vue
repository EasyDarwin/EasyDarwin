<script setup>
import { reactive, ref } from 'vue';
import { base } from "@/api";
import { getTokenStorage } from '@/utils/storage'
import { notification } from 'ant-design-vue'
import { ToolOutlined, UploadOutlined } from '@ant-design/icons-vue';
const headers = ref({
    "authorization":`Bearer ${getTokenStorage()}`
})
const formState = ref({
    disabled_captcha: true,
    https_listen_addr: "",
    https_cert_file: "",
    https_key_file: "",
});
const getBaseInfo = () => {
    base.getConfigBase().then(res => {
        if (res.status == 200) {
            formState.value = res.data;
        }
    })
};
const handleChange = (v) => {
    if (v.file&&v.file.xhr&&v.file.xhr.status==200) {
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
getBaseInfo()
const labelCol = {
    style: {
        width: '160px',
    },
};
</script>
<template>
    <a-card class="config-box">
        <h3 class="fw600">基础配置</h3>
        <a-divider />
        <a-form :model="formState" :label-col="labelCol">
            <a-form-item label="HTTPS 端口">
                <a-input-number style="width: 140px" v-model:value="formState.https_listen_addr" :min="1"
                    :max="65535" />
                <span class="info">修改后重启生效</span>
                
            </a-form-item>
            <a-form-item label="HTTPS Cert 文件路径">
                <a-input-group compact>
                    <a-input v-model:value="formState.https_cert_file" disabled style="width: calc(100% - 46px)" />
                    <a-upload name="file"
                        action="/api/v1/configs/upload/cert" :headers="headers"
                        :showUploadList="false"
                        @change="handleChange">
                        <a-button class="brtl">
                            <UploadOutlined />
                        </a-button>
                    </a-upload>
                </a-input-group>
                <span class="info">修改后重启生效</span>
            </a-form-item>
            <a-form-item label="HTTPS key 文件路径">
                <a-input-group compact>
                    <a-input v-model:value="formState.https_key_file" disabled style="width: calc(100% - 46px)" />
                    <a-upload  name="file"
                        action="/api/v1/configs/upload/key" :headers="headers"
                         :showUploadList="false"
                        @change="handleChange">
                        <a-button class="brtl">
                            <UploadOutlined />
                        </a-button>
                    </a-upload>
                </a-input-group>
                <span class="info">修改后重启生效</span>
            </a-form-item>
            <a-form-item label="登录验证码">
                <a-radio-group v-model:value="formState.disabled_captcha">
                    <a-radio-button :value="false">开启</a-radio-button>
                    <a-radio-button :value="true">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>

            <a-form-item>
                <a-popconfirm
                    title="保存后系统将会重启?"
                    ok-text="确认"
                    cancel-text="取消"
                    @confirm="onSubmit"
                >
                <a-button class="ml160px" type="primary">保存</a-button>
            </a-popconfirm>
            </a-form-item>
        </a-form>
    </a-card>
</template>
<style lang="less">
.config-box {
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
}
</style>