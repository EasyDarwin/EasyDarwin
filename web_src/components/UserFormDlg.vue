<template>
    <FormDlg title="编辑用户" @hide="onHide" @submit="onSubmit" ref="dlg" :disabled="errors.any()">
        <div class="alert alert-success alert-dismissible" v-if="!form.id">
            <small>新增用户, 初始默认密码是 {{defaultPwd}}</small>
            <button type="button" class="close" data-dismiss="alert" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        </div>       
        <input type="hidden" v-model.trim="form.id" name="id">
        <div :class="['form-group', { 'has-error': errors.has('name')}]">
            <label for="input-name" class="col-sm-3 control-label"><span class="text-red">*</span> 用户名</label>
            <div class="col-sm-7">
                <input type="text" id="input-name" class="form-control" name="name" data-vv-as="用户名" v-validate="'required'" v-model.trim="form.name" @keydown.enter.prevent="onSubmit">
                <span class="help-block">{{errors.first('name')}}</span>
            </div>
        </div>                   
    </FormDlg>
</template>

<script>
import Vue from 'vue'
import FormDlg from 'components/FormDlg.vue'
import $ from 'jquery'

export default {
    props: {
        defaultPwd: {
            type: String,
            default: ''
        }
    },
    data() {
        return {
            roles: [],
            form: this.defForm()
        }
    },
    components: {
        FormDlg
    },
    methods: {
        defForm() {
            return {
                id: '',
                name: ''
            }
        },
        onHide() {
            this.errors.clear();
            this.form = this.defForm();
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
            $.get('/user/save', this.form).then(data => {
                this.$refs['dlg'].hide();
                this.$emit('submit');
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
