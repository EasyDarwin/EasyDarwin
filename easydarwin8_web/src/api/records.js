import request from "./request";
export default {
  getRecordsCloudList(data){
    return request({
        url: '/record/channels',
        method: 'get',
        params: data
    });
  },
  getRecordsCloudInfo(id,data){
    return request({
        url: `/record/channels/${id}`,
        method: 'get',
        params: data
    });
  },
  delRecordsChannels(id,data){
    return request({
        url: `/record/channels/${id}`,
        method: 'delete',
        data
    });
  },
  getRecordsCloudMonth(id,data){
    return request({
        url: `/record/month/${id}`,
        method: 'get',
        params: data
    });
  },
  // 录像计划
  getRecordsPlansList(data){
    return request({
        url: '/records/plans',
        method: 'get',
        params: data
    });
  },
  postRecordsPlansList(data){
    return request({
        url: '/records/plans',
        method: 'post',
        data
    });
  },
  postRecordsPlansChannels(id,data){
    return request({
        url: `/records/plans/${id}/channels`,
        method: 'post',
        data
    });
  },
  delRecordsPlansChannels(id,data){
    return request({
        url: `/records/plans/${id}/channels`,
        method: 'delete',
        data
    });
  }
}