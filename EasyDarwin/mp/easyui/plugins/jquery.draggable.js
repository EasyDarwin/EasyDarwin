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
function _1(e){
var _2=$.data(e.data.target,"draggable");
var _3=_2.options;
var _4=_2.proxy;
var _5=e.data;
var _6=_5.startLeft+e.pageX-_5.startX;
var _7=_5.startTop+e.pageY-_5.startY;
if(_4){
if(_4.parent()[0]==document.body){
if(_3.deltaX!=null&&_3.deltaX!=undefined){
_6=e.pageX+_3.deltaX;
}else{
_6=e.pageX-e.data.offsetWidth;
}
if(_3.deltaY!=null&&_3.deltaY!=undefined){
_7=e.pageY+_3.deltaY;
}else{
_7=e.pageY-e.data.offsetHeight;
}
}else{
if(_3.deltaX!=null&&_3.deltaX!=undefined){
_6+=e.data.offsetWidth+_3.deltaX;
}
if(_3.deltaY!=null&&_3.deltaY!=undefined){
_7+=e.data.offsetHeight+_3.deltaY;
}
}
}
if(e.data.parent!=document.body){
_6+=$(e.data.parent).scrollLeft();
_7+=$(e.data.parent).scrollTop();
}
if(_3.axis=="h"){
_5.left=_6;
}else{
if(_3.axis=="v"){
_5.top=_7;
}else{
_5.left=_6;
_5.top=_7;
}
}
};
function _8(e){
var _9=$.data(e.data.target,"draggable");
var _a=_9.options;
var _b=_9.proxy;
if(!_b){
_b=$(e.data.target);
}
_b.css({left:e.data.left,top:e.data.top});
$("body").css("cursor",_a.cursor);
};
function _c(e){
if(!$.fn.draggable.isDragging){
return false;
}
var _d=$.data(e.data.target,"draggable");
var _e=_d.options;
var _f=$(".droppable").filter(function(){
return e.data.target!=this;
}).filter(function(){
var _10=$.data(this,"droppable").options.accept;
if(_10){
return $(_10).filter(function(){
return this==e.data.target;
}).length>0;
}else{
return true;
}
});
_d.droppables=_f;
var _11=_d.proxy;
if(!_11){
if(_e.proxy){
if(_e.proxy=="clone"){
_11=$(e.data.target).clone().insertAfter(e.data.target);
}else{
_11=_e.proxy.call(e.data.target,e.data.target);
}
_d.proxy=_11;
}else{
_11=$(e.data.target);
}
}
_11.css("position","absolute");
_1(e);
_8(e);
_e.onStartDrag.call(e.data.target,e);
return false;
};
function _12(e){
if(!$.fn.draggable.isDragging){
return false;
}
var _13=$.data(e.data.target,"draggable");
_1(e);
if(_13.options.onDrag.call(e.data.target,e)!=false){
_8(e);
}
var _14=e.data.target;
_13.droppables.each(function(){
var _15=$(this);
if(_15.droppable("options").disabled){
return;
}
var p2=_15.offset();
if(e.pageX>p2.left&&e.pageX<p2.left+_15.outerWidth()&&e.pageY>p2.top&&e.pageY<p2.top+_15.outerHeight()){
if(!this.entered){
$(this).trigger("_dragenter",[_14]);
this.entered=true;
}
$(this).trigger("_dragover",[_14]);
}else{
if(this.entered){
$(this).trigger("_dragleave",[_14]);
this.entered=false;
}
}
});
return false;
};
function _16(e){
if(!$.fn.draggable.isDragging){
_17();
return false;
}
_12(e);
var _18=$.data(e.data.target,"draggable");
var _19=_18.proxy;
var _1a=_18.options;
if(_1a.revert){
if(_1b()==true){
$(e.data.target).css({position:e.data.startPosition,left:e.data.startLeft,top:e.data.startTop});
}else{
if(_19){
var _1c,top;
if(_19.parent()[0]==document.body){
_1c=e.data.startX-e.data.offsetWidth;
top=e.data.startY-e.data.offsetHeight;
}else{
_1c=e.data.startLeft;
top=e.data.startTop;
}
_19.animate({left:_1c,top:top},function(){
_1d();
});
}else{
$(e.data.target).animate({left:e.data.startLeft,top:e.data.startTop},function(){
$(e.data.target).css("position",e.data.startPosition);
});
}
}
}else{
$(e.data.target).css({position:"absolute",left:e.data.left,top:e.data.top});
_1b();
}
_1a.onStopDrag.call(e.data.target,e);
_17();
function _1d(){
if(_19){
_19.remove();
}
_18.proxy=null;
};
function _1b(){
var _1e=false;
_18.droppables.each(function(){
var _1f=$(this);
if(_1f.droppable("options").disabled){
return;
}
var p2=_1f.offset();
if(e.pageX>p2.left&&e.pageX<p2.left+_1f.outerWidth()&&e.pageY>p2.top&&e.pageY<p2.top+_1f.outerHeight()){
if(_1a.revert){
$(e.data.target).css({position:e.data.startPosition,left:e.data.startLeft,top:e.data.startTop});
}
$(this).trigger("_drop",[e.data.target]);
_1d();
_1e=true;
this.entered=false;
return false;
}
});
if(!_1e&&!_1a.revert){
_1d();
}
return _1e;
};
return false;
};
function _17(){
if($.fn.draggable.timer){
clearTimeout($.fn.draggable.timer);
$.fn.draggable.timer=undefined;
}
$(document).unbind(".draggable");
$.fn.draggable.isDragging=false;
setTimeout(function(){
$("body").css("cursor","");
},100);
};
$.fn.draggable=function(_20,_21){
if(typeof _20=="string"){
return $.fn.draggable.methods[_20](this,_21);
}
return this.each(function(){
var _22;
var _23=$.data(this,"draggable");
if(_23){
_23.handle.unbind(".draggable");
_22=$.extend(_23.options,_20);
}else{
_22=$.extend({},$.fn.draggable.defaults,$.fn.draggable.parseOptions(this),_20||{});
}
var _24=_22.handle?(typeof _22.handle=="string"?$(_22.handle,this):_22.handle):$(this);
$.data(this,"draggable",{options:_22,handle:_24});
if(_22.disabled){
$(this).css("cursor","");
return;
}
_24.unbind(".draggable").bind("mousemove.draggable",{target:this},function(e){
if($.fn.draggable.isDragging){
return;
}
var _25=$.data(e.data.target,"draggable").options;
if(_26(e)){
$(this).css("cursor",_25.cursor);
}else{
$(this).css("cursor","");
}
}).bind("mouseleave.draggable",{target:this},function(e){
$(this).css("cursor","");
}).bind("mousedown.draggable",{target:this},function(e){
if(_26(e)==false){
return;
}
$(this).css("cursor","");
var _27=$(e.data.target).position();
var _28=$(e.data.target).offset();
var _29={startPosition:$(e.data.target).css("position"),startLeft:_27.left,startTop:_27.top,left:_27.left,top:_27.top,startX:e.pageX,startY:e.pageY,offsetWidth:(e.pageX-_28.left),offsetHeight:(e.pageY-_28.top),target:e.data.target,parent:$(e.data.target).parent()[0]};
$.extend(e.data,_29);
var _2a=$.data(e.data.target,"draggable").options;
if(_2a.onBeforeDrag.call(e.data.target,e)==false){
return;
}
$(document).bind("mousedown.draggable",e.data,_c);
$(document).bind("mousemove.draggable",e.data,_12);
$(document).bind("mouseup.draggable",e.data,_16);
$.fn.draggable.timer=setTimeout(function(){
$.fn.draggable.isDragging=true;
_c(e);
},_2a.delay);
return false;
});
function _26(e){
var _2b=$.data(e.data.target,"draggable");
var _2c=_2b.handle;
var _2d=$(_2c).offset();
var _2e=$(_2c).outerWidth();
var _2f=$(_2c).outerHeight();
var t=e.pageY-_2d.top;
var r=_2d.left+_2e-e.pageX;
var b=_2d.top+_2f-e.pageY;
var l=e.pageX-_2d.left;
return Math.min(t,r,b,l)>_2b.options.edge;
};
});
};
$.fn.draggable.methods={options:function(jq){
return $.data(jq[0],"draggable").options;
},proxy:function(jq){
return $.data(jq[0],"draggable").proxy;
},enable:function(jq){
return jq.each(function(){
$(this).draggable({disabled:false});
});
},disable:function(jq){
return jq.each(function(){
$(this).draggable({disabled:true});
});
}};
$.fn.draggable.parseOptions=function(_30){
var t=$(_30);
return $.extend({},$.parser.parseOptions(_30,["cursor","handle","axis",{"revert":"boolean","deltaX":"number","deltaY":"number","edge":"number","delay":"number"}]),{disabled:(t.attr("disabled")?true:undefined)});
};
$.fn.draggable.defaults={proxy:null,revert:false,cursor:"move",deltaX:null,deltaY:null,handle:null,disabled:false,edge:0,axis:null,delay:100,onBeforeDrag:function(e){
},onStartDrag:function(e){
},onDrag:function(e){
},onStopDrag:function(e){
}};
$.fn.draggable.isDragging=false;
})(jQuery);

