$(function(){
	$(".select").each(function(){
		var s=$(this);
		var z=parseInt(s.css("z-index"));
		var dt=$(this).children("dt");
		var dd=$(this).children("dd");
		var _show=function(){dd.slideDown(200);dt.addClass("cur");s.css("z-index",z+1);};   //展开效果
		var _hide=function(){dd.slideUp(200);dt.removeClass("cur");s.css("z-index",z);};    //关闭效果


		dt.click(function(){dd.is(":hidden")?_show():_hide();});

		dd.find("a").click(function(){dt.addClass('active').html($(this).html());_hide();});     //选择效果（如需要传值，可自定义参数，在此处返回对应的“value”值 ）

		$("body").click(function(i){ !$(i.target).parents(".select").first().is(s) ? _hide():"";});
	})
})