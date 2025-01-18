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
    "enable_https": false,
    "url_pattern": "",
    "use_memory_as_disk_flag": false,
    "use_m3u8_memory_flag": false,
    "cleanup_mode": 0,
    "sub_session_timeout_ms": 0,
    "sub_session_hash_key": "",
    "out_path": "",
    "fragment_duration_ms": 0,
    "fragment_num": 0,
    "delete_threshold": 0,
    "boundary_by_video": false,
    "fmp4": {
        "enable": false,
        "enable_https": false,
        "url_pattern": ""
    }
});

const onSubmit = () => {
    base.postConfigHls(formState.value, true).then(res => {
        if (res.status == 200) {
            notification.success({ description: "修改成功!" });
        }
    })
};
formState.value = props.info.config
console.log(props.info.config);

const labelName = computed(() => props.info.label)
watch(() => props.info, () => {
    formState.value = props.info.config
}, { deep: true }) 
</script>
<template>

    <a-form :model="formState" layout="vertical">
        <h3 class="fw600">{{ labelName }}</h3>
        <a-divider />

        <a-form-item label="HLS文件清理模式">
            <a-select ref="select" v-model:value="formState.cleanup_mode" style="max-width: 800px;">
                <a-select-option :value="0">不删除m3u8+ts文件，可用于录制等场景</a-select-option>
                <a-select-option :value="1">在输入流结束后删除m3u8+ts文件</a-select-option>
                <a-select-option :value="2">持续删除ts文件，保留`delete_threshold + fragment_num + 1</a-select-option>
            </a-select>
        </a-form-item>
        <a-form-item label="HLS播放超时(ms)">
            <span class="info">统计HLS播放者信息时，判定HLS播放者超时离开的时间，单位是毫秒</span>
            <a-input-number style="width: 120px" v-model:value="formState.sub_session_timeout_ms" />
        </a-form-item>

        <!-- <a-form-item label="HLS文件保存路径">
            <span class="info">HLS的m3u8和文件的输出根目录</span>
            <a-input  v-model:value="formState.out_path" />
        </a-form-item> -->

        <a-form-item label="TS文件切片时长(ms)">
            <span class="info">单个TS文件切片时长，单位毫秒</span>
            <a-input-number style="width: 120px" v-model:value="formState.fragment_duration_ms" />
        </a-form-item>
        <a-form-item label="TS文件个数">
            <span class="info">直播时临时存放的ts文件的数量</span>
            <a-input-number style="width: 120px" v-model:value="formState.fragment_num" />
        </a-form-item>
        <a-form-item label="删除过期TS文件">
            <span class="info">更早过期的ts文件将被删除</span>
            <a-input-number style="width: 120px" v-model:value="formState.delete_threshold" />
        </a-form-item>
        <a-form-item label="启用内存保存HLS录像">
            <span class="info">使用内存取代磁盘，保存m3u8+ts文件</span>
            <a-radio-group v-model:value="formState.use_memory_as_disk_flag">
                <a-radio-button :value="true">开启</a-radio-button>
                <a-radio-button :value="false">关闭</a-radio-button>
            </a-radio-group>
        </a-form-item>
        <a-form-item label="启用内存保存HLS直播">
            <span class="info">使用内存取代磁盘，保存直播临时m3u8+ts文件</span>
            <a-radio-group v-model:value="formState.use_m3u8_memory_flag">
                <a-radio-button :value="true">开启</a-radio-button>
                <a-radio-button :value="false">关闭</a-radio-button>
            </a-radio-group>
        </a-form-item>
        <a-form-item label="是否以视频切片">
            <a-radio-group v-model:value="formState.boundary_by_video">
                <a-radio-button :value="true">开启</a-radio-button>
                <a-radio-button :value="false">关闭</a-radio-button>
            </a-radio-group>
        </a-form-item>
        <a-form-item label="HLS密钥">
            <span class="info">私钥，计算播放者唯一ID时使用</span>
            <a-input v-model:value="formState.sub_session_hash_key" />
        </a-form-item>
        <a-form-item label="HLS文件保存路径">
            <span class="info">HLS的m3u8和文件的输出根目录</span>
            <a-input v-model:value="formState.out_path" />
        </a-form-item>
        <a-tabs type="card">
            <a-tab-pane key="1" tab="HLS">
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
            </a-tab-pane>
            <a-tab-pane key="2" tab="FMP4">
                <a-form-item label="启用HTTP">
                    <a-radio-group v-model:value="formState.fmp4.enable">
                        <a-radio-button :value="true">开启</a-radio-button>
                        <a-radio-button :value="false">关闭</a-radio-button>
                    </a-radio-group>
                </a-form-item>
                <a-form-item label="启用HTTPS">
                    <a-radio-group v-model:value="formState.fmp4.enable_https">
                        <a-radio-button :value="true">开启</a-radio-button>
                        <a-radio-button :value="false">关闭</a-radio-button>
                    </a-radio-group>
                </a-form-item>
                <a-form-item label="地址格式">
                    <a-input v-model:value="formState.fmp4.url_pattern" />
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
