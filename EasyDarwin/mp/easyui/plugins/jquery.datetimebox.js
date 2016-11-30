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
var _3=$.data(_2,"datetimebox");
var _4=_3.options;
$(_2).datebox($.extend({},_4,{onShowPanel:function(){
var _5=$(this).datetimebox("getValue");
_d(this,_5,true);
_4.onShowPanel.call(this);
},formatter:$.fn.datebox.defaults.formatter,parser:$.fn.datebox.defaults.parser}));
$(_2).removeClass("datebox-f").addClass("datetimebox-f");
$(_2).datebox("calendar").calendar({onSelect:function(_6){
_4.onSelect.call(this.target,_6);
}});
if(!_3.spinner){
var _7=$(_2).datebox("panel");
var p=$("<div style=\"padding:2px\"><input></div>").insertAfter(_7.children("div.datebox-calendar-inner"));
_3.spinner=p.children("input");
}
_3.spinner.timespinner({width:_4.spinnerWidth,showSeconds:_4.showSeconds,separator:_4.timeSeparator});
$(_2).datetimebox("initValue",_4.value);
};
function _8(_9){
var c=$(_9).datetimebox("calendar");
var t=$(_9).datetimebox("spinner");
var _a=c.calendar("options").current;
return new Date(_a.getFullYear(),_a.getMonth(),_a.getDate(),t.timespinner("getHours"),t.timespinner("getMinutes"),t.timespinner("getSeconds"));
};
function _b(_c,q){
_d(_c,q,true);
};
function _e(_f){
var _10=$.data(_f,"datetimebox").options;
var _11=_8(_f);
_d(_f,_10.formatter.call(_f,_11));
$(_f).combo("hidePanel");
};
function _d(_12,_13,_14){
var _15=$.data(_12,"datetimebox").options;
$(_12).combo("setValue",_13);
if(!_14){
if(_13){
var _16=_15.parser.call(_12,_13);
$(_12).combo("setText",_15.formatter.call(_12,_16));
$(_12).combo("setValue",_15.formatter.call(_12,_16));
}else{
$(_12).combo("setText",_13);
}
}
var _16=_15.parser.call(_12,_13);
$(_12).datetimebox("calendar").calendar("moveTo",_16);
$(_12).datetimebox("spinner").timespinner("setValue",_17(_16));
function _17(_18){
function _19(_1a){
return (_1a<10?"0":"")+_1a;
};
var tt=[_19(_18.getHours()),_19(_18.getMinutes())];
if(_15.showSeconds){
tt.push(_19(_18.getSeconds()));
}
return tt.join($(_12).datetimebox("spinner").timespinner("options").separator);
};
};
$.fn.datetimebox=function(_1b,_1c){
if(typeof _1b=="string"){
var _1d=$.fn.datetimebox.methods[_1b];
if(_1d){
return _1d(this,_1c);
}else{
return this.datebox(_1b,_1c);
}
}
_1b=_1b||{};
return this.each(function(){
var _1e=$.data(this,"datetimebox");
if(_1e){
$.extend(_1e.options,_1b);
}else{
$.data(this,"datetimebox",{options:$.extend({},$.fn.datetimebox.defaults,$.fn.datetimebox.parseOptions(this),_1b)});
}
_1(this);
});
};
$.fn.datetimebox.methods={options:function(jq){
var _1f=jq.datebox("options");
return $.extend($.data(jq[0],"datetimebox").options,{originalValue:_1f.originalValue,disabled:_1f.disabled,readonly:_1f.readonly});
},cloneFrom:function(jq,_20){
return jq.each(function(){
$(this).datebox("cloneFrom",_20);
$.data(this,"datetimebox",{options:$.extend(true,{},$(_20).datetimebox("options")),spinner:$(_20).datetimebox("spinner")});
$(this).removeClass("datebox-f").addClass("datetimebox-f");
});
},spinner:function(jq){
return $.data(jq[0],"datetimebox").spinner;
},initValue:function(jq,_21){
return jq.each(function(){
var _22=$(this).datetimebox("options");
var _23=_22.value;
if(_23){
_23=_22.formatter.call(this,_22.parser.call(this,_23));
}
$(this).combo("initValue",_23).combo("setText",_23);
});
},setValue:function(jq,_24){
return jq.each(function(){
_d(this,_24);
});
},reset:function(jq){
return jq.each(function(){
var _25=$(this).datetimebox("options");
$(this).datetimebox("setValue",_25.originalValue);
});
}};
$.fn.datetimebox.parseOptions=function(_26){
var t=$(_26);
return $.extend({},$.fn.datebox.parseOptions(_26),$.parser.parseOptions(_26,["timeSeparator","spinnerWidth",{showSeconds:"boolean"}]));
};
$.fn.datetimebox.defaults=$.extend({},$.fn.datebox.defaults,{spinnerWidth:"100%",showSeconds:true,timeSeparator:":",keyHandler:{up:function(e){
},down:function(e){
},left:function(e){
},right:function(e){
},enter:function(e){
_e(this);
},query:function(q,e){
_b(this,q);
}},buttons:[{text:function(_27){
return $(_27).datetimebox("options").currentText;
},handler:function(_28){
var _29=$(_28).datetimebox("options");
_d(_28,_29.formatter.call(_28,new Date()));
$(_28).datetimebox("hidePanel");
}},{text:function(_2a){
return $(_2a).datetimebox("options").okText;
},handler:function(_2b){
_e(_2b);
}},{text:function(_2c){
return $(_2c).datetimebox("options").closeText;
},handler:function(_2d){
$(_2d).datetimebox("hidePanel");
}}],formatter:function(_2e){
var h=_2e.getHours();
var M=_2e.getMinutes();
var s=_2e.getSeconds();
function _2f(_30){
return (_30<10?"0":"")+_30;
};
var _31=$(this).datetimebox("spinner").timespinner("options").separator;
var r=$.fn.datebox.defaults.formatter(_2e)+" "+_2f(h)+_31+_2f(M);
if($(this).datetimebox("options").showSeconds){
r+=_31+_2f(s);
}
return r;
},parser:function(s){
if($.trim(s)==""){
return new Date();
}
var dt=s.split(" ");
var d=$.fn.datebox.defaults.parser(dt[0]);
if(dt.length<2){
return d;
}
var _32=$(this).datetimebox("spinner").timespinner("options").separator;
var tt=dt[1].split(_32);
var _33=parseInt(tt[0],10)||0;
var _34=parseInt(tt[1],10)||0;
var _35=parseInt(tt[2],10)||0;
return new Date(d.getFullYear(),d.getMonth(),d.getDate(),_33,_34,_35);
}});
})(jQuery);

