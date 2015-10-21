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
					var str=str_run_time+": "+strs[0]+"D-"+strs[1]+"H-"+strs[2]+"M-"+strs[3]+"S"
					document.getElementById('dvruntime').innerHTML=str;
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
