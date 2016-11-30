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
var _3=$.data(_2,"datetimespinner").options;
$(_2).addClass("datetimespinner-f").timespinner(_3);
};
$.fn.datetimespinner=function(_4,_5){
if(typeof _4=="string"){
var _6=$.fn.datetimespinner.methods[_4];
if(_6){
return _6(this,_5);
}else{
return this.timespinner(_4,_5);
}
}
_4=_4||{};
return this.each(function(){
var _7=$.data(this,"datetimespinner");
if(_7){
$.extend(_7.options,_4);
}else{
$.data(this,"datetimespinner",{options:$.extend({},$.fn.datetimespinner.defaults,$.fn.datetimespinner.parseOptions(this),_4)});
}
_1(this);
});
};
$.fn.datetimespinner.methods={options:function(jq){
var _8=jq.timespinner("options");
return $.extend($.data(jq[0],"datetimespinner").options,{width:_8.width,value:_8.value,originalValue:_8.originalValue,disabled:_8.disabled,readonly:_8.readonly});
}};
$.fn.datetimespinner.parseOptions=function(_9){
return $.extend({},$.fn.timespinner.parseOptions(_9),$.parser.parseOptions(_9,[]));
};
$.fn.datetimespinner.defaults=$.extend({},$.fn.timespinner.defaults,{formatter:function(_a){
if(!_a){
return "";
}
return $.fn.datebox.defaults.formatter.call(this,_a)+" "+$.fn.timespinner.defaults.formatter.call(this,_a);
},parser:function(s){
s=$.trim(s);
if(!s){
return null;
}
var dt=s.split(" ");
var _b=$.fn.datebox.defaults.parser.call(this,dt[0]);
if(dt.length<2){
return _b;
}
var _c=$.fn.timespinner.defaults.parser.call(this,dt[1]);
return new Date(_b.getFullYear(),_b.getMonth(),_b.getDate(),_c.getHours(),_c.getMinutes(),_c.getSeconds());
},selections:[[0,2],[3,5],[6,10],[11,13],[14,16],[17,19]]});
})(jQuery);

