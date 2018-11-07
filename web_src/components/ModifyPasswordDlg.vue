<template>
    <FormDlg title="修改密码" @hide="onHide" @show="onShow" @submit="onSubmit" ref="dlg" :disabled="errors.any()">
        <div :class="{'form-group':true, 'has-feedback':true,'has-error': errors.has('oldPassword')}">
            <label for="old-password" class="col-sm-4 control-label">原密码
                <span class="text-red">*</span>
            </label>
            <div class="col-sm-7">
                <input type="password" class="form-control" id="old-password" name="oldPassword" v-model.trim="form.oldPassword" autocomplete="new-password" data-vv-as="原密码" v-validate="'required'" @keydown.enter="$el.querySelector('#new-password').focus()">
            </div>
        </div>
        <div :class="{'form-group':true, 'has-feedback':true,'has-error': errors.has('newPassword')}">
            <label for="new-password" class="col-sm-4 control-label">新密码
                <span class="text-red">*</span>
            </label>
            <div class="col-sm-7">
                <input type="password" class="form-control" id="new-password" name="newPassword" v-model.trim="form.newPassword" autocomplete="new-password" data-vv-as="新密码" v-validate="'required'" @keydown.enter="$el.querySelector('#new-password2').focus()">
            </div>
        </div>
        <div :class="{'form-group':true, 'has-feedback':true,'has-error': errors.has('newPassword2')}">
            <label for="new-password2" class="col-sm-4 control-label">确认密码
                <span class="text-red">*</span>
            </label>
            <div class="col-sm-7">
                <input type="password" class="form-control" id="new-password2" name="newPassword2" v-model.trim="form.newPassword2" autocomplete="new-password" data-vv-as="确认密码" v-validate="'required|confirmed:newPassword'" @keydown.enter="onSubmit">
            </div>
        </div>
    </FormDlg>
</template>

<script>
import FormDlg from 'components/FormDlg.vue'
import $ from 'jquery'
import crypto from 'crypto'

export default {
    data() {
        return {
            form: this.defForm()
        }
    },
    components: {
        FormDlg
    },
    methods: {
        defForm() {
            return {
                oldPassword: '',
                newPassword: '',
                newPassword2: ''
            }
        },
        onHide() {
            this.form = this.defForm();
        },
        onShow() {
            this.errors.clear();
            // this.$el.querySelector('#old-password').focus();
        },
        async onSubmit() {
            var ok = await this.$validator.validateAll();
            if(!ok) {
                var e = this.errors.items[0];
                this.$message({
                    type: 'error',
                    message: e.msg
                })
                $(`[name=${e.field}]`).focus();
                return;
            }
            $.get('/user/modify-password', {
                oldPassword: this.md5(this.form.oldPassword),
                newPassword: this.md5(this.form.newPassword),
                newPassword2: this.md5(this.form.newPassword2)
            }).then(data => {
                this.$refs['dlg'].hide();
                this.$alert("密码修改成功,即将重新登录!", "提示").then(() => {
                    window.location.href = '/logout';
                }).catch(() => {
                    window.location.href = '/logout';
                })                
            })
        },
        show(data) {
            this.errors.clear();
            if(data) {
                Object.assign(this.form, data);
            }
            this.$refs['dlg'].show();
        },
        md5(text) {
            return crypto.createHash('md5').update(text, "utf8").digest('hex');
        }        
    }
}
</script>
