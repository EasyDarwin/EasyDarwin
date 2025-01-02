<script setup>
import { watch, onBeforeUnmount,onMounted, ref } from 'vue'
import { PictureFilled } from '@ant-design/icons-vue'
const emit = defineEmits([''])
const isError = ref(false)
const props = defineProps({
    imgUrl: {
        type: String,
        default: "",
    },
})
const snapError = () => {
    isError.value = true
}

watch(() => props.videoUrl, (newValue) => {
 
}, { deep: true })
onMounted(() => {
    
})

onBeforeUnmount(() => {
})
</script>
<template>
   <a-flex class="img-box cp" justify="center" align="center" >
    <a-flex v-if="isError || imgUrl==''"  justify="center" align="center">
        <PictureFilled />
    </a-flex>
    <img :src="imgUrl" alt="png" @error="snapError($event)" v-else>
   </a-flex>
</template>

<style scoped lang="less">
    .img-box {
        width: 100%;
        height: 100%;
        .anticon{
            font-size: 28px;
        }
        img {
            max-height: 100%;
            max-width: 100%;
        }
    }
</style>