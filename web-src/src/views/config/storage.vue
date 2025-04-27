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
const popconfirmOpen = ref(false)
const formState = ref({
    "path": "./r",
    "days": 3,
    "disk_usage_ratio": 85.95,
    "fragment_duration_s": 30,
});
const onSubmit = (type) => {
    base.postConfigStorage(formState.value, type).then(res => {
        if (res.status == 200) {
            notification.success({ description: "修改成功!" });
        }
        popconfirmOpen.value = false
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
            <a-form-item label="录像存储路径">
                <a-input  v-model:value="formState.path" />
            </a-form-item>
            <a-form-item label="录像保存天数 (天)">
                <a-input-number style="width: 120px" v-model:value="formState.days" />
                
            </a-form-item>
            <a-form-item label="磁盘存储最大阈值 (%)">
                <a-input-number style="width: 120px" v-model:value="formState.disk_usage_ratio" :step="0.01"/>
            </a-form-item>
            <a-form-item label="录像分片时长 (秒)">
                <a-input-number style="width: 120px" v-model:value="formState.fragment_duration_s" />
            </a-form-item>
            <a-form-item>
                <br>
                <a-popconfirm :open="popconfirmOpen" title="重启后生效?" ok-text="保存" @confirm="onSubmit(false)">
                    <template #cancelButton>
                        <a-button size="small" type="primary" danger @click="onSubmit(true)">重启</a-button>
                        <a-button size="small" style="margin-left: 12px;" @click="popconfirmOpen =false" >取消</a-button>
                    </template>
                    <a-button type="primary" @click="popconfirmOpen =true">保存</a-button>
                </a-popconfirm>
            </a-form-item>
        </a-form>
</template>
