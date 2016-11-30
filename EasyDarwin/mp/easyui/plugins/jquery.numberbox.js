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
var _3=$.data(_2,"numberbox");
var _4=_3.options;
$(_2).addClass("numberbox-f").textbox(_4);
$(_2).textbox("textbox").css({imeMode:"disabled"});
$(_2).attr("numberboxName",$(_2).attr("textboxName"));
_3.numberbox=$(_2).next();
_3.numberbox.addClass("numberbox");
var _5=_4.parser.call(_2,_4.value);
var _6=_4.formatter.call(_2,_5);
$(_2).numberbox("initValue",_5).numberbox("setText",_6);
};
function _7(_8,_9){
var _a=$.data(_8,"numberbox");
var _b=_a.options;
var _9=_b.parser.call(_8,_9);
var _c=_b.formatter.call(_8,_9);
_b.value=_9;
$(_8).textbox("setText",_c).textbox("setValue",_9);
_c=_b.formatter.call(_8,$(_8).textbox("getValue"));
$(_8).textbox("setText",_c);
};
$.fn.numberbox=function(_d,_e){
if(typeof _d=="string"){
var _f=$.fn.numberbox.methods[_d];
if(_f){
return _f(this,_e);
}else{
return this.textbox(_d,_e);
}
}
_d=_d||{};
return this.each(function(){
var _10=$.data(this,"numberbox");
if(_10){
$.extend(_10.options,_d);
}else{
_10=$.data(this,"numberbox",{options:$.extend({},$.fn.numberbox.defaults,$.fn.numberbox.parseOptions(this),_d)});
}
_1(this);
});
};
$.fn.numberbox.methods={options:function(jq){
var _11=jq.data("textbox")?jq.textbox("options"):{};
return $.extend($.data(jq[0],"numberbox").options,{width:_11.width,originalValue:_11.originalValue,disabled:_11.disabled,readonly:_11.readonly});
},fix:function(jq){
return jq.each(function(){
$(this).numberbox("setValue",$(this).numberbox("getText"));
});
},setValue:function(jq,_12){
return jq.each(function(){
_7(this,_12);
});
},clear:function(jq){
return jq.each(function(){
$(this).textbox("clear");
$(this).numberbox("options").value="";
});
},reset:function(jq){
return jq.each(function(){
$(this).textbox("reset");
$(this).numberbox("setValue",$(this).numberbox("getValue"));
});
}};
$.fn.numberbox.parseOptions=function(_13){
var t=$(_13);
return $.extend({},$.fn.textbox.parseOptions(_13),$.parser.parseOptions(_13,["decimalSeparator","groupSeparator","suffix",{min:"number",max:"number",precision:"number"}]),{prefix:(t.attr("prefix")?t.attr("prefix"):undefined)});
};
$.fn.numberbox.defaults=$.extend({},$.fn.textbox.defaults,{inputEvents:{keypress:function(e){
var _14=e.data.target;
var _15=$(_14).numberbox("options");
return _15.filter.call(_14,e);
},blur:function(e){
var _16=e.data.target;
$(_16).numberbox("setValue",$(_16).numberbox("getText"));
},keydown:function(e){
if(e.keyCode==13){
var _17=e.data.target;
$(_17).numberbox("setValue",$(_17).numberbox("getText"));
}
}},min:null,max:null,precision:0,decimalSeparator:".",groupSeparator:"",prefix:"",suffix:"",filter:function(e){
var _18=$(this).numberbox("options");
var s=$(this).numberbox("getText");
if(e.which==13){
return true;
}
if(e.which==45){
return (s.indexOf("-")==-1?true:false);
}
var c=String.fromCharCode(e.which);
if(c==_18.decimalSeparator){
return (s.indexOf(c)==-1?true:false);
}else{
if(c==_18.groupSeparator){
return true;
}else{
if((e.which>=48&&e.which<=57&&e.ctrlKey==false&&e.shiftKey==false)||e.which==0||e.which==8){
return true;
}else{
if(e.ctrlKey==true&&(e.which==99||e.which==118)){
return true;
}else{
return false;
}
}
}
}
},formatter:function(_19){
if(!_19){
return _19;
}
_19=_19+"";
var _1a=$(this).numberbox("options");
var s1=_19,s2="";
var _1b=_19.indexOf(".");
if(_1b>=0){
s1=_19.substring(0,_1b);
s2=_19.substring(_1b+1,_19.length);
}
if(_1a.groupSeparator){
var p=/(\d+)(\d{3})/;
while(p.test(s1)){
s1=s1.replace(p,"$1"+_1a.groupSeparator+"$2");
}
}
if(s2){
return _1a.prefix+s1+_1a.decimalSeparator+s2+_1a.suffix;
}else{
return _1a.prefix+s1+_1a.suffix;
}
},parser:function(s){
s=s+"";
var _1c=$(this).numberbox("options");
if(parseFloat(s)!=s){
if(_1c.prefix){
s=$.trim(s.replace(new RegExp("\\"+$.trim(_1c.prefix),"g"),""));
}
if(_1c.suffix){
s=$.trim(s.replace(new RegExp("\\"+$.trim(_1c.suffix),"g"),""));
}
if(_1c.groupSeparator){
s=$.trim(s.replace(new RegExp("\\"+_1c.groupSeparator,"g"),""));
}
if(_1c.decimalSeparator){
s=$.trim(s.replace(new RegExp("\\"+_1c.decimalSeparator,"g"),"."));
}
s=s.replace(/\s/g,"");
}
var val=parseFloat(s).toFixed(_1c.precision);
if(isNaN(val)){
val="";
}else{
if(typeof (_1c.min)=="number"&&val<_1c.min){
val=_1c.min.toFixed(_1c.precision);
}else{
if(typeof (_1c.max)=="number"&&val>_1c.max){
val=_1c.max.toFixed(_1c.precision);
}
}
}
return val;
}});
})(jQuery);

