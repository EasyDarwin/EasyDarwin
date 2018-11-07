<template>
  <div class="login-box">
    <div class="login-logo text-success">
        <b>EasyDarwin 登录</b>
    </div>
    <div class="login-box-body">
        <br>
        <form autocomplete="off">
            <div :class="{'form-group':true, 'has-feedback':true,'has-error': errors.has('username')}">
                <input type="text" class="form-control" :placeholder="usernamePlaceholder" v-validate="'required'" data-vv-as="用户名" name="username" v-model.trim="username" @keydown.enter="$el.querySelector('[name=password]').focus()">
                <span class="glyphicon glyphicon-user form-control-feedback"></span>
            </div>
            <div :class="{'form-group':true, 'has-feedback':true,'has-error': errors.has('password')}">
                <input type="password" class="form-control" :placeholder="passwordPlaceholder" autocomplete="new-password" v-validate="'required'" data-vv-as="密码" name="password" v-model.trim="password" @keydown.enter="doLogin">
                <span class="glyphicon glyphicon-lock form-control-feedback"></span>
            </div>
            <br>
            <div class="form-group">
                <button type="button" class="btn btn-success btn-block btn-flat" @click.prevent="doLogin" :disabled="isLoading">登&nbsp;&nbsp;录</button>
            </div>
        </form>

    </div>
    <!-- /.login-box-body -->
    </div>
<!-- /.login-box -->
</template>

<script>
import "bootstrap/dist/css/bootstrap.css"
import "admin-lte/dist/css/AdminLTE.css"
import "admin-lte/dist/css/skins/_all-skins.css"
import "assets/styles/custom.less"

import "bootstrap/dist/js/bootstrap.js"
import "admin-lte/dist/js/adminlte.js"

import Vue from 'vue'
import VeeValidate, { Validator } from "vee-validate";
import zh_CN from "vee-validate/dist/locale/zh_CN";
import crypto from 'crypto'

Validator.localize("zh_CN", zh_CN);
Vue.use(VeeValidate, {
  locale: "zh_CN",
//   delay: 500,
  dictionary: {
    zh_CN: {
      messages: {
        required: field => `${field} 不能为空`,
        confirmed: (field, targetField) => `${field} 和 ${targetField} 不匹配`,
        regex: field => `${field} 不符合要求格式`
      }
    }
  }
});

import ElementUI from "element-ui"
import 'assets/styles/element-custom.scss'

Vue.use(ElementUI);

import $ from 'jquery'
$.ajaxSetup({ cache: false });
export default {
    data() {
        return {
            username: '',
            password: '',
            isLoading: false,
            defaultPassword: '',
            defaultUsername: ''
        }
    },
    mounted() {
        this.$el.querySelector('[name=username]').focus();
        this.getDefaultLoginInfo();
    },
    computed: {
        passwordPlaceholder() {
            if(this.defaultPassword) {
                return `密码(${this.defaultPassword})`
            }
            return "密码"
        },
        usernamePlaceholder() {
            if(this.defaultUsername) {
                return `用户名(${this.defaultUsername})`
            }
            return "用户名";
        }
    },
    methods: {
        md5(text) {
            return crypto.createHash('md5').update(text, "utf8").digest('hex');
        },
        getDefaultLoginInfo() {
            $.get("/api/v1/defaultlogininfo").then(data => {
                this.defaultPassword = data.password;
                this.defaultUsername = data.username;
            })
        },
        async doLogin() {
            var ok = await this.$validator.validateAll();
            if(!ok){
                var e = this.errors.items[0];
                this.$message({
                    type: 'error',
                    message: e.msg
                });
                $(`[name=${e.field}]`).focus();
                return;
            }
            this.isLoading = true;
            $.get('/api/v1/login',{
                username: this.username,
                password: this.md5(this.password)
            }).then(data => {
                window.location.href = '/';
            }).always(() => {
                this.isLoading = false;
            })
        }
    }
};
</script>

