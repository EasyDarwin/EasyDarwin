import Vue from 'vue'
import store from "./store";
import router from './router'

Vue.prototype.env = process.env

new Vue({
  el: '#app',
  store,
  router,
  template: `
  <transition>
    <router-view></router-view>
  </transition>
  `
})