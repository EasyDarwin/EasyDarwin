<template>
  <aside id="sider" class="main-sidebar">
    <section class="sidebar">
      <ul class="sidebar-menu">
        <template v-for="(item,index) in menus">
          <router-link class="treeview" v-if="item.path && !item.target" :key="index" :to="item.path" tag="li" :exact="item.path == '/'">
            <a>
              <i :class="['fa', 'fa-' + item.icon]"></i>
              <span>{{item.title}}</span>
            </a>
          </router-link>
          <li class="treeview" v-if="item.path && item.target" :key="index">
            <a :href="item.path" :target="item.target">
              <i :class="['fa', 'fa-' + item.icon]"></i>
              <span>{{item.title}}</span>
            </a>
          </li> 
        </template>
      </ul>      
    </section>
  </aside>
</template>

<script>
import { mapState } from "vuex";

export default {
  props: {
    menus : {
        default : () => []
    }
  },
  computed: {
    ...mapState(['userInfo']),
    path(){
      return location.pathname;
    }
  },
  methods: {
    checkRoles(roles) {
      return !roles || roles.some((val, idx, array) => {
        if(!this.userInfo) return false;
        var _roles = this.userInfo.roles || [];
        return _roles.indexOf(val) >= 0;
      })
    }
  }
}

</script>

<style lang="css" scoped="true">
  .main-sidebar{
  /* Fix for IE */
  -webkit-transition: none;
  -o-transition: none;
  transition: none;
}
</style>




