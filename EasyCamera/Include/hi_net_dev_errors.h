#ifndef __HI_ERRORS_H__
#define __HI_ERRORS_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HI_NET_DEV_PARAM_ERROR                  0x41001
#define HI_NET_DEV_MEMORY_ERROR                 0x41002
#define HI_NET_DEV_NOT_SUPPORT                  0x41003
#define HI_NET_DEV_HANDLE_NOT_EXIST          0x41004
#define HI_NET_DEV_PARAM_CHECK_ERROR		0x51001		//Paramter input error
#define HI_NET_DEV_PARAM_CMD_ERROR		0x51002		//No command
#define HI_NET_DEV_PARAM_PARSE_ERROR		0x51003		//parsse command 
#define HI_NET_DEV_NET_CONNECT_FAIL		0x52001		//connect host failure
#define HI_NET_DEV_NET_TRANSFER_FAIL		0x52002		//transfer host failure
#define HI_NET_DEV_NET_RETURN_ERROR		0x52003		//host return error
#define HI_NET_DEV_NET_NOT_SUPPORT		0x53000		//device not support the paramter
#define HI_NET_DEV_VOICE_HAS_RUN                0x54001        //对讲已经运行
#define HI_NET_DEV_VOICE_NOT_RUN                0x54002        //对讲没有运行
#define HI_NET_DEV_RECORD_HAS_RUN                0x54003
#define HI_NET_DEV_RECORD_NOT_RUN                0x54004

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

