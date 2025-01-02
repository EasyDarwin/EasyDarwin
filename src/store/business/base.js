import { defineStore } from 'pinia'
import { ref } from 'vue'
import { base } from "@/api";
import { getTokenStorage } from '@/utils/storage'
export const useBaseStore = defineStore(
  'base',
  () => {
    const serverInfo = ref({})

    const getServerInfo = async () => {
    const toUpperCaseStr = (str) => {
        return str[0].toUpperCase() + str.slice(1);
    }
    if (serverInfo.value&&serverInfo.value.version) return
      const res = await base.versionInfo()
      if (res.status == 200) {
        serverInfo.value = res.data
        serverInfo.value.server =toUpperCaseStr(serverInfo.value.server)
      }
    }
    return { getServerInfo, serverInfo }
  },
)
