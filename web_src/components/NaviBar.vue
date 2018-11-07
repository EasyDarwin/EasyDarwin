<template>
  <header class="main-header">
    <router-link class="logo" style="position: relative;" to="/">
        <div class="logo-mini">{{logoMiniText}}</div>
        <div class="logo-lg">{{logoText}}</div>
        
        <span class='logo-lg label label-warning' style='position: absolute;top:8px;right: 20px;font-size: 8px;' v-if="versionText">{{versionText}}</span>
    </router-link>    
  
    <nav class="navbar navbar-static-top">
      <a class="sidebar-toggle" data-toggle="push-menu" role="button">
        <span class="sr-only">Toggle navigation</span>
      </a>

      <div class="navbar-custom-menu">
          <ul class="nav navbar-nav">
              <li class="dropdown" v-if="userInfo">
                <a href="#" class="dropdown-toggle" data-toggle="dropdown">欢迎, {{userInfo.name}}</a>
                <ul class="dropdown-menu">
                  <li>
                      <a @click.prevent="$emit('modify-password')" role="button"><i class="fa fa-key"></i> 修改密码</a>
                  </li>
                  <router-link to="/logout" tag="li">
                      <a><i class="fa fa-sign-out"></i> 注 销</a>
                  </router-link>
                </ul>
              </li>
              <li v-else>
                <a href="/login.html"><i class="fa fa-user"></i> 登录</a>
              </li>
          </ul>
      </div>      
    </nav>
  </header>
</template>

<script>
import {mapState} from 'vuex'

export default {
  props: {
    logoText: {
      default: "AdminLte"
    },
    logoMiniText: {
      default: "AD"
    },
    versionText: {
      default: ""
    }
  },
  components: {
  },
  computed: {
    ...mapState(['userInfo'])
  }
}

</script>

<style lang="less" scoped="true">
.main-header .navbar,
.main-header .logo {
  /* Fix for IE */
  -webkit-transition: none;
  -o-transition: none;
  transition: none;
}

.main-header .logo {
  padding: 0;
}

.main-header .navbar .dropdown-menu li a {
  color: #777;
  &:hover {
    background-color: #e1e3e9;
  }
}
</style>

