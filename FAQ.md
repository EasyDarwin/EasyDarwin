# EasyDarwin常见问题 #

## 1. EasyDarwin收费吗？ ##

不要钱

## 2. EasyDarwin返回401 Unauthorized ##

在easydarwin.xml中配置的Movies目录里面放一个名为**qtaccess**的文件，内容为：

	<Limit WRITE>  
	require any-user  
	</Limit>
	<Limit READ>
	require any-user
	</Limit>

## 3. RTSP推送到EasyDarwin出现404错误 ##
	
方法一：用EasyPusher做直播推送；

方法二：修改RTSP ANNOUNCE里面的sdp信息，把 "o=- %u %u IN IP4 %s/r/n" 和 "c=IN IP4 %s/r/n"里面的ip地址改成127.0.0.1就可以了;

方法三：我们将EasyDarwin部署到公网，当服务器置身内网，用端口映射的方式对外提供服务，在接收RTSP/RTP推送的时候，经常会出现在SETUP步骤Darwin返回404错误，经过查找原因，主要是EasyDarwin对推送的sdp信息中的IP地址不能识别，服务器并不知道自己已经置身于公网的地址：

**ifconfig查看地址信息**

![EasyDarwin](http://www.easydarwin.org/d/file/article/doc/EasyDarwin/9d86ccd17b3d38b84c27ed1008a9bf38.jpg)

**ip addr查看eth0地址列表**

![EasyDarwin](http://www.easydarwin.org/d/file/article/doc/EasyDarwin/45d612cba5ddc67617e734f532df78c7.jpg)

我们通过命令：ip addr add dev eth0 [公网IP]，向eth0添加一个公网地址就解决问题了：

![](http://www.easydarwin.org/d/file/article/doc/EasyDarwin/267035e801fadf06ea5f6acb5486a988.jpg)

Windows添加公网地址的方法：

![EasyDarwin](http://www.easydarwin.org/d/file/article/doc/EasyDarwin/8604f16ba3eb39ba2dd4e32a63f1fcf1.jpg)

![EasyDarwin](http://www.easydarwin.org/d/file/article/doc/EasyDarwin/8604f16ba3eb39ba2dd4e32a63f1fcf1.jpg)

## 4. EasyDarwin做转发延时10几秒? ##
	
在EasyDarwin QTSSReflectorModule转发模块中，有一个控制转发Buffer时间的配置**reflector_buffer_size_sec**，我们将这个配置改成0，也就是在服务器端不做缓存，直接转发，这样在网络条件充足的情况下对比转发和实时流，转发带来的延时也几乎可以忽略了：


## 5. 看不到直播或者点播视频，如何排查? ##

我们可以先配置easydarwin.xml文件中的

	<PREF NAME="RTSP_debug_printfs" TYPE="Bool16" >true</PREF>

字段为true，然后重新启动EasyDarwin，请求EasyDarwin，如果一点报文都没有打印，那就是你访问的地址错了！如果报文有打印，那就可以具体看看返回的错误码是多少了，错误码对照表：
	
<table>
	<tbody>
		<tr><td><em>响应码</em></td><td><em>报文描述</em></td><td><em>定义</em></td></tr>
		<tr><td>200</td><td>Success OK</td><td>成功创建</td></tr>
		<tr><td>201</td><td>Success Created</td><td>成功创建</td></tr>
		<tr><td>202</td><td>Success Accepted	</td><td>已接受用于处理，但处理尚未完成</td></tr>
		<tr><td>204</td><td>Success No Content	</td><td>已接收请求，但不存在要回送的信息</td></tr>
		<tr><td>206</td><td>Success Partial Content	</td><td>已接收请求，但要回送的信息不完整</td></tr>
		<tr><td>301</td><td>Redirect Permanent Moved	</td><td>请求的数据具有新的位置且更改是永久的。</td></tr>
		<tr><td>302</td><td>Redirect Temp Moved	</td><td>请求的数据临时具有不同 URI</td></tr>
		<tr><td>303</td><td>Redirect See Other	</td><td>可在另一 URI 下找到对请求的响应</td></tr>
		<tr><td>305</td><td>Use Proxy	</td><td>必须通过位置字段中提供的代理来访问请求的资源</td></tr>
		<tr><td>400</td><td>Client Bad Request	</td><td>请求中有语法问题，或不能满足请求</td></tr>
		<tr><td>401</td><td>Client Unauthorized	</td><td>未授权客户端访问数据</td></tr>
		<tr><td>402</td><td>Payment Required	</td><td>需要付款,表示计费系统已有效</td></tr>
		<tr><td>403</td><td>Client Forbidden	</td><td>禁止, 即使有授权也不需要访问</td></tr>
		<tr><td>404</td><td>Not Found	</td><td>服务器找不到给定的资源</td></tr>
		<tr><td>405</td><td>Method Not Allowed	</td><td>请求的方法不支持</td></tr>
		<tr><td>407</td><td>Proxy Authentication Required	</td><td>代理认证请求，客户机首先必须使用代理认证自身</td></tr>
		<tr><td>408</td><td>Request Timeout	</td><td>请求超时</td></tr>
		<tr><td>409</td><td>Conflict	</td><td>请求冲突</td></tr>
		<tr><td>412</td><td>Precondition Failed	</td><td>前提条件失败</td></tr>
		<tr><td>415</td><td>Unsupported Media Type	</td><td>服务器拒绝服务请求，因为不支持请求实体的格式</td></tr>
		<tr><td>500</td><td>Server Internal Error	</td><td>内部错误,因为意外情况，服务器不能完成请求</td></tr>
		<tr><td>501</td><td>Server Not Implemented	</td><td>未执行,服务器不支持请求</td></tr>
		<tr><td>502</td><td>Server Bad Gateway	</td><td>错误网关,服务器接收到来自上游服务器的无效响应</td></tr>
		<tr><td>503</td><td>Server Unavailable	</td><td>由于临时过载或无法获得服务护,服务器无法处理请求</td></tr>
		<tr><td>505</td><td>RTSP Version Not Supported	</td><td>不支持的RTSP版本</td></tr>

	</tbody>
</table>


## 6. mp4点播返回415错误(Requested movie hasn't been hinted) ##

MP4文件需要先经过RTSP/RTP Hint处理，处理工具可以选择MP4Box或者MediaCoder等；

## 7. /bin/sh^M: bad interpreter: No such file or directory ##

**感谢QQ:591003999提供的完整解决方案**

从git上导下来的Buildit的格式是：fileformat=dos，但是在Linux下的话 是需要fileformat=unix 所以 ./Buildit 就出现错误 无法执行，需要修改文件格式，

- 在Windows下转换： 

> 利用一些编辑器如UltraEdit或EditPlus等工具先将脚本编码转换，再放到Linux中执行。转换方式如下
> 
> **（UltraEdit）：File-->Conversions-->DOS->UNIX**
> 
> 即可。 

- 也可在Linux中转换： 

> 首先要确保文件有可执行权限 
> 
> 	#sh>chmod a+x filename 
> 
> 然后修改文件格式 
> 
> 	#sh>vi filename 
> 
> 利用如下命令查看文件格式 
> 
> 	:set ff 或 :set fileformat 
> 
> 可以看到如下信息 
> 
> 	fileformat=dos 或 fileformat=unix 
> 
> 利用如下命令修改文件格式
>  
> 	:set ff=unix 或 :set fileformat=unix 
> 
> :wq (存盘退出) 
> 
> 最后再执行文件 
> 	
> 	#sh>./filename
> 


# 获取更多信息 #

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：288214068

Copyright &copy; EasyDarwin.org 2012-2015

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)
