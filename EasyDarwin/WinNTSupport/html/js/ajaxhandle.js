function getruntime()
{
	$.ajax({
                url: '/api/getServiceRunTime',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
                data: { },
                success: function (json) {
					
					var strs=json.RunTime.split('|');
					var version=json.version;
					var str=str_run_time+": "+strs[0]+"D-"+strs[1]+"H-"+strs[2]+"M-"+strs[3]+"S"
					document.getElementById('dvruntime').innerHTML=str+"  ["+version+"]";
                },
				error:function(msg)
				{	alert(textStatus);}
            });
}
window.onload = function () {
	   getruntime();
	setInterval("getruntime()",1000);
}
function fLoad()
{
	
	getruntime();
	setInterval("getruntime()",1000);
}
//++++++++++++++++++++++++++++++++++++++++++++++
//端口设置
function setPort()
{
	
	if($('#txtport').val()==null||$('#txtport').val()=='')
	{
		alert(str_Mongoose_port_warn);
		return false;
	}
	if($('#txtrtspport').val()==null||$('#txtrtspport').val()=='')
	{
		alert(str_RTSP_port_warn);
		return false;
	}
	if($('#txthttpport').val()==null||$('#txthttpport').val()=='')
	{
		alert(str_HTTP_port_warn);
		return false;
	}
	if($('#txtreflectbuffer').val()==null||$('#txtreflectbuffer').val()=='')
	{
		alert(str_str_ReflectBuffer_warn);
		return false;
	}
	if($('#txtmoviesfolder').val()==null||$('#txtmoviesfolder').val()=='')
	{
		alert(str_MoviesFolder_warn);
		return false;
	}
	if($('#txtlogfolder').val()==null||$('#txtlogfolder').val()=='')
	{
		alert(str_LogFolder_warn);
		return false;
	}
	if(!isPort($('#txtport').val(),str_Mongoose_port_warn1))
		return false;
	if(!isPort($('#txtrtspport').val(),str_RTSP_port_warn1))
		return false;
	if(!isPort($('#txthttpport').val(),str_HTTP_port_warn1))
		return false;
	if(!istime($('#txtreflectbuffer').val(),str_str_ReflectBuffer_warn1))
		return false;
	$.ajax({
                url: '/api/setPort',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: {n1: $('#txtport').val(),n2:$("#txtrtspport").val(),n3:$("#txthttpport").val(),n4:$("#txtreflectbuffer").val(),n5:$("#txtmoviesfolder").val(),n6:$("#txtlogfolder").val()},
                success: function (json) {
					
					 alert(str_warn_s);
					 window.location.reload();
                },
				error:function(msg)
				{	alert("error");}
            });
}
function getPort()
{
	
	$.ajax({
                url: '/api/getPort',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: { },
                success: function (json) {
					$("#txtport").val(json.MongoosePort);
					$("#txtrtspport").val(json.RTSPPort);
					$("#txthttpport").val(json.HTTPPort);
					$("#txtreflectbuffer").val(json.reflectbuffer);
					$("#txtmoviesfolder").val(json.moviesfolder);
					$("#txtlogfolder").val(json.logfolder);
                },
				error:function(msg)
				{	alert(textStatus);}
            });
}
//++++++++++++++++++++++++++++++++++++++++++++++
//重启
function restart()
{
	
	$.ajax({
                url: '/api/restart',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: {},
                success: function (json) {
					
					 alert(str_warn_s);
					 window.location.reload();
                },
				error:function(msg)
				{	alert(str_warn_s);}
            });
}


//++++++++++++++++++++++++++++++++++++++++++++++
//hls设置
function setHLS()
{
	
	if($('#txthttproot').val()==null||$('#txthttproot').val()=='')
	{
		alert(str_hls_http_root_warn);
		return false;
	}
	if($('#txttsd').val()==null||$('#txttsd').val()=='')
	{
		alert(str_hls_live_tsduration_warn);
		return false;
	}
	if($('#txttsc').val()==null||$('#txttsc').val()=='')
	{
		alert(str_hls_live_tscapacity_warn);
		return false;
	}
	
	if(!istime($('#txttsd').val(),str_hls_live_tsduration_warn1))
		return false;
	if(!istime($('#txttsc').val(),str_hls_live_tscapacity_warn1))
		return false;
	$.ajax({
                url: '/api/setHLS',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: {n1: $('#txthttproot').val(),n2:$("#txttsd").val(),n3:$("#txttsc").val()},
                success: function (json) {
					
					 alert(str_warn_s);
					 window.location.reload();
                },
				error:function(msg)
				{	alert("error");}
            });
}
function getHLS()
{
	
	$.ajax({
                url: '/api/getHLS',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: { },
                success: function (json) {
					$("#txthttproot").val(json.httproot);
					$("#txttsd").val(json.tsd);
					$("#txttsc").val(json.tsc);
					
                },
				error:function(msg)
				{	alert(textStatus);}
            });
}
//++++++++++++++++++++++++++++++++++++++++++++++



//++++++++++++++++++++++++++++++++++++++++++++++
//rtsp设置
function setRTSP()
{
	var out=$("input[type='checkbox']").is(':checked');
	if(out==true)
	{
		out=1;
	}
	else{
		out=0;
	}
	if($('#txtbuffersecs').val()==null||$('#txtbuffersecs').val()=='')
	{
		alert(str_rtsp_buffersecs_warn);
		return false;
	}
	if(!istime($('#txtbuffersecs').val(),str_rtsp_buffersecs_warn1))
		return false;
	$.ajax({
                url: '/api/setRTSP',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: {n1: $('#txtbuffersecs').val(),n2:out},
                success: function (json) {
					
					 alert(str_warn_s);
					 window.location.reload();
                },
				error:function(msg)
				{	alert("error");}
            });
}
function getRTSP()
{
	
	$.ajax({
                url: '/api/getRTSP',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: { },
                success: function (json) {
					
					$("#txtbuffersecs").val(json.buffersecs);
					if(json.out==1)
					{
						$("[name = chkItem]:checkbox").attr("checked", true);
						
					}
					
					  
					
                },
				error:function(msg)
				{	alert(textStatus);}
            });
}

//++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++
//hls直播列表
function gotoAddHlsList()
{
	window.location.href='addhlslist.html';
	return false;
}
function gotoHlsList()
{
	window.location.href='hlslist.html';
	return false;
}
function getHLSList()
{
	$.ajax({
                url: '/api/getHLSList',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: { },
                success: function (json) {
					var pagecount=json.EasyDarwin.Body.SessionCount;
					//var pagecount=siteList.EasyDarwin.Body.SessionCount;
					if(pagecount=="0")
					{
						var html="<div class=\"panel-body no-padding\" ><table class=\"table table-striped\"><thead><tr class=\"warning\"><th>Index</th><th>Name</th><th>Source</th><th>Url</th><th>Bitrate</th><th>Option</th></tr></thead><tbody>";
						html+="</tbody></table></div>";
						document.getElementById("divload").innerHTML= html;
					}
					else{
							OutputHtml(parseInt(pagecount),1,json);
					}
				
                },
				error:function(msg)
				{	alert(textStatus);}
            });
}
function BHLSNameRepeat()
{
	var re=false;
	$.ajax({
                url: '/api/getHLSList',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: { },
                success: function (json) {
					var pagecount=json.EasyDarwin.Body.SessionCount;
					for(var i = 0;i < pagecount;i ++)
					{
						if($('#txtname').val()==json.EasyDarwin.Body.Sessions[i].name)
						{
							re=true;
							
							break;
						}
					}
					
                },
				error:function(msg)
				{	}
            });
	
	return re;
}
function addHLSList()
{
	
	if($('#txtname').val()==null||$('#txtname').val()=='')
	{
		alert(str_hls_list_name_warn);
		return false;
	}
	if($('#txturl').val()==null||$('#txturl').val()=='')
	{
		alert(str_hls_list_url_warn);
		return false;
	}
	if($('#txttimeout').val()==null||$('#txttimeout').val()=='')
	{
		alert(str_hls_list_timeout_warn);
		return false;
	}
	if(!istime($('#txttimeout').val(),str_hls_list_timeout_warn1))
		return false;
	
	if(BHLSNameRepeat()==true)
	{
		
		alert(str_hls_list_name_warn1);
		return false;
	}
	else{
		$.ajax({
                url: '/api/addHLSList',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: {n1: $('#txtname').val(),n2: $('#txturl').val(),n3: $('#txttimeout').val()},
                success: function (json) {
					
					 if(json.result=="1")
					 {
						 alert(str_warn_s);
						 window.location.href='hlslist.html';
						 
					 }
					 else{
						 alert(str_warn_f);
						 window.location.reload();
					 }
					 
                },
				error:function(msg)
				{	alert("error");}
            });
			return false;
	}
	
	
}
//++++++++++++++++++++++++++++++++++++++++++++++
function GetQueryString(name)
{
     var reg = new RegExp("(^|&)"+ name +"=([^&]*)(&|$)");
     var r = window.location.search.substr(1).match(reg);
     if(r!=null)return  unescape(r[2]); return null;
}

function load(seturl) {
           
            var url = seturl;
             var snap = '';
             var flashvars = {
                 f: 'm3u8.swf',
                 a: url,
                 c: 0,
                 s: 4,
                 lv: 0,
                 p:1
               
             };
             var params = { bgcolor: '#FFF', allowFullScreen: true, allowScriptAccess: 'always' }; //这里定义播放器的其它参数如背景色（跟flashvars中的b不同），是否支持全屏，是否支持交互
             var video = [url , url , url ];
             
             CKobject.embed('ckplayer/ckplayer.swf', 'dvplay', 'ckplayer_dvplay', '100%', '100%', false, flashvars, video, params);
        }
		
		function getpar()
		{
			var name=GetQueryString("name");
			var url=null;
			$.ajax({
                url: '/api/getHLSList',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: { },
                success: function (json) {
					var pagecount=json.EasyDarwin.Body.SessionCount;
					for(var i = 0;i < pagecount;i ++)
					{
						if(name==json.EasyDarwin.Body.Sessions[i].name)
						{
							url=json.EasyDarwin.Body.Sessions[i].url;
							
							break;
						}
					}
					
                },
				error:function(msg)
				{	}
            });
			load(url);
		}
//++++++++++++++++++++++++++++
function deleteHLSList(name)
{
if(confirm(str_confirm_delete))
 {
 $.ajax({
                url: '/api/StopHLS',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: {n1: name},
                success: function (json) {
					alert(str_warn_s);
					 window.location.reload();
					 
                },
				error:function(msg)
				{	alert("error");}
            });
 }

		
			
}
function gotolist()
{	
			
	window.location.href='hlslist.html';
	return false;
}

function getRTSPList()
{
	
	$.ajax({
                url: '/api/getRTSPList',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: { },
                success: function (json) {
					var pagecount=json.EasyDarwin.Body.SessionCount;
					//var pagecount=siteList.EasyDarwin.Body.SessionCount;
					if(pagecount=="0")
					{
						var html="<div class=\"panel-body no-padding\" ><table class=\"table table-striped\"><thead><tr class=\"warning\"><th>Index</th><th>Name</th><th>Source</th><th>Url</th></tr></thead><tbody>";
						html+="</tbody></table></div>";
						document.getElementById("divload").innerHTML= html;
					}
					else{
							OutputHtml(parseInt(pagecount),1,json);
					}
					
				
                },
				error:function(msg)
				{	alert(textStatus);}
            });
}