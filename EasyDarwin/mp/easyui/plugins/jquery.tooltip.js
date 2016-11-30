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
$(_2).addClass("tooltip-f");
};
function _3(_4){
var _5=$.data(_4,"tooltip").options;
$(_4).unbind(".tooltip").bind(_5.showEvent+".tooltip",function(e){
$(_4).tooltip("show",e);
}).bind(_5.hideEvent+".tooltip",function(e){
$(_4).tooltip("hide",e);
}).bind("mousemove.tooltip",function(e){
if(_5.trackMouse){
_5.trackMouseX=e.pageX;
_5.trackMouseY=e.pageY;
$(_4).tooltip("reposition");
}
});
};
function _6(_7){
var _8=$.data(_7,"tooltip");
if(_8.showTimer){
clearTimeout(_8.showTimer);
_8.showTimer=null;
}
if(_8.hideTimer){
clearTimeout(_8.hideTimer);
_8.hideTimer=null;
}
};
function _9(_a){
var _b=$.data(_a,"tooltip");
if(!_b||!_b.tip){
return;
}
var _c=_b.options;
var _d=_b.tip;
var _e={left:-100000,top:-100000};
if($(_a).is(":visible")){
_e=_f(_c.position);
if(_c.position=="top"&&_e.top<0){
_e=_f("bottom");
}else{
if((_c.position=="bottom")&&(_e.top+_d._outerHeight()>$(window)._outerHeight()+$(document).scrollTop())){
_e=_f("top");
}
}
if(_e.left<0){
if(_c.position=="left"){
_e=_f("right");
}else{
$(_a).tooltip("arrow").css("left",_d._outerWidth()/2+_e.left);
_e.left=0;
}
}else{
if(_e.left+_d._outerWidth()>$(window)._outerWidth()+$(document)._scrollLeft()){
if(_c.position=="right"){
_e=_f("left");
}else{
var _10=_e.left;
_e.left=$(window)._outerWidth()+$(document)._scrollLeft()-_d._outerWidth();
$(_a).tooltip("arrow").css("left",_d._outerWidth()/2-(_e.left-_10));
}
}
}
}
_d.css({left:_e.left,top:_e.top,zIndex:(_c.zIndex!=undefined?_c.zIndex:($.fn.window?$.fn.window.defaults.zIndex++:""))});
_c.onPosition.call(_a,_e.left,_e.top);
function _f(_11){
_c.position=_11||"bottom";
_d.removeClass("tooltip-top tooltip-bottom tooltip-left tooltip-right").addClass("tooltip-"+_c.position);
var _12,top;
if(_c.trackMouse){
t=$();
_12=_c.trackMouseX+_c.deltaX;
top=_c.trackMouseY+_c.deltaY;
}else{
var t=$(_a);
_12=t.offset().left+_c.deltaX;
top=t.offset().top+_c.deltaY;
}
switch(_c.position){
case "right":
_12+=t._outerWidth()+12+(_c.trackMouse?12:0);
top-=(_d._outerHeight()-t._outerHeight())/2;
break;
case "left":
_12-=_d._outerWidth()+12+(_c.trackMouse?12:0);
top-=(_d._outerHeight()-t._outerHeight())/2;
break;
case "top":
_12-=(_d._outerWidth()-t._outerWidth())/2;
top-=_d._outerHeight()+12+(_c.trackMouse?12:0);
break;
case "bottom":
_12-=(_d._outerWidth()-t._outerWidth())/2;
top+=t._outerHeight()+12+(_c.trackMouse?12:0);
break;
}
return {left:_12,top:top};
};
};
function _13(_14,e){
var _15=$.data(_14,"tooltip");
var _16=_15.options;
var tip=_15.tip;
if(!tip){
tip=$("<div tabindex=\"-1\" class=\"tooltip\">"+"<div class=\"tooltip-content\"></div>"+"<div class=\"tooltip-arrow-outer\"></div>"+"<div class=\"tooltip-arrow\"></div>"+"</div>").appendTo("body");
_15.tip=tip;
_17(_14);
}
_6(_14);
_15.showTimer=setTimeout(function(){
$(_14).tooltip("reposition");
tip.show();
_16.onShow.call(_14,e);
var _18=tip.children(".tooltip-arrow-outer");
var _19=tip.children(".tooltip-arrow");
var bc="border-"+_16.position+"-color";
_18.add(_19).css({borderTopColor:"",borderBottomColor:"",borderLeftColor:"",borderRightColor:""});
_18.css(bc,tip.css(bc));
_19.css(bc,tip.css("backgroundColor"));
},_16.showDelay);
};
function _1a(_1b,e){
var _1c=$.data(_1b,"tooltip");
if(_1c&&_1c.tip){
_6(_1b);
_1c.hideTimer=setTimeout(function(){
_1c.tip.hide();
_1c.options.onHide.call(_1b,e);
},_1c.options.hideDelay);
}
};
function _17(_1d,_1e){
var _1f=$.data(_1d,"tooltip");
var _20=_1f.options;
if(_1e){
_20.content=_1e;
}
if(!_1f.tip){
return;
}
var cc=typeof _20.content=="function"?_20.content.call(_1d):_20.content;
_1f.tip.children(".tooltip-content").html(cc);
_20.onUpdate.call(_1d,cc);
};
function _21(_22){
var _23=$.data(_22,"tooltip");
if(_23){
_6(_22);
var _24=_23.options;
if(_23.tip){
_23.tip.remove();
}
if(_24._title){
$(_22).attr("title",_24._title);
}
$.removeData(_22,"tooltip");
$(_22).unbind(".tooltip").removeClass("tooltip-f");
_24.onDestroy.call(_22);
}
};
$.fn.tooltip=function(_25,_26){
if(typeof _25=="string"){
return $.fn.tooltip.methods[_25](this,_26);
}
_25=_25||{};
return this.each(function(){
var _27=$.data(this,"tooltip");
if(_27){
$.extend(_27.options,_25);
}else{
$.data(this,"tooltip",{options:$.extend({},$.fn.tooltip.defaults,$.fn.tooltip.parseOptions(this),_25)});
_1(this);
}
_3(this);
_17(this);
});
};
$.fn.tooltip.methods={options:function(jq){
return $.data(jq[0],"tooltip").options;
},tip:function(jq){
return $.data(jq[0],"tooltip").tip;
},arrow:function(jq){
return jq.tooltip("tip").children(".tooltip-arrow-outer,.tooltip-arrow");
},show:function(jq,e){
return jq.each(function(){
_13(this,e);
});
},hide:function(jq,e){
return jq.each(function(){
_1a(this,e);
});
},update:function(jq,_28){
return jq.each(function(){
_17(this,_28);
});
},reposition:function(jq){
return jq.each(function(){
_9(this);
});
},destroy:function(jq){
return jq.each(function(){
_21(this);
});
}};
$.fn.tooltip.parseOptions=function(_29){
var t=$(_29);
var _2a=$.extend({},$.parser.parseOptions(_29,["position","showEvent","hideEvent","content",{trackMouse:"boolean",deltaX:"number",deltaY:"number",showDelay:"number",hideDelay:"number"}]),{_title:t.attr("title")});
t.attr("title","");
if(!_2a.content){
_2a.content=_2a._title;
}
return _2a;
};
$.fn.tooltip.defaults={position:"bottom",content:null,trackMouse:false,deltaX:0,deltaY:0,showEvent:"mouseenter",hideEvent:"mouseleave",showDelay:200,hideDelay:100,onShow:function(e){
},onHide:function(e){
},onUpdate:function(_2b){
},onPosition:function(_2c,top){
},onDestroy:function(){
}};
})(jQuery);

