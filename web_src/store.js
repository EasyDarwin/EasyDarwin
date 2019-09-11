import Vue from "vue";
import Vuex from "vuex";

Vue.use(Vuex);

const store = new Vuex.Store({
    state: {
        logoText: process.env.SYS_TITLE,
        logoMiniText: "ED",
        serverInfo: {},
        userInfo: null,
        menus: [
            {
                path: '/',
                title: "首页",
                icon: 'dashboard'
            }, {
                path: '/pushers/1',
                title: "推流列表",
                icon: "video-camera"
            }, {
                path: '/players/1',
                title: "拉流列表",
                icon: "play"
            }, {
                path: "/about",
                icon: "support",
                title: "版本信息"                
            }, {
                path: "/apidoc",
                target: "blank",
                icon: "book",
                title: "接口文档"
            }
        ]
    },
    mutations: {
        updateServerInfo(state, serverInfo) {
            state.serverInfo = serverInfo;
        },
        updateUserInfo(state, userInfo) {
            state.userInfo = userInfo;
        }
    },
    actions : {
        getServerInfo({commit}){
            return new Promise((resolve, reject) => {
                $.get('/api/v1/serverinfo').then(serverInfo => {
                    commit('updateServerInfo', serverInfo);
                    resolve(serverInfo);
                }).fail(() => {
                    resolve(null);
                });
            })
        },
        getUserInfo({ commit, state }) {
            return new Promise((resolve, reject) => {
                $.get("/api/v1/userinfo").then(userInfo => {
                    commit('updateUserInfo', userInfo);
                    resolve(userInfo);
                }).fail(() => {
                    resolve(null);
                })
            })
        },   
        logout({ commit, state }) {
            return new Promise((resolve, reject) => {
                $.get('/api/v1/logout').then(data => {
                    commit('updateUserInfo', null);
                    resolve(null);
                }).fail(() => {
                    resolve(null);
                })
            })
        }
    }
})

export default store;