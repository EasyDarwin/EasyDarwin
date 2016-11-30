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
var _3=$.data(_2,"datebox");
var _4=_3.options;
$(_2).addClass("datebox-f").combo($.extend({},_4,{onShowPanel:function(){
_5(this);
_6(this);
_7(this);
_18(this,$(this).datebox("getText"),true);
_4.onShowPanel.call(this);
}}));
if(!_3.calendar){
var _8=$(_2).combo("panel").css("overflow","hidden");
_8.panel("options").onBeforeDestroy=function(){
var c=$(this).find(".calendar-shared");
if(c.length){
c.insertBefore(c[0].pholder);
}
};
var cc=$("<div class=\"datebox-calendar-inner\"></div>").prependTo(_8);
if(_4.sharedCalendar){
var c=$(_4.sharedCalendar);
if(!c[0].pholder){
c[0].pholder=$("<div class=\"calendar-pholder\" style=\"display:none\"></div>").insertAfter(c);
}
c.addClass("calendar-shared").appendTo(cc);
if(!c.hasClass("calendar")){
c.calendar();
}
_3.calendar=c;
}else{
_3.calendar=$("<div></div>").appendTo(cc).calendar();
}
$.extend(_3.calendar.calendar("options"),{fit:true,border:false,onSelect:function(_9){
var _a=this.target;
var _b=$(_a).datebox("options");
_18(_a,_b.formatter.call(_a,_9));
$(_a).combo("hidePanel");
_b.onSelect.call(_a,_9);
}});
}
$(_2).combo("textbox").parent().addClass("datebox");
$(_2).datebox("initValue",_4.value);
function _5(_c){
var _d=$(_c).datebox("options");
var _e=$(_c).combo("panel");
_e.unbind(".datebox").bind("click.datebox",function(e){
if($(e.target).hasClass("datebox-button-a")){
var _f=parseInt($(e.target).attr("datebox-button-index"));
_d.buttons[_f].handler.call(e.target,_c);
}
});
};
function _6(_10){
var _11=$(_10).combo("panel");
if(_11.children("div.datebox-button").length){
return;
}
var _12=$("<div class=\"datebox-button\"><table cellspacing=\"0\" cellpadding=\"0\" style=\"width:100%\"><tr></tr></table></div>").appendTo(_11);
var tr=_12.find("tr");
for(var i=0;i<_4.buttons.length;i++){
var td=$("<td></td>").appendTo(tr);
var btn=_4.buttons[i];
var t=$("<a class=\"datebox-button-a\" href=\"javascript:void(0)\"></a>").html($.isFunction(btn.text)?btn.text(_10):btn.text).appendTo(td);
t.attr("datebox-button-index",i);
}
tr.find("td").css("width",(100/_4.buttons.length)+"%");
};
function _7(_13){
var _14=$(_13).combo("panel");
var cc=_14.children("div.datebox-calendar-inner");
_14.children()._outerWidth(_14.width());
_3.calendar.appendTo(cc);
_3.calendar[0].target=_13;
if(_4.panelHeight!="auto"){
var _15=_14.height();
_14.children().not(cc).each(function(){
_15-=$(this).outerHeight();
});
cc._outerHeight(_15);
}
_3.calendar.calendar("resize");
};
};
function _16(_17,q){
_18(_17,q,true);
};
function _19(_1a){
var _1b=$.data(_1a,"datebox");
var _1c=_1b.options;
var _1d=_1b.calendar.calendar("options").current;
if(_1d){
_18(_1a,_1c.formatter.call(_1a,_1d));
$(_1a).combo("hidePanel");
}
};
function _18(_1e,_1f,_20){
var _21=$.data(_1e,"datebox");
var _22=_21.options;
var _23=_21.calendar;
_23.calendar("moveTo",_22.parser.call(_1e,_1f));
if(_20){
$(_1e).combo("setValue",_1f);
}else{
if(_1f){
_1f=_22.formatter.call(_1e,_23.calendar("options").current);
}
$(_1e).combo("setText",_1f).combo("setValue",_1f);
}
};
$.fn.datebox=function(_24,_25){
if(typeof _24=="string"){
var _26=$.fn.datebox.methods[_24];
if(_26){
return _26(this,_25);
}else{
return this.combo(_24,_25);
}
}
_24=_24||{};
return this.each(function(){
var _27=$.data(this,"datebox");
if(_27){
$.extend(_27.options,_24);
}else{
$.data(this,"datebox",{options:$.extend({},$.fn.datebox.defaults,$.fn.datebox.parseOptions(this),_24)});
}
_1(this);
});
};
$.fn.datebox.methods={options:function(jq){
var _28=jq.combo("options");
return $.extend($.data(jq[0],"datebox").options,{width:_28.width,height:_28.height,originalValue:_28.originalValue,disabled:_28.disabled,readonly:_28.readonly});
},cloneFrom:function(jq,_29){
return jq.each(function(){
$(this).combo("cloneFrom",_29);
$.data(this,"datebox",{options:$.extend(true,{},$(_29).datebox("options")),calendar:$(_29).datebox("calendar")});
$(this).addClass("datebox-f");
});
},calendar:function(jq){
return $.data(jq[0],"datebox").calendar;
},initValue:function(jq,_2a){
return jq.each(function(){
var _2b=$(this).datebox("options");
var _2c=_2b.value;
if(_2c){
_2c=_2b.formatter.call(this,_2b.parser.call(this,_2c));
}
$(this).combo("initValue",_2c).combo("setText",_2c);
});
},setValue:function(jq,_2d){
return jq.each(function(){
_18(this,_2d);
});
},reset:function(jq){
return jq.each(function(){
var _2e=$(this).datebox("options");
$(this).datebox("setValue",_2e.originalValue);
});
}};
$.fn.datebox.parseOptions=function(_2f){
return $.extend({},$.fn.combo.parseOptions(_2f),$.parser.parseOptions(_2f,["sharedCalendar"]));
};
$.fn.datebox.defaults=$.extend({},$.fn.combo.defaults,{panelWidth:180,panelHeight:"auto",sharedCalendar:null,keyHandler:{up:function(e){
},down:function(e){
},left:function(e){
},right:function(e){
},enter:function(e){
_19(this);
},query:function(q,e){
_16(this,q);
}},currentText:"Today",closeText:"Close",okText:"Ok",buttons:[{text:function(_30){
return $(_30).datebox("options").currentText;
},handler:function(_31){
var now=new Date();
$(_31).datebox("calendar").calendar({year:now.getFullYear(),month:now.getMonth()+1,current:new Date(now.getFullYear(),now.getMonth(),now.getDate())});
_19(_31);
}},{text:function(_32){
return $(_32).datebox("options").closeText;
},handler:function(_33){
$(this).closest("div.combo-panel").panel("close");
}}],formatter:function(_34){
var y=_34.getFullYear();
var m=_34.getMonth()+1;
var d=_34.getDate();
return (m<10?("0"+m):m)+"/"+(d<10?("0"+d):d)+"/"+y;
},parser:function(s){
if(!s){
return new Date();
}
var ss=s.split("/");
var m=parseInt(ss[0],10);
var d=parseInt(ss[1],10);
var y=parseInt(ss[2],10);
if(!isNaN(y)&&!isNaN(m)&&!isNaN(d)){
return new Date(y,m-1,d);
}else{
return new Date();
}
},onSelect:function(_35){
}});
})(jQuery);

