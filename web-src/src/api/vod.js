import request from './request'

export default {
  // 获取点播列表
  getVodList(data) {
    return request({
      url: '/vod/list',
      method: 'get',
      params: data,
    })
  },

  // 获取单个点播信息
  getVodItemInfo(id) {
    return request({
      url: `/vod/get`,
      method: 'get',
      params: { id },
    })
  },

  // 删除点播
  deleteVod(id) {
    return request({
      url: `/vod/remove`,
      method: 'post',
      data: { id },
    })
  },

  // 上传点播文件
  uploadVod(data, onUploadProgress) {
    return request({
      url: `/vod/upload`,
      method: 'post',
      onUploadProgress: progressEvent => {
        const progresss = Math.round(
          (progressEvent.loaded / progressEvent.total) * 100,
        )
        console.log('>>>progress', progresss, progressEvent)
        onUploadProgress(progresss)
      },
      data,
    })
  },

  // 下载点播文件
  downloadVod(id) {
    return request({
      url: `/vod/download/${id}`,
      method: 'get',
      responseType: 'blob',
    })
  },

  // 获取转码进度
  getVodProgress(id) {
    return request({
      url: `/vod/progress`,
      method: 'get',
      params: { id },
    })
  },

  // 获取服务器支持上传文件类型
  getVodUploadAccept() {
    return request({
      url: `/vod/accept`,
      method: 'get',
    })
  },

  // 重新转码
  vodRetran(id) {
    return request({
      url: `/vod/retran`,
      method: 'post',
      data: { id },
    })
  },

  // 编辑点播文件
  vodEdit(data) {
    return request({
      url: `/vod/save`,
      method: 'post',
      data,
    })
  },

  // 上传封面
  uploadVodSnap(data, onUploadProgress) {
    return request({
      url: `/vod/snap`,
      method: 'post',
      headers: {
        'content-type': 'application/x-www-form-urlencoded; charset=UTF-8',
      },
      onUploadProgress: progressEvent => {
        const progresss = Math.round(
          (progressEvent.loaded / progressEvent.total) * 100,
        )
        onUploadProgress(progresss)
      },
      data,
    })
  },
}
