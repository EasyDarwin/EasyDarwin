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
$(_2).addClass("numberspinner-f");
var _3=$.data(_2,"numberspinner").options;
$(_2).numberbox(_3).spinner(_3);
$(_2).numberbox("setValue",_3.value);
};
function _4(_5,_6){
var _7=$.data(_5,"numberspinner").options;
var v=parseFloat($(_5).numberbox("getValue")||_7.value)||0;
if(_6){
v-=_7.increment;
}else{
v+=_7.increment;
}
$(_5).numberbox("setValue",v);
};
$.fn.numberspinner=function(_8,_9){
if(typeof _8=="string"){
var _a=$.fn.numberspinner.methods[_8];
if(_a){
return _a(this,_9);
}else{
return this.numberbox(_8,_9);
}
}
_8=_8||{};
return this.each(function(){
var _b=$.data(this,"numberspinner");
if(_b){
$.extend(_b.options,_8);
}else{
$.data(this,"numberspinner",{options:$.extend({},$.fn.numberspinner.defaults,$.fn.numberspinner.parseOptions(this),_8)});
}
_1(this);
});
};
$.fn.numberspinner.methods={options:function(jq){
var _c=jq.numberbox("options");
return $.extend($.data(jq[0],"numberspinner").options,{width:_c.width,value:_c.value,originalValue:_c.originalValue,disabled:_c.disabled,readonly:_c.readonly});
}};
$.fn.numberspinner.parseOptions=function(_d){
return $.extend({},$.fn.spinner.parseOptions(_d),$.fn.numberbox.parseOptions(_d),{});
};
$.fn.numberspinner.defaults=$.extend({},$.fn.spinner.defaults,$.fn.numberbox.defaults,{spin:function(_e){
_4(this,_e);
}});
})(jQuery);

