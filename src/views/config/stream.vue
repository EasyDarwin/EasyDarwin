<script setup>
import { ref, watch, computed } from 'vue';
import { base } from "@/api";
import { notification } from 'ant-design-vue'
const props = defineProps({
    info: {
        type: Object,
        default: () => ({}),
    },
})

const formState = ref({
    "enable": false,
    "gop_cache_num": 0,
    "single_gop_max_frame_num": 0,
    "http_flv": {
        "enable": false,
        "enable_https": false,
        "url_pattern": ""
    },
    "http_fmp4": {
        "enable": false,
        "enable_https": false,
        "url_pattern": ""
    },
    "http_ts": {
        "enable": false,
        "enable_https": false,
        "url_pattern": ""
    }
});

const onSubmit = () => {
    base.postConfigStream(formState.value, true).then(res => {
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
        <a-form-item label="API接口">
            <a-radio-group v-model:value="formState.enable">
                <a-radio-button :value="true">开启</a-radio-button>
                <a-radio-button :value="false">关闭</a-radio-button>
            </a-radio-group>
        </a-form-item>
        <a-form-item label="GOP 缓存个数">
            <a-input-number style="width: 120px" v-model:value="formState.gop_cache_num" />
        </a-form-item>
        <a-form-item label="GOP 最大帧数">
            <span class="info">0 表示不限制，此参数可以限制每个 gop 的最大帧数</span>
            <a-input-number style="width: 120px" v-model:value="formState.single_gop_max_frame_num" />
        </a-form-item>
        <a-tabs type="card">
            <a-tab-pane key="1" tab="HTTP_FLV">
                <a-form-item label="启用HTTP">
                    <a-radio-group v-model:value="formState.http_flv.enable">
                        <a-radio-button :value="true">开启</a-radio-button>
                        <a-radio-button :value="false">关闭</a-radio-button>
                    </a-radio-group>
                </a-form-item>
                <a-form-item label="启用HTTPS">
                    <a-radio-group v-model:value="formState.http_flv.enable_https">
                        <a-radio-button :value="true">开启</a-radio-button>
                        <a-radio-button :value="false">关闭</a-radio-button>
                    </a-radio-group>
                </a-form-item>
                <a-form-item label="地址格式">
                    <a-input v-model:value="formState.http_flv.url_pattern" />
                </a-form-item>
            </a-tab-pane>
            <a-tab-pane key="2" tab="HTTP_FMP4">
                <a-form-item label="启用HTTP">
                    <a-radio-group v-model:value="formState.http_fmp4.enable">
                        <a-radio-button :value="true">开启</a-radio-button>
                        <a-radio-button :value="false">关闭</a-radio-button>
                    </a-radio-group>
                </a-form-item>
                <a-form-item label="启用HTTPS">
                    <a-radio-group v-model:value="formState.http_fmp4.enable_https">
                        <a-radio-button :value="true">开启</a-radio-button>
                        <a-radio-button :value="false">关闭</a-radio-button>
                    </a-radio-group>
                </a-form-item>
                <a-form-item label="地址格式">
                    <a-input v-model:value="formState.http_fmp4.url_pattern" />
                </a-form-item>
            </a-tab-pane>
            <a-tab-pane key="3" tab="HTTP_TS">
                <a-form-item label="启用HTTP">
                    <a-radio-group v-model:value="formState.http_ts.enable">
                        <a-radio-button :value="true">开启</a-radio-button>
                        <a-radio-button :value="false">关闭</a-radio-button>
                    </a-radio-group>
                </a-form-item>
                <a-form-item label="启用HTTPS">
                    <a-radio-group v-model:value="formState.http_ts.enable_https">
                        <a-radio-button :value="true">开启</a-radio-button>
                        <a-radio-button :value="false">关闭</a-radio-button>
                    </a-radio-group>
                </a-form-item>
                <a-form-item label="地址格式">
                    <a-input v-model:value="formState.http_ts.url_pattern" />
                </a-form-item>
            </a-tab-pane>
        </a-tabs>
        <a-form-item>
            <br>
            <a-popconfirm title="保存后系统将会重启?" ok-text="确认" cancel-text="取消" @confirm="onSubmit">
                <a-button type="primary">保存</a-button>
            </a-popconfirm>
        </a-form-item>
    </a-form>
</template>
