import { defineStore } from 'pinia'
import { ref,computed } from 'vue'
import { user } from "@/api";
import { useRouter } from 'vue-router'
import { setTokenStorage,removeTokenStorage } from '@/utils/storage'
import CryptoJS from 'crypto-js'
export const useUserStore = defineStore(
  'user',
  () => {
    const userInfo = ref({})
    const setUserInfo = data => {
      userInfo.value = data
    }

    const router = useRouter()
   
    const login = async data => {
      return new Promise((resolve, reject) => {
        user.userLogin({
          ...data,
          password: CryptoJS.SHA256(data.password).toString(),
        }).then(res=>{
          if (res.status == 200) {
            router.push('/')
            setUserInfo(res.data.user)
            setTokenStorage(res.data.token)
          } else {
            reject()
          }
        }).catch(err=>{
          reject(err)
        })
      });
    }
    const logout = async () => {
      const res = await user.userLogout()
      if (res.status == 200) {
        router.push('/login')
        userInfo.value = {}
        removeTokenStorage()
      }
    }
    const changePassword = data => {
      return new Promise((resolve, reject) => {
        user.changePassword(getUsername.value,{
          password: CryptoJS.SHA256(data.password).toString(),
          new_password: CryptoJS.SHA256(data.new_password).toString()
        }).then(res=>{
          if (res.status == 200) {
            resolve()
          } else {
            reject()
          }
        }).catch(err=>{
          reject()
        })
      });
    }
    const getUsername = computed(() => {
      if (userInfo.value&&userInfo.value.username) {
        return userInfo.value.username
      } else{
        return "empty"
      }
    })
    return { userInfo, setUserInfo, login, logout, changePassword,getUsername }
  },
  { persist: true },
)
