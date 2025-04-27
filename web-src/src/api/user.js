import request from "./request";
export default {
  // Get Login Authentication
  // captcha(data){
  //   return request({
  //       url: '/captcha',
  //       method: 'post',
  //       data
  //   });
  // },
  // change your password
  changePassword(username,data){
    return request({
        url: `/users/${username}/reset-password`,
        method: 'put',
        data
    });
  },
  // user login
  userLogin(data){
    return request({
        url: '/login',
        method: 'post',
        data
    });
  },
  // user logout
  userLogout(data){
      return request({
          url: '/logout',
          method: 'post',
          data
      });
    }
}
