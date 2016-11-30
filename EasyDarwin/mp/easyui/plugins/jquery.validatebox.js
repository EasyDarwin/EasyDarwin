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
$(_2).addClass("validatebox-text");
};
function _3(_4){
var _5=$.data(_4,"validatebox");
_5.validating=false;
if(_5.timer){
clearTimeout(_5.timer);
}
$(_4).tooltip("destroy");
$(_4).unbind();
$(_4).remove();
};
function _6(_7){
var _8=$.data(_7,"validatebox").options;
var _9=$(_7);
_9.unbind(".validatebox");
if(_8.novalidate||_9.is(":disabled")){
return;
}
for(var _a in _8.events){
$(_7).bind(_a+".validatebox",{target:_7},_8.events[_a]);
}
};
function _b(e){
var _c=e.data.target;
var _d=$.data(_c,"validatebox");
var _e=$(_c);
if($(_c).attr("readonly")){
return;
}
_d.validating=true;
_d.value=undefined;
(function(){
if(_d.validating){
if(_d.value!=_e.val()){
_d.value=_e.val();
if(_d.timer){
clearTimeout(_d.timer);
}
_d.timer=setTimeout(function(){
$(_c).validatebox("validate");
},_d.options.delay);
}else{
_f(_c);
}
setTimeout(arguments.callee,200);
}
})();
};
function _10(e){
var _11=e.data.target;
var _12=$.data(_11,"validatebox");
if(_12.timer){
clearTimeout(_12.timer);
_12.timer=undefined;
}
_12.validating=false;
_13(_11);
};
function _14(e){
var _15=e.data.target;
if($(_15).hasClass("validatebox-invalid")){
_16(_15);
}
};
function _17(e){
var _18=e.data.target;
var _19=$.data(_18,"validatebox");
if(!_19.validating){
_13(_18);
}
};
function _16(_1a){
var _1b=$.data(_1a,"validatebox");
var _1c=_1b.options;
$(_1a).tooltip($.extend({},_1c.tipOptions,{content:_1b.message,position:_1c.tipPosition,deltaX:_1c.deltaX})).tooltip("show");
_1b.tip=true;
};
function _f(_1d){
var _1e=$.data(_1d,"validatebox");
if(_1e&&_1e.tip){
$(_1d).tooltip("reposition");
}
};
function _13(_1f){
var _20=$.data(_1f,"validatebox");
_20.tip=false;
$(_1f).tooltip("hide");
};
function _21(_22){
var _23=$.data(_22,"validatebox");
var _24=_23.options;
var box=$(_22);
_24.onBeforeValidate.call(_22);
var _25=_26();
_24.onValidate.call(_22,_25);
return _25;
function _27(msg){
_23.message=msg;
};
function _28(_29,_2a){
var _2b=box.val();
var _2c=/([a-zA-Z_]+)(.*)/.exec(_29);
var _2d=_24.rules[_2c[1]];
if(_2d&&_2b){
var _2e=_2a||_24.validParams||eval(_2c[2]);
if(!_2d["validator"].call(_22,_2b,_2e)){
box.addClass("validatebox-invalid");
var _2f=_2d["message"];
if(_2e){
for(var i=0;i<_2e.length;i++){
_2f=_2f.replace(new RegExp("\\{"+i+"\\}","g"),_2e[i]);
}
}
_27(_24.invalidMessage||_2f);
if(_23.validating){
_16(_22);
}
return false;
}
}
return true;
};
function _26(){
box.removeClass("validatebox-invalid");
_13(_22);
if(_24.novalidate||box.is(":disabled")){
return true;
}
if(_24.required){
if(box.val()==""){
box.addClass("validatebox-invalid");
_27(_24.missingMessage);
if(_23.validating){
_16(_22);
}
return false;
}
}
if(_24.validType){
if($.isArray(_24.validType)){
for(var i=0;i<_24.validType.length;i++){
if(!_28(_24.validType[i])){
return false;
}
}
}else{
if(typeof _24.validType=="string"){
if(!_28(_24.validType)){
return false;
}
}else{
for(var _30 in _24.validType){
var _31=_24.validType[_30];
if(!_28(_30,_31)){
return false;
}
}
}
}
}
return true;
};
};
function _32(_33,_34){
var _35=$.data(_33,"validatebox").options;
if(_34!=undefined){
_35.novalidate=_34;
}
if(_35.novalidate){
$(_33).removeClass("validatebox-invalid");
_13(_33);
}
_21(_33);
_6(_33);
};
$.fn.validatebox=function(_36,_37){
if(typeof _36=="string"){
return $.fn.validatebox.methods[_36](this,_37);
}
_36=_36||{};
return this.each(function(){
var _38=$.data(this,"validatebox");
if(_38){
$.extend(_38.options,_36);
}else{
_1(this);
$.data(this,"validatebox",{options:$.extend({},$.fn.validatebox.defaults,$.fn.validatebox.parseOptions(this),_36)});
}
_32(this);
_21(this);
});
};
$.fn.validatebox.methods={options:function(jq){
return $.data(jq[0],"validatebox").options;
},destroy:function(jq){
return jq.each(function(){
_3(this);
});
},validate:function(jq){
return jq.each(function(){
_21(this);
});
},isValid:function(jq){
return _21(jq[0]);
},enableValidation:function(jq){
return jq.each(function(){
_32(this,false);
});
},disableValidation:function(jq){
return jq.each(function(){
_32(this,true);
});
}};
$.fn.validatebox.parseOptions=function(_39){
var t=$(_39);
return $.extend({},$.parser.parseOptions(_39,["validType","missingMessage","invalidMessage","tipPosition",{delay:"number",deltaX:"number"}]),{required:(t.attr("required")?true:undefined),novalidate:(t.attr("novalidate")!=undefined?true:undefined)});
};
$.fn.validatebox.defaults={required:false,validType:null,validParams:null,delay:200,missingMessage:"This field is required.",invalidMessage:null,tipPosition:"right",deltaX:0,novalidate:false,events:{focus:_b,blur:_10,mouseenter:_14,mouseleave:_17,click:function(e){
var t=$(e.data.target);
if(!t.is(":focus")){
t.trigger("focus");
}
}},tipOptions:{showEvent:"none",hideEvent:"none",showDelay:0,hideDelay:0,zIndex:"",onShow:function(){
$(this).tooltip("tip").css({color:"#000",borderColor:"#CC9933",backgroundColor:"#FFFFCC"});
},onHide:function(){
$(this).tooltip("destroy");
}},rules:{email:{validator:function(_3a){
return /^((([a-z]|\d|[!#\$%&'\*\+\-\/=\?\^_`{\|}~]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])+(\.([a-z]|\d|[!#\$%&'\*\+\-\/=\?\^_`{\|}~]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])+)*)|((\x22)((((\x20|\x09)*(\x0d\x0a))?(\x20|\x09)+)?(([\x01-\x08\x0b\x0c\x0e-\x1f\x7f]|\x21|[\x23-\x5b]|[\x5d-\x7e]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(\\([\x01-\x09\x0b\x0c\x0d-\x7f]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF]))))*(((\x20|\x09)*(\x0d\x0a))?(\x20|\x09)+)?(\x22)))@((([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.)+(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.?$/i.test(_3a);
},message:"Please enter a valid email address."},url:{validator:function(_3b){
return /^(https?|ftp):\/\/(((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:)*@)?(((\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5]))|((([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.)+(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.?)(:\d*)?)(\/((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)+(\/(([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)*)*)?)?(\?((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)|[\uE000-\uF8FF]|\/|\?)*)?(\#((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)|\/|\?)*)?$/i.test(_3b);
},message:"Please enter a valid URL."},length:{validator:function(_3c,_3d){
var len=$.trim(_3c).length;
return len>=_3d[0]&&len<=_3d[1];
},message:"Please enter a value between {0} and {1}."},remote:{validator:function(_3e,_3f){
var _40={};
_40[_3f[1]]=_3e;
var _41=$.ajax({url:_3f[0],dataType:"json",data:_40,async:false,cache:false,type:"post"}).responseText;
return _41=="true";
},message:"Please fix this field."}},onBeforeValidate:function(){
},onValidate:function(_42){
}};
})(jQuery);

