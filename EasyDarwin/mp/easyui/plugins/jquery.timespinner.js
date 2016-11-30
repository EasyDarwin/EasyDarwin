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
var _3=0;
if(typeof _2.selectionStart=="number"){
_3=_2.selectionStart;
}else{
if(_2.createTextRange){
var _4=_2.createTextRange();
var s=document.selection.createRange();
s.setEndPoint("StartToStart",_4);
_3=s.text.length;
}
}
return _3;
};
function _5(_6,_7,_8){
if(_6.setSelectionRange){
_6.setSelectionRange(_7,_8);
}else{
if(_6.createTextRange){
var _9=_6.createTextRange();
_9.collapse();
_9.moveEnd("character",_8);
_9.moveStart("character",_7);
_9.select();
}
}
};
function _a(_b){
var _c=$.data(_b,"timespinner").options;
$(_b).addClass("timespinner-f").spinner(_c);
var _d=_c.formatter.call(_b,_c.parser.call(_b,_c.value));
$(_b).timespinner("initValue",_d);
};
function _e(e){
var _f=e.data.target;
var _10=$.data(_f,"timespinner").options;
var _11=_1(this);
for(var i=0;i<_10.selections.length;i++){
var _12=_10.selections[i];
if(_11>=_12[0]&&_11<=_12[1]){
_13(_f,i);
return;
}
}
};
function _13(_14,_15){
var _16=$.data(_14,"timespinner").options;
if(_15!=undefined){
_16.highlight=_15;
}
var _17=_16.selections[_16.highlight];
if(_17){
var tb=$(_14).timespinner("textbox");
_5(tb[0],_17[0],_17[1]);
tb.focus();
}
};
function _18(_19,_1a){
var _1b=$.data(_19,"timespinner").options;
var _1a=_1b.parser.call(_19,_1a);
var _1c=_1b.formatter.call(_19,_1a);
$(_19).spinner("setValue",_1c);
};
function _1d(_1e,_1f){
var _20=$.data(_1e,"timespinner").options;
var s=$(_1e).timespinner("getValue");
var _21=_20.selections[_20.highlight];
var s1=s.substring(0,_21[0]);
var s2=s.substring(_21[0],_21[1]);
var s3=s.substring(_21[1]);
var v=s1+((parseInt(s2,10)||0)+_20.increment*(_1f?-1:1))+s3;
$(_1e).timespinner("setValue",v);
_13(_1e);
};
$.fn.timespinner=function(_22,_23){
if(typeof _22=="string"){
var _24=$.fn.timespinner.methods[_22];
if(_24){
return _24(this,_23);
}else{
return this.spinner(_22,_23);
}
}
_22=_22||{};
return this.each(function(){
var _25=$.data(this,"timespinner");
if(_25){
$.extend(_25.options,_22);
}else{
$.data(this,"timespinner",{options:$.extend({},$.fn.timespinner.defaults,$.fn.timespinner.parseOptions(this),_22)});
}
_a(this);
});
};
$.fn.timespinner.methods={options:function(jq){
var _26=jq.data("spinner")?jq.spinner("options"):{};
return $.extend($.data(jq[0],"timespinner").options,{width:_26.width,value:_26.value,originalValue:_26.originalValue,disabled:_26.disabled,readonly:_26.readonly});
},setValue:function(jq,_27){
return jq.each(function(){
_18(this,_27);
});
},getHours:function(jq){
var _28=$.data(jq[0],"timespinner").options;
var vv=jq.timespinner("getValue").split(_28.separator);
return parseInt(vv[0],10);
},getMinutes:function(jq){
var _29=$.data(jq[0],"timespinner").options;
var vv=jq.timespinner("getValue").split(_29.separator);
return parseInt(vv[1],10);
},getSeconds:function(jq){
var _2a=$.data(jq[0],"timespinner").options;
var vv=jq.timespinner("getValue").split(_2a.separator);
return parseInt(vv[2],10)||0;
}};
$.fn.timespinner.parseOptions=function(_2b){
return $.extend({},$.fn.spinner.parseOptions(_2b),$.parser.parseOptions(_2b,["separator",{showSeconds:"boolean",highlight:"number"}]));
};
$.fn.timespinner.defaults=$.extend({},$.fn.spinner.defaults,{inputEvents:$.extend({},$.fn.spinner.defaults.inputEvents,{click:function(e){
_e.call(this,e);
},blur:function(e){
var t=$(e.data.target);
t.timespinner("setValue",t.timespinner("getText"));
},keydown:function(e){
if(e.keyCode==13){
var t=$(e.data.target);
t.timespinner("setValue",t.timespinner("getText"));
}
}}),formatter:function(_2c){
if(!_2c){
return "";
}
var _2d=$(this).timespinner("options");
var tt=[_2e(_2c.getHours()),_2e(_2c.getMinutes())];
if(_2d.showSeconds){
tt.push(_2e(_2c.getSeconds()));
}
return tt.join(_2d.separator);
function _2e(_2f){
return (_2f<10?"0":"")+_2f;
};
},parser:function(s){
var _30=$(this).timespinner("options");
var _31=_32(s);
if(_31){
var min=_32(_30.min);
var max=_32(_30.max);
if(min&&min>_31){
_31=min;
}
if(max&&max<_31){
_31=max;
}
}
return _31;
function _32(s){
if(!s){
return null;
}
var tt=s.split(_30.separator);
return new Date(1900,0,0,parseInt(tt[0],10)||0,parseInt(tt[1],10)||0,parseInt(tt[2],10)||0);
};
},selections:[[0,2],[3,5],[6,8]],separator:":",showSeconds:false,highlight:0,spin:function(_33){
_1d(this,_33);
}});
})(jQuery);

