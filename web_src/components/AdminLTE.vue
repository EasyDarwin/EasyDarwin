<template>
  <div class="wrapper">
    <NaviBar :logoText="logoText" :logoMiniText="logoMiniText" :versionText="versionText" @modify-password="$refs['modify-password-dlg'].show()"></NaviBar>
    <Sider :menus="menus"></Sider>
    <ModifyPasswordDlg ref="modify-password-dlg"></ModifyPasswordDlg>
    <div class="content-wrapper">
      <section class="content">
        <router-view></router-view>
      </section>
    </div>
    <footer class="main-footer">
      <div class="pull-right hidden-xs hide">
        {{env.SYS_TITLE}}
      </div>
      <strong>Copyright &copy; {{ thisYear() }} <a :href="env.COMP_URL">{{env.COMP_INFO}}</a>.</strong> All rights reserved.
    </footer>
  </div>
</template>

<script>
import "font-awesome/css/font-awesome.css"
import "bootstrap/dist/css/bootstrap.css"
import "admin-lte/dist/css/AdminLTE.css"
import "admin-lte/dist/css/skins/_all-skins.css"
import "assets/styles/custom.less"

import "bootstrap/dist/js/bootstrap.js"
import "admin-lte/dist/js/adminlte.js"

import { mapState, mapActions } from "vuex"
import Vue from 'vue'
import moment from 'moment'
import Sider from 'components/Sider.vue'
import NaviBar from 'components/NaviBar.vue'
import ModifyPasswordDlg from 'components/ModifyPasswordDlg.vue'

import ElementUI from "element-ui"
import 'assets/styles/element-custom.scss'
Vue.use(ElementUI);

import VCharts from 'v-charts'
Vue.use(VCharts);

import VueClipboards from 'vue-clipboards';
Vue.use(VueClipboards);

import fullscreen from 'vue-fullscreen'
Vue.use(fullscreen);

import BackToTop from 'vue-backtotop'
Vue.use(BackToTop)

import VeeValidate, { Validator } from "vee-validate";
import zh_CN from "vee-validate/dist/locale/zh_CN";
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

import $ from 'jquery'
$.ajaxSetup({ cache: false });
export default {
  data() {
    return {
    }
  },
  mounted() {
    $(document).ajaxError((evt, xhr, opts, ex) => {
      if (xhr.status == 401) {
        top.location.href = '/';
        return false;
      }
      let msg = xhr.responseText || "网络请求失败";
      if (xhr.status == 404) {
        msg = "请求服务不存在或已停止";
      }
      this.$message({
        type: 'error',
        message: msg
      })
    }).on("click", ".main-header .sidebar-toggle", function () {
      setTimeout(() => {
        localStorage["sidebar-collapse"] = $("body").hasClass("sidebar-collapse") ? "sidebar-collapse" : "";
      }, 500)
    }).ready(() => {
      $("body").layout("fix");
      this.fixHover();
    });
    $("body").addClass(localStorage["sidebar-collapse"]);
    this.getServerInfo();
  },
  components: {
    NaviBar, Sider, ModifyPasswordDlg
  },
  computed: {
    ...mapState([
      "logoText",
      "logoMiniText",
      "menus",
      "serverInfo"
    ]),
    versionText(){
      let text = "";
      if(this.serverInfo){
        text = this.serverInfo.Version || "";
      }
      return text.substring(text.indexOf(" ") + 1);
    }
  },
  methods: {
    ...mapActions([
      "getServerInfo"
    ]),
    fixHover() {
        if(videojs.browser.IS_IOS||videojs.browser.IS_ANDROID) {
            for(var sheetI = document.styleSheets.length - 1; sheetI >= 0; sheetI--) {
                var sheet = document.styleSheets[sheetI];
                if(sheet.cssRules) {
                    for(var ruleI = sheet.cssRules.length - 1; ruleI >= 0; ruleI--) {
                        var rule = sheet.cssRules[ruleI];
                        if(rule.selectorText) {
                            rule.selectorText = rule.selectorText.replace(":hover", ":active");
                            rule.selectorText = rule.selectorText.replace(":focus", ":active");
                        }
                    }
                }
            }
        }
    },
    thisYear() {
      return moment().format('YYYY');
    }
  }
}
</script>

<style lang="less" scoped>
.content-wrapper, .right-side, .main-footer {
  transition: none;
}
</style>

<style lang="less">
.vue-back-to-top {
  background-color: transparent;
  left: 30px;
  bottom: 10px;
}
.sidebar-collapse .vue-back-to-top {
  display: none;
}
</style>