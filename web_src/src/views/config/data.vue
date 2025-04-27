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
const popconfirmOpen = ref(false)
const formState = ref({
    "Dsn": "",
    "MaxIdleConns": 0,
    "MaxOpenConns": 0,
    "ConnMaxLifetime": 0,
    "SlowThreshold": 0
});
const onSubmit = (type) => {
    base.postConfigData(formState.value, type).then(res => {
        if (res.status == 200) {
            notification.success({ description: "修改成功!" });
        }
        popconfirmOpen.value = false
    })
};
formState.value = props.info.config
const labelName = computed(() => props.info.label)
watch(() => props.info, () => {
    formState.value = props.info.config
}, { deep: true }) 
</script>
<template>

    <a-form :model="formState" layout="vertical">
        <h3 class="fw600">{{ labelName }}</h3>
        <a-divider />

        <a-form-item label="最大空闲连接数">
            <a-input-number style="width: 120px" v-model:value="formState.MaxIdleConns" :min="1" :max="65535" />
        </a-form-item>
        <a-form-item label="最大打开连接数">
            <a-input-number style="width: 120px" v-model:value="formState.MaxOpenConns" :min="1" :max="65535" />
        </a-form-item>
        <a-form-item label="连接最大生命周期(s)">
            <a-input-number style="width: 120px" v-model:value="formState.ConnMaxLifetime" />
        </a-form-item>
        <a-form-item label="慢查询阈值(ms)">
            <a-input-number style="width: 120px" v-model:value="formState.SlowThreshold" />
        </a-form-item>
        <a-form-item label="数据库路径">
            <span class="info">默认为相对软件安装目录路径</span>
            <a-input v-model:value="formState.Dsn" />
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
