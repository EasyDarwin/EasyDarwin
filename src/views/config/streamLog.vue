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
    "assert_behavior": 0,
    "level": 0,
    "filename": "",
    "is_to_stdout": false,
    "is_rotate_daily": false,
    "short_file_flag": false,

    "timestamp_flag": false,
    "timestamp_with_ms_flag": false,
    "level_flag": false
});
const onSubmit = () => {
    base.postConfigStreamLog(formState.value, true).then(res => {
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

   
            <a-form-item label="日志断言">
                <a-select ref="select"  v-model:value="formState.assert_behavior" >
                    <a-select-option :value="1">只打印错误日志</a-select-option>
                    <a-select-option :value="2">打印并退出程序</a-select-option>
                    <a-select-option :value="3">打印并panic</a-select-option>
                </a-select>
            </a-form-item>
            <a-form-item label="日志级别">
                <a-select ref="select"  v-model:value="formState.level" >
                    <a-select-option :value="0">trace</a-select-option>
                    <a-select-option :value="1">debug</a-select-option>
                    <a-select-option :value="2">info</a-select-option>
                    <a-select-option :value="3">warn</a-select-option>
                    <a-select-option :value="4">error</a-select-option>
                    <a-select-option :value="5">fatal</a-select-option>
                </a-select>
            </a-form-item>
            <a-form-item label="输出到控制台">
                <span class="info">是否以stdout输出到控制台 TODO(chef): 再增加一个stderr的配置</span>
                <a-radio-group v-model:value="formState.is_to_stdout">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="日志翻转">
                <span class="info">日志按天翻转</span>
                <a-radio-group v-model:value="formState.is_rotate_daily">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="日志行号">
                <span class="info">是否在每行日志尾部添加源码文件及行号的信息</span>
                <a-radio-group v-model:value="formState.short_file_flag">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="日志时间戳">
                <span class="info">是否在每行日志首部添加时间戳的信息</span>
                <a-radio-group v-model:value="formState.timestamp_flag">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="毫秒时间戳">
                <span class="info">时间戳是否精确到毫秒</span>
                <a-radio-group v-model:value="formState.timestamp_with_ms_flag">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="显示级别">
                <span class="info">日志是否包含日志级别字段</span>
                <a-radio-group v-model:value="formState.level_flag">
                    <a-radio-button :value="true">开启</a-radio-button>
                    <a-radio-button :value="false">关闭</a-radio-button>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="日志路径">
                <a-input v-model:value="formState.filename" />
            </a-form-item>
            <a-form-item>
                <br>
                <a-popconfirm title="保存后系统将会重启?" ok-text="确认" cancel-text="取消" @confirm="onSubmit">
                    <a-button type="primary">保存</a-button>
                </a-popconfirm>
            </a-form-item>
        </a-form>
</template>
