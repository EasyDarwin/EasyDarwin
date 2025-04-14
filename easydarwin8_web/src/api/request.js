import axios from 'axios'
import qs from 'qs'
import { getTokenStorage } from '@/utils/storage'
import { notification } from 'ant-design-vue'
const request = axios.create({
  baseURL: '/api/v1/',
})
// 请求拦截器
request.interceptors.request.use(
  config => {
    // 添加请求参数序列化
    config.paramsSerializer = {
      serialize: params => qs.stringify(params, { arrayFormat: 'repeat' }),
    }
    config.headers['authorization'] = `Bearer ${getTokenStorage()}`;
    return config
  },
  error => {
    return Promise.reject(error)
  },
)

// 响应拦截器
request.interceptors.response.use(
  response => {
    return response
  },
  error => {
    if (error.response.status == 401) {
      location.href = "/#/login"
      let msg = "接口鉴权失败 401!"
      if (error.response.data && error.response.data.msg) {
        msg = error.response.data.msg
      }
      notification.error({
        description: msg
      });
      return Promise.reject(error)
    } else {
      let msg = "api error"
      if (error.response.data && error.response.data.msg) {
        msg = error.response.data.msg
      }
      notification.error({
        description: msg
      });
      return Promise.reject(error)
    }
  },
)
export default request;