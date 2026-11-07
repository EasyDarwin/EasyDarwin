import axios from 'axios'
import qs from 'qs'
import { getTokenStorage } from '@/utils/storage'
import { notification } from 'ant-design-vue'
const request = axios.create({
  baseURL: '/api/v1/',
})
// request interceptor
request.interceptors.request.use(
  config => {
    // Add request parameter serialization
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

// Response Interceptor
request.interceptors.response.use(
  response => {
    return response
  },
  error => {
    if (error.response.status == 401) {
      location.href = "/#/login"
      let msg = "Interface Authentication Failure 401!"
      if (error.response.data && error.response.data.msg) {
        msg = error.response.data.msg
      }
      notification.error({
        description: msg
      });
      return Promise.reject(error)
    } else {
      // 优先展示后端返回的具体错误信息（details 优先于通用 msg），避免提示 [object Object]
      let msg = "api error"
      const data = error.response.data
      if (data) {
        if (Array.isArray(data.details) && data.details.length > 0) {
          msg = data.details.join("；")
        } else if (data.msg) {
          msg = data.msg
        } else if (typeof data === 'string') {
          msg = data
        }
      }
      notification.error({
        description: msg
      });
      return Promise.reject(error)
    }
  },
)
export default request;
