/**
 * jQuery EasyUI 1.4.4
 * 
 * Copyright (c) 2009-2015 www.jeasyui.com. All rights reserved.
 *
 * Licensed under the freeware license: http://www.jeasyui.com/license_freeware.php
 * To use it on other terms please contact us: info@jeasyui.com
 *
 */
(function($){
function _1(_2){
$(_2).addClass("progressbar");
$(_2).html("<div class=\"progressbar-text\"></div><div class=\"progressbar-value\"><div class=\"progressbar-text\"></div></div>");
$(_2).bind("_resize",function(e,_3){
if($(this).hasClass("easyui-fluid")||_3){
_4(_2);
}
return false;
});
return $(_2);
};
function _4(_5,_6){
var _7=$.data(_5,"progressbar").options;
var _8=$.data(_5,"progressbar").bar;
if(_6){
_7.width=_6;
}
_8._size(_7);
_8.find("div.progressbar-text").css("width",_8.width());
_8.find("div.progressbar-text,div.progressbar-value").css({height:_8.height()+"px",lineHeight:_8.height()+"px"});
};
$.fn.progressbar=function(_9,_a){
if(typeof _9=="string"){
var _b=$.fn.progressbar.methods[_9];
if(_b){
return _b(this,_a);
}
}
_9=_9||{};
return this.each(function(){
var _c=$.data(this,"progressbar");
if(_c){
$.extend(_c.options,_9);
}else{
_c=$.data(this,"progressbar",{options:$.extend({},$.fn.progressbar.defaults,$.fn.progressbar.parseOptions(this),_9),bar:_1(this)});
}
$(this).progressbar("setValue",_c.options.value);
_4(this);
});
};
$.fn.progressbar.methods={options:function(jq){
return $.data(jq[0],"progressbar").options;
},resize:function(jq,_d){
return jq.each(function(){
_4(this,_d);
});
},getValue:function(jq){
return $.data(jq[0],"progressbar").options.value;
},setValue:function(jq,_e){
if(_e<0){
_e=0;
}
if(_e>100){
_e=100;
}
return jq.each(function(){
var _f=$.data(this,"progressbar").options;
var _10=_f.text.replace(/{value}/,_e);
var _11=_f.value;
_f.value=_e;
$(this).find("div.progressbar-value").width(_e+"%");
$(this).find("div.progressbar-text").html(_10);
if(_11!=_e){
_f.onChange.call(this,_e,_11);
}
});
}};
$.fn.progressbar.parseOptions=function(_12){
return $.extend({},$.parser.parseOptions(_12,["width","height","text",{value:"number"}]));
};
$.fn.progressbar.defaults={width:"auto",height:22,value:0,text:"{value}%",onChange:function(_13,_14){
}};
})(jQuery);

