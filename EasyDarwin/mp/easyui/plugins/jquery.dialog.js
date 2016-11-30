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
var _3=$.data(_2,"dialog").options;
_3.inited=false;
$(_2).window($.extend({},_3,{onResize:function(w,h){
if(_3.inited){
_b(this);
_3.onResize.call(this,w,h);
}
}}));
var _4=$(_2).window("window");
if(_3.toolbar){
if($.isArray(_3.toolbar)){
$(_2).siblings("div.dialog-toolbar").remove();
var _5=$("<div class=\"dialog-toolbar\"><table cellspacing=\"0\" cellpadding=\"0\"><tr></tr></table></div>").appendTo(_4);
var tr=_5.find("tr");
for(var i=0;i<_3.toolbar.length;i++){
var _6=_3.toolbar[i];
if(_6=="-"){
$("<td><div class=\"dialog-tool-separator\"></div></td>").appendTo(tr);
}else{
var td=$("<td></td>").appendTo(tr);
var _7=$("<a href=\"javascript:void(0)\"></a>").appendTo(td);
_7[0].onclick=eval(_6.handler||function(){
});
_7.linkbutton($.extend({},_6,{plain:true}));
}
}
}else{
$(_3.toolbar).addClass("dialog-toolbar").appendTo(_4);
$(_3.toolbar).show();
}
}else{
$(_2).siblings("div.dialog-toolbar").remove();
}
if(_3.buttons){
if($.isArray(_3.buttons)){
$(_2).siblings("div.dialog-button").remove();
var _8=$("<div class=\"dialog-button\"></div>").appendTo(_4);
for(var i=0;i<_3.buttons.length;i++){
var p=_3.buttons[i];
var _9=$("<a href=\"javascript:void(0)\"></a>").appendTo(_8);
if(p.handler){
_9[0].onclick=p.handler;
}
_9.linkbutton(p);
}
}else{
$(_3.buttons).addClass("dialog-button").appendTo(_4);
$(_3.buttons).show();
}
}else{
$(_2).siblings("div.dialog-button").remove();
}
_3.inited=true;
var _a=_3.closed;
_4.show();
$(_2).window("resize");
if(_a){
_4.hide();
}
};
function _b(_c,_d){
var t=$(_c);
var _e=t.dialog("options");
var _f=_e.noheader;
var tb=t.siblings(".dialog-toolbar");
var bb=t.siblings(".dialog-button");
tb.insertBefore(_c).css({position:"relative",borderTopWidth:(_f?1:0),top:(_f?tb.length:0)});
bb.insertAfter(_c).css({position:"relative",top:-1});
tb.add(bb)._outerWidth(t._outerWidth()).find(".easyui-fluid:visible").each(function(){
$(this).triggerHandler("_resize");
});
var _10=tb._outerHeight()+bb._outerHeight();
if(!isNaN(parseInt(_e.height))){
t._outerHeight(t._outerHeight()-_10);
}else{
var _11=t._size("min-height");
if(_11){
t._size("min-height",_11-_10);
}
var _12=t._size("max-height");
if(_12){
t._size("max-height",_12-_10);
}
}
var _13=$.data(_c,"window").shadow;
if(_13){
var cc=t.panel("panel");
_13.css({width:cc._outerWidth(),height:cc._outerHeight()});
}
};
$.fn.dialog=function(_14,_15){
if(typeof _14=="string"){
var _16=$.fn.dialog.methods[_14];
if(_16){
return _16(this,_15);
}else{
return this.window(_14,_15);
}
}
_14=_14||{};
return this.each(function(){
var _17=$.data(this,"dialog");
if(_17){
$.extend(_17.options,_14);
}else{
$.data(this,"dialog",{options:$.extend({},$.fn.dialog.defaults,$.fn.dialog.parseOptions(this),_14)});
}
_1(this);
});
};
$.fn.dialog.methods={options:function(jq){
var _18=$.data(jq[0],"dialog").options;
var _19=jq.panel("options");
$.extend(_18,{width:_19.width,height:_19.height,left:_19.left,top:_19.top,closed:_19.closed,collapsed:_19.collapsed,minimized:_19.minimized,maximized:_19.maximized});
return _18;
},dialog:function(jq){
return jq.window("window");
}};
$.fn.dialog.parseOptions=function(_1a){
var t=$(_1a);
return $.extend({},$.fn.window.parseOptions(_1a),$.parser.parseOptions(_1a,["toolbar","buttons"]),{toolbar:(t.children(".dialog-toolbar").length?t.children(".dialog-toolbar").removeClass("dialog-toolbar"):undefined),buttons:(t.children(".dialog-button").length?t.children(".dialog-button").removeClass("dialog-button"):undefined)});
};
$.fn.dialog.defaults=$.extend({},$.fn.window.defaults,{title:"New Dialog",collapsible:false,minimizable:false,maximizable:false,resizable:false,toolbar:null,buttons:null});
})(jQuery);

