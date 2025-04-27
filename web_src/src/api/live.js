import request from "./request";
export default {
  getLiveList(data){
    return request({
        url: '/live',
        method: 'get',
        params: data
    });
  },
  getLiveInfo(id){
    return request({
        url: `/live/info/${id}`,
        method: 'get',
    });
  },
  postPlayStart(id){
    return request({
        url: `/live/play/start/${id}`,
        method: 'get'
    });
  },

  postPlayStop(id){
    return request({
        url: `/live/play/stop/${id}`,
        method: 'get'
    });
  },
  getStreamInfo(id){
    return request({
        url: `/live/stream/info/${id}`,
        method: 'get',
    });
  },
  postLive(t,data){
    return request({
        url: `/live/${t}`,
        method: 'post',
        data
    });
  },
  putLive(t,id,data){
    return request({
        url: `/live/${t}/${id}`,
        method: 'put',
        data
    });
  },
  putLiveOne(t,id,type,value){
    return request({
        url: `/live/${t}/${id}/${type}/${value}`,
        method: 'put'
    });
  },
  deleteLive(id){
    return request({
        url: `/live/${id}`,
        method: 'delete'
    });
  },
 
}