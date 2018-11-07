import Vue from 'vue'

import Login from 'components/Login.vue'

new Vue({
  el: '#app',
  template: `<Login></Login>`,
  components: {
    Login
  },
  mounted() {
    $(document).ajaxError((evt, xhr, opts, ex) => {
        if (xhr.status == 404) {
            xhr.responseText = "请求服务不存在或已停止";
        }
        var msg = xhr.responseText;
        try {
            msg = JSON.parse(msg);
        } catch (e) {
        }
        if (typeof msg != 'undefined' && msg) {
            this.$message({
                type: 'error',
                message: msg
            })
        }
    });
  }
})