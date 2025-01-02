import request from "./request";
export default {
  versionInfo(data){
    return request({
        url: '/version',
        method: 'get',
        params: data
    });
  },
  setReboot(){
    return request({
        url: '/app/reboot',
        method: 'post',
    });
  },
  getConfigBase(){
    return request({
        url: '/configs/base',
        method: 'get',
    });
  },
  postConfigBase(data,is_reload){
    return request({
        url: `/configs/base?is_reload=${is_reload}`,
        method: 'post',
        data
    });
  },
}