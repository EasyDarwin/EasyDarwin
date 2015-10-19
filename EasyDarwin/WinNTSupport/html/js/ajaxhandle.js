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
					var str="服务器稳定运行时间: "+strs[0]+"D-"+strs[1]+"H-"+strs[2]+"M-"+strs[3]+"S"
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
		alert("端口号不能为空");
		return false;
	}
	if(!isPort($('#txtport').val()))
		return false;
	$.ajax({
                url: '/api/setPort',
				contentType: 'application/x-www-form-urlencoded; charset=utf-8', 
                method: 'POST',
                dataType: 'json',
				cache: false,
				async: false,
                data: {n1: $('#txtport').val()},
                success: function (json) {
					
					 alert("操作成功");
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
					
					 alert("操作成功");
					 window.location.reload();
                },
				error:function(msg)
				{	alert("操作成功");}
            });
}
