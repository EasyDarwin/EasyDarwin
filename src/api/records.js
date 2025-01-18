import request from "./request";
export default {
  getRecordsPlansList(data){
    return request({
        url: '/records/plans',
        method: 'get',
        params: data
    });
  }
}