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
    "dir": "",
    "level": "",
    "max_age": 0,
    "rotation_time": 0,
    "rotation_size": 0
});
const onSubmit = () => {
    base.postConfigBaseLog(formState.value, true).then(res => {
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
      
            <a-form-item label="日志级别">
                <a-select ref="select"  v-model:value="formState.level" style="max-width: 120px;">
                    <a-select-option value="debug">debug</a-select-option>
                    <a-select-option value="info">info</a-select-option>
                    <a-select-option value="warn">warn</a-select-option>
                    <a-select-option value="error">error</a-select-option>
                </a-select>
            </a-form-item>
            <a-form-item label="保留时长(s)">
                <span class="info">保留日志多久，超过时间自动删除</span>
                <a-input-number style="width: 120px" v-model:value="formState.max_age"  />
            </a-form-item>
            <a-form-item label="日志分割(s)">
                <span class="info">多久时间，分割一个新的日志文件</span>
                <a-input-number style="width: 120px" v-model:value="formState.rotation_time" />
            </a-form-item>
            <a-form-item label="日志大小(MB)">
                <span class="info">多大文件，分割一个新的日志文件</span>
                <a-input-number style="width: 120px" v-model:value="formState.rotation_size" />
            </a-form-item>
            <a-form-item label="日志路径">
                <a-input v-model:value="formState.dir" />
            </a-form-item>
            <a-form-item>
                <br>
                <a-popconfirm title="保存后系统将会重启?" ok-text="确认" cancel-text="取消" @confirm="onSubmit">
                    <a-button type="primary">保存</a-button>
                </a-popconfirm>
            </a-form-item>
        </a-form>
</template>
