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
  postConfigStream(data,is_reload){
    return request({
        url: `/configs/stream?is_reload=${is_reload}`,
        method: 'post',
        data
    });
  },
  postConfigData(data,is_reload){
    return request({
        url: `/configs/data?is_reload=${is_reload}`,
        method: 'post',
        data
    });
  },
  postConfigHls(data,is_reload){
    return request({
        url: `/configs/hls?is_reload=${is_reload}`,
        method: 'post',
        data
    });
  },
  postConfigRtc(data,is_reload){
    return request({
        url: `/configs/rtc?is_reload=${is_reload}`,
        method: 'post',
        data
    });
  },
  postConfigRtmp(data,is_reload){
    return request({
        url: `/configs/rtmp?is_reload=${is_reload}`,
        method: 'post',
        data
    });
  },
  postConfigRtsp(data,is_reload){
    return request({
        url: `/configs/rtsp?is_reload=${is_reload}`,
        method: 'post',
        data
    });
  },
  postConfigRecord(data,is_reload){
    return request({
        url: `/configs/record?is_reload=${is_reload}`,
        method: 'post',
        data
    });
  },
  postConfigBaseLog(data,is_reload){
    return request({
        url: `/configs/baselog?is_reload=${is_reload}`,
        method: 'post',
        data
    });
  },
  postConfigStreamLog(data,is_reload){
    return request({
        url: `/configs/streamlog?is_reload=${is_reload}`,
        method: 'post',
        data
    });
  },
}