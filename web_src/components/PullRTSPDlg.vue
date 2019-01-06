<template>
    <FormDlg title="编辑拉流" @hide="onHide" @show="onShow" @submit="onSubmit" ref="dlg" :disabled="errors.any() || bLoading">     
        <div :class="['form-group', { 'has-error': errors.has('url')}]">
            <label for="input-url" class="col-sm-3 control-label"><span class="text-red">*</span> RTSP地址</label>
            <div class="col-sm-8">
                <input type="text" id="input-url" class="form-control" name="url" data-vv-as="RTSP地址" v-validate="'required'" v-model.trim="form.url">
                <span class="help-block">{{errors.first('url')}}</span>
            </div>
        </div>                   
        <div :class="['form-group', { 'has-error': errors.has('customPath')}]">
            <label for="input-custom-path" class="col-sm-3 control-label">自定义路径</label>
            <div class="col-sm-8">
                <input type="text" id="input-custom-path" class="form-control" name="customPath" data-vv-as="自定义路径" v-model.trim="form.customPath" placeholder="/your/custom/path">
                <span class="help-block">{{errors.first('customPath')}}</span>
            </div>
        </div> 
        <div class="form-group">
            <label for="input-transport" class="col-sm-3 control-label">传输协议</label>
            <div class="col-sm-8">
                <el-radio-group id="input-transport" v-model.trim="form.transType" size="mini">
                    <el-radio-button label="TCP"></el-radio-button>
                    <el-radio-button label="UDP"></el-radio-button>
                    <!-- <el-radio-button label="Multicast"></el-radio-button> -->
                </el-radio-group>
            </div>
        </div>          
        <div :class="['form-group', { 'has-error': errors.has('idleTimeout')}]">
            <label for="input-idle-timeout" class="col-sm-3 control-label">空闲超时(秒)</label>
            <div class="col-sm-8">
                <input type="text" id="input-idle-timeout" class="form-control" name="idleTimeout" data-vv-as="空闲超时" v-validate="'numeric'" v-model.trim="form.idleTimeout" placeholder="默认使用系统配置">
                <span class="help-block">{{errors.first('idleTimeout')}}</span>
            </div>
        </div>   
        <div :class="['form-group', { 'has-error': errors.has('heartbeatInterval')}]">
            <label for="input-heartbeat-interval" class="col-sm-3 control-label">心跳间隔(秒)</label>
            <div class="col-sm-8">
                <input type="text" id="input-heartbeat-interval" class="form-control" name="heartbeatInterval" data-vv-as="心跳间隔" v-validate="'numeric'" v-model.trim="form.heartbeatInterval" placeholder="默认使用系统配置">
                <span class="help-block">{{errors.first('heartbeatInterval')}}</span>
            </div>
        </div>                         
    </FormDlg>
</template>

<script>
import Vue from 'vue'
import FormDlg from 'components/FormDlg.vue'
import $ from 'jquery'

export default {
    data() {
        return {
            bLoading: false,
            form: this.defForm()
        }
    },
    components: {
        FormDlg
    },
    methods: {
        defForm() {
            return {
                url: '',
                customPath: '',
                transType: 'TCP',
                idleTimeout: '',
                heartbeatInterval: ''
            }
        },
        onHide() {
            this.errors.clear();
            this.form = this.defForm();
        },
        onShow() {
            document.querySelector(`[name=url]`).focus();
        },
        async onSubmit() {
            var ok = await this.$validator.validateAll();
            if(!ok) {
                var e = this.errors.items[0];
                this.$message({
                    type: 'error',
                    message: e.msg
                });
                document.querySelector(`[name=${e.field}]`).focus();
                return;
            }
            this.bLoading = true;
            $.get('/api/v1/stream/start', this.form).then(data => {
                this.$refs['dlg'].hide();
                this.$emit('submit');
            }).always(() => {
                this.bLoading = false;
            })
        },
        show(data) {
            this.errors.clear();
            if(data) {
                Object.assign(this.form, data);
            }
            this.$refs['dlg'].show();
        }
    }
}
</script>
