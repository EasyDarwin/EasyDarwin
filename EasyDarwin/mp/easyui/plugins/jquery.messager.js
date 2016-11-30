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
function _1(){
$(document).unbind(".messager").bind("keydown.messager",function(e){
if(e.keyCode==27){
$("body").children("div.messager-window").children("div.messager-body").each(function(){
$(this).dialog("close");
});
}else{
if(e.keyCode==9){
var _2=$("body").children("div.messager-window");
if(!_2.length){
return;
}
var _3=_2.find(".messager-input,.messager-button .l-btn");
for(var i=0;i<_3.length;i++){
if($(_3[i]).is(":focus")){
$(_3[i>=_3.length-1?0:i+1]).focus();
return false;
}
}
}
}
});
};
function _4(){
$(document).unbind(".messager");
};
function _5(_6){
var _7=$.extend({},$.messager.defaults,{modal:false,shadow:false,draggable:false,resizable:false,closed:true,style:{left:"",top:"",right:0,zIndex:$.fn.window.defaults.zIndex++,bottom:-document.body.scrollTop-document.documentElement.scrollTop},title:"",width:250,height:100,minHeight:0,showType:"slide",showSpeed:600,content:_6.msg,timeout:4000},_6);
var _8=$("<div class=\"messager-body\"></div>").appendTo("body");
_8.dialog($.extend({},_7,{noheader:(_7.title?false:true),openAnimation:(_7.showType),closeAnimation:(_7.showType=="show"?"hide":_7.showType),openDuration:_7.showSpeed,closeDuration:_7.showSpeed,onOpen:function(){
_8.dialog("dialog").hover(function(){
if(_7.timer){
clearTimeout(_7.timer);
}
},function(){
_9();
});
_9();
function _9(){
if(_7.timeout>0){
_7.timer=setTimeout(function(){
if(_8.length&&_8.data("dialog")){
_8.dialog("close");
}
},_7.timeout);
}
};
if(_6.onOpen){
_6.onOpen.call(this);
}else{
_7.onOpen.call(this);
}
},onClose:function(){
if(_7.timer){
clearTimeout(_7.timer);
}
if(_6.onClose){
_6.onClose.call(this);
}else{
_7.onClose.call(this);
}
_8.dialog("destroy");
}}));
_8.dialog("dialog").css(_7.style);
_8.dialog("open");
return _8;
};
function _a(_b){
_1();
var _c=$("<div class=\"messager-body\"></div>").appendTo("body");
_c.dialog($.extend({},_b,{noheader:(_b.title?false:true),onClose:function(){
_4();
if(_b.onClose){
_b.onClose.call(this);
}
setTimeout(function(){
_c.dialog("destroy");
},100);
}}));
var _d=_c.dialog("dialog").addClass("messager-window");
_d.find(".dialog-button").addClass("messager-button").find("a:first").focus();
return _c;
};
function _e(_f,_10){
_f.dialog("close");
_f.dialog("options").fn(_10);
};
$.messager={show:function(_11){
return _5(_11);
},alert:function(_12,msg,_13,fn){
var _14=typeof _12=="object"?_12:{title:_12,msg:msg,icon:_13,fn:fn};
var cls=_14.icon?"messager-icon messager-"+_14.icon:"";
_14=$.extend({},$.messager.defaults,{content:"<div class=\""+cls+"\"></div>"+"<div>"+_14.msg+"</div>"+"<div style=\"clear:both;\"/>"},_14);
if(!_14.buttons){
_14.buttons=[{text:_14.ok,onClick:function(){
_e(dlg);
}}];
}
var dlg=_a(_14);
return dlg;
},confirm:function(_15,msg,fn){
var _16=typeof _15=="object"?_15:{title:_15,msg:msg,fn:fn};
_16=$.extend({},$.messager.defaults,{content:"<div class=\"messager-icon messager-question\"></div>"+"<div>"+_16.msg+"</div>"+"<div style=\"clear:both;\"/>"},_16);
if(!_16.buttons){
_16.buttons=[{text:_16.ok,onClick:function(){
_e(dlg,true);
}},{text:_16.cancel,onClick:function(){
_e(dlg,false);
}}];
}
var dlg=_a(_16);
return dlg;
},prompt:function(_17,msg,fn){
var _18=typeof _17=="object"?_17:{title:_17,msg:msg,fn:fn};
_18=$.extend({},$.messager.defaults,{content:"<div class=\"messager-icon messager-question\"></div>"+"<div>"+_18.msg+"</div>"+"<br/>"+"<div style=\"clear:both;\"/>"+"<div><input class=\"messager-input\" type=\"text\"/></div>"},_18);
if(!_18.buttons){
_18.buttons=[{text:_18.ok,onClick:function(){
_e(dlg,dlg.find(".messager-input").val());
}},{text:_18.cancel,onClick:function(){
_e(dlg);
}}];
}
var dlg=_a(_18);
dlg.find("input.messager-input").focus();
return dlg;
},progress:function(_19){
var _1a={bar:function(){
return $("body>div.messager-window").find("div.messager-p-bar");
},close:function(){
var dlg=$("body>div.messager-window>div.messager-body:has(div.messager-progress)");
if(dlg.length){
dlg.dialog("close");
}
}};
if(typeof _19=="string"){
var _1b=_1a[_19];
return _1b();
}
_19=_19||{};
var _1c=$.extend({},{title:"",minHeight:0,content:undefined,msg:"",text:undefined,interval:300},_19);
var dlg=_a($.extend({},$.messager.defaults,{content:"<div class=\"messager-progress\"><div class=\"messager-p-msg\">"+_1c.msg+"</div><div class=\"messager-p-bar\"></div></div>",closable:false,doSize:false},_1c,{onClose:function(){
if(this.timer){
clearInterval(this.timer);
}
if(_19.onClose){
_19.onClose.call(this);
}else{
$.messager.defaults.onClose.call(this);
}
}}));
var bar=dlg.find("div.messager-p-bar");
bar.progressbar({text:_1c.text});
dlg.dialog("resize");
if(_1c.interval){
dlg[0].timer=setInterval(function(){
var v=bar.progressbar("getValue");
v+=10;
if(v>100){
v=0;
}
bar.progressbar("setValue",v);
},_1c.interval);
}
return dlg;
}};
$.messager.defaults=$.extend({},$.fn.dialog.defaults,{ok:"Ok",cancel:"Cancel",width:300,height:"auto",minHeight:150,modal:true,collapsible:false,minimizable:false,maximizable:false,resizable:false,fn:function(){
}});
})(jQuery);

