<script setup>
import { ref, reactive, defineExpose, toRaw } from 'vue';
import { notification } from 'ant-design-vue'
import { live } from "@/api";
import { copyText } from "@/utils";
import { CopyOutlined,ReloadOutlined } from '@ant-design/icons-vue'
const emit = defineEmits(['ok'])
const open = ref(false)
const liveType = ref("pull")
const isAdd = ref(false)
const id = ref(0)
const formState = reactive({});
const onFinish = () => {
    if (isAdd.value) {
        live.postLive(liveType.value,toRaw(formState)).then(res => {
            if (res.status == 200) {
                notification.success({description:"添加成功!"});
                emit("ok")
            }
        })
    } else {
        live.putLive(liveType.value,id.value, toRaw(formState)).then(res => {
            if (res.status == 200) {
                notification.success({description:"修改成功!"});
                emit("ok")
            }
        })
    }
};
const onFinishFailed = errorInfo => {
    console.log('Failed:', errorInfo);
};
const setOpen = (data) => {

    init()
    if (data == null) {
        isAdd.value = true
    } else {
        liveType.value= data.liveType
        isAdd.value = false
        formState.name = data?.name
        formState.url = data?.url
        formState.enable = data?.enable
        formState.onDemand = data?.onDemand
        formState.audio = data?.audio
        formState.transType = data?.transType
        formState.authed = data?.authed
        id.value = data?.id
    }
    open.value = true
};
const onSign = () => {
    live.putLiveOne(liveType.value,id.value, "sign", 0).then(res => {
        if (res.status == 200) {
            queryData()
            notification.success({ description: "更新成功!" });

        }
    })
}
const setClose = () => {
    open.value = false
}
const init = () => {
    formState.name = ''
    formState.url = ''
    formState.authed = true
    formState.enable = true
    formState.onDemand = true
    formState.audio = false
    formState.transType = "TCP"
    id.value = 0
    liveType.value='pull'
}
const labelCol = {
    style: {
        width: '80px',
    },
};
defineExpose({
    setOpen,
    setClose
});
</script>
<template>
    <a-modal v-model:open="open" :title="isAdd ? '添加' : '编辑'">
        <a-form :model="formState" :label-col="labelCol" @finish="onFinish" @finishFailed="onFinishFailed">
            <a-form-item label="启用">
                <a-switch v-model:checked="formState.enable" />
            </a-form-item>
            <a-form-item label="类型">
                <a-radio-group v-model:value="liveType"> 
                    <template v-if="id==0">
                        <a-radio-button value="pull" >拉流</a-radio-button>
                        <a-radio-button value="push" >推流</a-radio-button>
                    </template>
                    <template v-else>
                        <a-radio-button value="pull" v-if="liveType=='pull'">拉流</a-radio-button>
                        <a-radio-button value="push" v-if="liveType=='push'">推流</a-radio-button>
                    </template>
                </a-radio-group>
            </a-form-item>
            <a-form-item label="名称" name="name" :rules="[{ required: true, message: '名称不能为空!' }]">
                <a-input v-model:value="formState.name" placeholder="请输入名称" />
            </a-form-item>
            <template v-if="liveType == 'push'">
                <a-form-item label="推流鉴权">
                    <a-switch v-model:checked="formState.authed" />
                </a-form-item>
                <a-form-item label="推流地址" v-if="formState.url!=''">
                    <a-input v-model:value="formState.url" disabled>
                        <template #addonBefore>
                            <a-popover placement="top">
                                <template #content>
                                    <a-tag :bordered="false" color="warning" >注: 启用鉴权后更新鉴权，推流会断开重新校验!</a-tag>
                                </template>
                                <template #title></template>
                                <ReloadOutlined class="cp" @click="onSign()" />
                            </a-popover>
                        </template>
                        <template #addonAfter>
                            <CopyOutlined class="cp" @click="copyText(formState.url)" />
                        </template>
                    </a-input>
                </a-form-item>
            </template>
            <template v-if="liveType == 'pull'">
                <a-form-item label="地址">
                    <a-input v-model:value="formState.url" placeholder="请输入地址" />
                </a-form-item>
                <a-form-item label="协议">
                    <a-radio-group v-model:value="formState.transType">
                        <a-radio-button value="TCP">TCP</a-radio-button>
                        <a-radio-button value="UDP">UDP</a-radio-button>
                        <a-radio-button value="Multicast">其他</a-radio-button>
                    </a-radio-group>
                </a-form-item>
                <a-form-item label="其他">
                    <a-checkbox v-model:checked="formState.onDemand">按需</a-checkbox>
                    <a-checkbox v-model:checked="formState.audio">音频</a-checkbox>
                </a-form-item>
            </template>
          
            <a-flex justify="flex-end">
                <a-button @click="open = false">取消</a-button>
                <a-button type="primary" html-type="submit" class="ml16px">{{ isAdd ? '提交' : '修改' }}</a-button>
            </a-flex>
        </a-form>
        <template #footer></template>
    </a-modal>

</template>