import request from "./request";
export default {
  // 获取登录验证
  captcha(data){
    return request({
        url: '/captcha',
        method: 'post',
        data
    });
  },
  // 修改密码
  changePassword(username,data){
    return request({
        url: `/users/${username}/reset-password`,
        method: 'put',
        data
    });
  },
  // 用户登录
  userLogin(data){
    return request({
        url: '/login',
        method: 'post',
        data
    });
  },
  // 用户登出
  userLogout(data){
      return request({
          url: '/logout',
          method: 'post',
          data
      });
    }
}