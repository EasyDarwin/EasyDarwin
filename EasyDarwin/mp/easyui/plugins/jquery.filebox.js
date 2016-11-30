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
var _1=0;
function _2(_3){
var _4=$.data(_3,"filebox");
var _5=_4.options;
_5.fileboxId="filebox_file_id_"+(++_1);
$(_3).addClass("filebox-f").textbox(_5);
$(_3).textbox("textbox").attr("readonly","readonly");
_4.filebox=$(_3).next().addClass("filebox");
var _6=_7(_3);
var _8=$(_3).filebox("button");
if(_8.length){
$("<label class=\"filebox-label\" for=\""+_5.fileboxId+"\"></label>").appendTo(_8);
if(_8.linkbutton("options").disabled){
_6.attr("disabled","disabled");
}else{
_6.removeAttr("disabled");
}
}
};
function _7(_9){
var _a=$.data(_9,"filebox");
var _b=_a.options;
_a.filebox.find(".textbox-value").remove();
_b.oldValue="";
var _c=$("<input type=\"file\" class=\"textbox-value\">").appendTo(_a.filebox);
_c.attr("id",_b.fileboxId).attr("name",$(_9).attr("textboxName")||"");
_c.change(function(){
$(_9).filebox("setText",this.value);
_b.onChange.call(_9,this.value,_b.oldValue);
_b.oldValue=this.value;
});
return _c;
};
$.fn.filebox=function(_d,_e){
if(typeof _d=="string"){
var _f=$.fn.filebox.methods[_d];
if(_f){
return _f(this,_e);
}else{
return this.textbox(_d,_e);
}
}
_d=_d||{};
return this.each(function(){
var _10=$.data(this,"filebox");
if(_10){
$.extend(_10.options,_d);
}else{
$.data(this,"filebox",{options:$.extend({},$.fn.filebox.defaults,$.fn.filebox.parseOptions(this),_d)});
}
_2(this);
});
};
$.fn.filebox.methods={options:function(jq){
var _11=jq.textbox("options");
return $.extend($.data(jq[0],"filebox").options,{width:_11.width,value:_11.value,originalValue:_11.originalValue,disabled:_11.disabled,readonly:_11.readonly});
},clear:function(jq){
return jq.each(function(){
$(this).textbox("clear");
_7(this);
});
},reset:function(jq){
return jq.each(function(){
$(this).filebox("clear");
});
}};
$.fn.filebox.parseOptions=function(_12){
return $.extend({},$.fn.textbox.parseOptions(_12),{});
};
$.fn.filebox.defaults=$.extend({},$.fn.textbox.defaults,{buttonIcon:null,buttonText:"Choose File",buttonAlign:"right",inputEvents:{}});
})(jQuery);

