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
var _3=$.data(_2,"spinner");
var _4=_3.options;
var _5=$.extend(true,[],_4.icons);
_5.push({iconCls:"spinner-arrow",handler:function(e){
_6(e);
}});
$(_2).addClass("spinner-f").textbox($.extend({},_4,{icons:_5}));
var _7=$(_2).textbox("getIcon",_5.length-1);
_7.append("<a href=\"javascript:void(0)\" class=\"spinner-arrow-up\" tabindex=\"-1\"></a>");
_7.append("<a href=\"javascript:void(0)\" class=\"spinner-arrow-down\" tabindex=\"-1\"></a>");
$(_2).attr("spinnerName",$(_2).attr("textboxName"));
_3.spinner=$(_2).next();
_3.spinner.addClass("spinner");
};
function _6(e){
var _8=e.data.target;
var _9=$(_8).spinner("options");
var up=$(e.target).closest("a.spinner-arrow-up");
if(up.length){
_9.spin.call(_8,false);
_9.onSpinUp.call(_8);
$(_8).spinner("validate");
}
var _a=$(e.target).closest("a.spinner-arrow-down");
if(_a.length){
_9.spin.call(_8,true);
_9.onSpinDown.call(_8);
$(_8).spinner("validate");
}
};
$.fn.spinner=function(_b,_c){
if(typeof _b=="string"){
var _d=$.fn.spinner.methods[_b];
if(_d){
return _d(this,_c);
}else{
return this.textbox(_b,_c);
}
}
_b=_b||{};
return this.each(function(){
var _e=$.data(this,"spinner");
if(_e){
$.extend(_e.options,_b);
}else{
_e=$.data(this,"spinner",{options:$.extend({},$.fn.spinner.defaults,$.fn.spinner.parseOptions(this),_b)});
}
_1(this);
});
};
$.fn.spinner.methods={options:function(jq){
var _f=jq.textbox("options");
return $.extend($.data(jq[0],"spinner").options,{width:_f.width,value:_f.value,originalValue:_f.originalValue,disabled:_f.disabled,readonly:_f.readonly});
}};
$.fn.spinner.parseOptions=function(_10){
return $.extend({},$.fn.textbox.parseOptions(_10),$.parser.parseOptions(_10,["min","max",{increment:"number"}]));
};
$.fn.spinner.defaults=$.extend({},$.fn.textbox.defaults,{min:null,max:null,increment:1,spin:function(_11){
},onSpinUp:function(){
},onSpinDown:function(){
}});
})(jQuery);

