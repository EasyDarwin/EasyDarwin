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
var _3=$("<div class=\"slider\">"+"<div class=\"slider-inner\">"+"<a href=\"javascript:void(0)\" class=\"slider-handle\"></a>"+"<span class=\"slider-tip\"></span>"+"</div>"+"<div class=\"slider-rule\"></div>"+"<div class=\"slider-rulelabel\"></div>"+"<div style=\"clear:both\"></div>"+"<input type=\"hidden\" class=\"slider-value\">"+"</div>").insertAfter(_2);
var t=$(_2);
t.addClass("slider-f").hide();
var _4=t.attr("name");
if(_4){
_3.find("input.slider-value").attr("name",_4);
t.removeAttr("name").attr("sliderName",_4);
}
_3.bind("_resize",function(e,_5){
if($(this).hasClass("easyui-fluid")||_5){
_6(_2);
}
return false;
});
return _3;
};
function _6(_7,_8){
var _9=$.data(_7,"slider");
var _a=_9.options;
var _b=_9.slider;
if(_8){
if(_8.width){
_a.width=_8.width;
}
if(_8.height){
_a.height=_8.height;
}
}
_b._size(_a);
if(_a.mode=="h"){
_b.css("height","");
_b.children("div").css("height","");
}else{
_b.css("width","");
_b.children("div").css("width","");
_b.children("div.slider-rule,div.slider-rulelabel,div.slider-inner")._outerHeight(_b._outerHeight());
}
_c(_7);
};
function _d(_e){
var _f=$.data(_e,"slider");
var _10=_f.options;
var _11=_f.slider;
var aa=_10.mode=="h"?_10.rule:_10.rule.slice(0).reverse();
if(_10.reversed){
aa=aa.slice(0).reverse();
}
_12(aa);
function _12(aa){
var _13=_11.find("div.slider-rule");
var _14=_11.find("div.slider-rulelabel");
_13.empty();
_14.empty();
for(var i=0;i<aa.length;i++){
var _15=i*100/(aa.length-1)+"%";
var _16=$("<span></span>").appendTo(_13);
_16.css((_10.mode=="h"?"left":"top"),_15);
if(aa[i]!="|"){
_16=$("<span></span>").appendTo(_14);
_16.html(aa[i]);
if(_10.mode=="h"){
_16.css({left:_15,marginLeft:-Math.round(_16.outerWidth()/2)});
}else{
_16.css({top:_15,marginTop:-Math.round(_16.outerHeight()/2)});
}
}
}
};
};
function _17(_18){
var _19=$.data(_18,"slider");
var _1a=_19.options;
var _1b=_19.slider;
_1b.removeClass("slider-h slider-v slider-disabled");
_1b.addClass(_1a.mode=="h"?"slider-h":"slider-v");
_1b.addClass(_1a.disabled?"slider-disabled":"");
var _1c=_1b.find(".slider-inner");
_1c.html("<a href=\"javascript:void(0)\" class=\"slider-handle\"></a>"+"<span class=\"slider-tip\"></span>");
if(_1a.range){
_1c.append("<a href=\"javascript:void(0)\" class=\"slider-handle\"></a>"+"<span class=\"slider-tip\"></span>");
}
_1b.find("a.slider-handle").draggable({axis:_1a.mode,cursor:"pointer",disabled:_1a.disabled,onDrag:function(e){
var _1d=e.data.left;
var _1e=_1b.width();
if(_1a.mode!="h"){
_1d=e.data.top;
_1e=_1b.height();
}
if(_1d<0||_1d>_1e){
return false;
}else{
_1f(_1d,this);
return false;
}
},onStartDrag:function(){
_19.isDragging=true;
_1a.onSlideStart.call(_18,_1a.value);
},onStopDrag:function(e){
_1f(_1a.mode=="h"?e.data.left:e.data.top,this);
_1a.onSlideEnd.call(_18,_1a.value);
_1a.onComplete.call(_18,_1a.value);
_19.isDragging=false;
}});
_1b.find("div.slider-inner").unbind(".slider").bind("mousedown.slider",function(e){
if(_19.isDragging||_1a.disabled){
return;
}
var pos=$(this).offset();
_1f(_1a.mode=="h"?(e.pageX-pos.left):(e.pageY-pos.top));
_1a.onComplete.call(_18,_1a.value);
});
function _1f(pos,_20){
var _21=_22(_18,pos);
var s=Math.abs(_21%_1a.step);
if(s<_1a.step/2){
_21-=s;
}else{
_21=_21-s+_1a.step;
}
if(_1a.range){
var v1=_1a.value[0];
var v2=_1a.value[1];
var m=parseFloat((v1+v2)/2);
if(_20){
var _23=$(_20).nextAll(".slider-handle").length>0;
if(_21<=v2&&_23){
v1=_21;
}else{
if(_21>=v1&&(!_23)){
v2=_21;
}
}
}else{
if(_21<v1){
v1=_21;
}else{
if(_21>v2){
v2=_21;
}else{
_21<m?v1=_21:v2=_21;
}
}
}
$(_18).slider("setValues",[v1,v2]);
}else{
$(_18).slider("setValue",_21);
}
};
};
function _24(_25,_26){
var _27=$.data(_25,"slider");
var _28=_27.options;
var _29=_27.slider;
var _2a=$.isArray(_28.value)?_28.value:[_28.value];
var _2b=[];
if(!$.isArray(_26)){
_26=$.map(String(_26).split(_28.separator),function(v){
return parseFloat(v);
});
}
_29.find(".slider-value").remove();
var _2c=$(_25).attr("sliderName")||"";
for(var i=0;i<_26.length;i++){
var _2d=_26[i];
if(_2d<_28.min){
_2d=_28.min;
}
if(_2d>_28.max){
_2d=_28.max;
}
var _2e=$("<input type=\"hidden\" class=\"slider-value\">").appendTo(_29);
_2e.attr("name",_2c);
_2e.val(_2d);
_2b.push(_2d);
var _2f=_29.find(".slider-handle:eq("+i+")");
var tip=_2f.next();
var pos=_30(_25,_2d);
if(_28.showTip){
tip.show();
tip.html(_28.tipFormatter.call(_25,_2d));
}else{
tip.hide();
}
if(_28.mode=="h"){
var _31="left:"+pos+"px;";
_2f.attr("style",_31);
tip.attr("style",_31+"margin-left:"+(-Math.round(tip.outerWidth()/2))+"px");
}else{
var _31="top:"+pos+"px;";
_2f.attr("style",_31);
tip.attr("style",_31+"margin-left:"+(-Math.round(tip.outerWidth()))+"px");
}
}
_28.value=_28.range?_2b:_2b[0];
$(_25).val(_28.range?_2b.join(_28.separator):_2b[0]);
if(_2a.join(",")!=_2b.join(",")){
_28.onChange.call(_25,_28.value,(_28.range?_2a:_2a[0]));
}
};
function _c(_32){
var _33=$.data(_32,"slider").options;
var fn=_33.onChange;
_33.onChange=function(){
};
_24(_32,_33.value);
_33.onChange=fn;
};
function _30(_34,_35){
var _36=$.data(_34,"slider");
var _37=_36.options;
var _38=_36.slider;
var _39=_37.mode=="h"?_38.width():_38.height();
var pos=_37.converter.toPosition.call(_34,_35,_39);
if(_37.mode=="v"){
pos=_38.height()-pos;
}
if(_37.reversed){
pos=_39-pos;
}
return pos.toFixed(0);
};
function _22(_3a,pos){
var _3b=$.data(_3a,"slider");
var _3c=_3b.options;
var _3d=_3b.slider;
var _3e=_3c.mode=="h"?_3d.width():_3d.height();
var pos=_3c.mode=="h"?(_3c.reversed?(_3e-pos):pos):(_3c.reversed?pos:(_3e-pos));
var _3f=_3c.converter.toValue.call(_3a,pos,_3e);
return _3f.toFixed(0);
};
$.fn.slider=function(_40,_41){
if(typeof _40=="string"){
return $.fn.slider.methods[_40](this,_41);
}
_40=_40||{};
return this.each(function(){
var _42=$.data(this,"slider");
if(_42){
$.extend(_42.options,_40);
}else{
_42=$.data(this,"slider",{options:$.extend({},$.fn.slider.defaults,$.fn.slider.parseOptions(this),_40),slider:_1(this)});
$(this).removeAttr("disabled");
}
var _43=_42.options;
_43.min=parseFloat(_43.min);
_43.max=parseFloat(_43.max);
if(_43.range){
if(!$.isArray(_43.value)){
_43.value=$.map(String(_43.value).split(_43.separator),function(v){
return parseFloat(v);
});
}
if(_43.value.length<2){
_43.value.push(_43.max);
}
}else{
_43.value=parseFloat(_43.value);
}
_43.step=parseFloat(_43.step);
_43.originalValue=_43.value;
_17(this);
_d(this);
_6(this);
});
};
$.fn.slider.methods={options:function(jq){
return $.data(jq[0],"slider").options;
},destroy:function(jq){
return jq.each(function(){
$.data(this,"slider").slider.remove();
$(this).remove();
});
},resize:function(jq,_44){
return jq.each(function(){
_6(this,_44);
});
},getValue:function(jq){
return jq.slider("options").value;
},getValues:function(jq){
return jq.slider("options").value;
},setValue:function(jq,_45){
return jq.each(function(){
_24(this,[_45]);
});
},setValues:function(jq,_46){
return jq.each(function(){
_24(this,_46);
});
},clear:function(jq){
return jq.each(function(){
var _47=$(this).slider("options");
_24(this,_47.range?[_47.min,_47.max]:[_47.min]);
});
},reset:function(jq){
return jq.each(function(){
var _48=$(this).slider("options");
$(this).slider(_48.range?"setValues":"setValue",_48.originalValue);
});
},enable:function(jq){
return jq.each(function(){
$.data(this,"slider").options.disabled=false;
_17(this);
});
},disable:function(jq){
return jq.each(function(){
$.data(this,"slider").options.disabled=true;
_17(this);
});
}};
$.fn.slider.parseOptions=function(_49){
var t=$(_49);
return $.extend({},$.parser.parseOptions(_49,["width","height","mode",{reversed:"boolean",showTip:"boolean",range:"boolean",min:"number",max:"number",step:"number"}]),{value:(t.val()||undefined),disabled:(t.attr("disabled")?true:undefined),rule:(t.attr("rule")?eval(t.attr("rule")):undefined)});
};
$.fn.slider.defaults={width:"auto",height:"auto",mode:"h",reversed:false,showTip:false,disabled:false,range:false,value:0,separator:",",min:0,max:100,step:1,rule:[],tipFormatter:function(_4a){
return _4a;
},converter:{toPosition:function(_4b,_4c){
var _4d=$(this).slider("options");
return (_4b-_4d.min)/(_4d.max-_4d.min)*_4c;
},toValue:function(pos,_4e){
var _4f=$(this).slider("options");
return _4f.min+(_4f.max-_4f.min)*(pos/_4e);
}},onChange:function(_50,_51){
},onSlideStart:function(_52){
},onSlideEnd:function(_53){
},onComplete:function(_54){
}};
})(jQuery);

