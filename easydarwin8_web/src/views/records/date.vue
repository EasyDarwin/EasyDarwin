<template>
    <a-space direction="vertical" :size="12">
      <a-date-picker v-model:value="dateValue" :disabled-date="disabledDate" @change="onDay" @panelChange="onPanelChange">
        <template #dateRender="{ current }">
          <div class="ant-picker-cell-inner" :style="getCurrentStyle(current)">
            {{ current.date() }}
          </div>
        </template>
      </a-date-picker>
      
    </a-space>
    <!-- <a-space direction="vertical" :size="12" style="margin-left: 16px;">
        <a-time-range-picker v-model:value="timeValue"  style="width: 200px;" @change="onTime"/>
    </a-space> -->
  </template>
<script setup>
    import { ref, watch } from 'vue';
    import dayjs from 'dayjs';
    const emit = defineEmits(['time','date','day'])
    const props = defineProps({
        year: {
            type: Number,
            default: 0,
        },
        month: {
            type: Number,
            default: 0,
        },
        day: {
            type: Number,
            default: 0,
        },
        dataDay: {
            type: Array,
            default: ()=>[],
        },
    })
    const onTime = (current) => {
        emit('time', dayjs(current[0]).format('HHmmss'),dayjs(current[1]).format('HHmmss'))
    }
    const onDay = (current) => {
        if ((current.$M+1)!=props.month) {
            emit('date', current.$y,current.$M+1)
        }
        emit('day', current.$y,current.$M+1, current.$D)
    }
    const onPanelChange = (current)=>{
   
        emit('date', current.$y,current.$M+1)
    }
    const disabledDate = (current) => {
        if (props.month==(current.$M+1)) {
            return !isDay(current.date())
        }
        return true
       
    };
    const getCurrentStyle = (current) => {
        const style= {};
        if (props.month==(current.$M+1)) {
            if (isDay(current.date())) {
                style.border = '1px solid #1890ff';
              
                style.borderRadius = '50%';
            }
        }
        return style;
    };

    const isDay = (v)=>{
        let index = props.dataDay.indexOf(v);
        if (index!=-1)return true
        return false
    }
    const dateFormat = 'YYYY-M-D';
    const timeValue = ref([dayjs('00:00:00', 'HH:mm'),dayjs('23:59:59', 'HH:mm')]);
    const dateValue = ref();
 
    const initDate = ()=>{
        dateValue.value = dayjs(`${props.year}-${props.month}-${props.day}`, dateFormat)
    }
    initDate()
    watch(() => props.dataDay,  (v) => {
    },{ deep: true })
</script>
  
  