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
var _3=$.data(_2,"searchbox");
var _4=_3.options;
var _5=$.extend(true,[],_4.icons);
_5.push({iconCls:"searchbox-button",handler:function(e){
var t=$(e.data.target);
var _6=t.searchbox("options");
_6.searcher.call(e.data.target,t.searchbox("getValue"),t.searchbox("getName"));
}});
_7();
var _8=_9();
$(_2).addClass("searchbox-f").textbox($.extend({},_4,{icons:_5,buttonText:(_8?_8.text:"")}));
$(_2).attr("searchboxName",$(_2).attr("textboxName"));
_3.searchbox=$(_2).next();
_3.searchbox.addClass("searchbox");
_a(_8);
function _7(){
if(_4.menu){
_3.menu=$(_4.menu).menu();
var _b=_3.menu.menu("options");
var _c=_b.onClick;
_b.onClick=function(_d){
_a(_d);
_c.call(this,_d);
};
}else{
if(_3.menu){
_3.menu.menu("destroy");
}
_3.menu=null;
}
};
function _9(){
if(_3.menu){
var _e=_3.menu.children("div.menu-item:first");
_3.menu.children("div.menu-item").each(function(){
var _f=$.extend({},$.parser.parseOptions(this),{selected:($(this).attr("selected")?true:undefined)});
if(_f.selected){
_e=$(this);
return false;
}
});
return _3.menu.menu("getItem",_e[0]);
}else{
return null;
}
};
function _a(_10){
if(!_10){
return;
}
$(_2).textbox("button").menubutton({text:_10.text,iconCls:(_10.iconCls||null),menu:_3.menu,menuAlign:_4.buttonAlign,plain:false});
_3.searchbox.find("input.textbox-value").attr("name",_10.name||_10.text);
$(_2).searchbox("resize");
};
};
$.fn.searchbox=function(_11,_12){
if(typeof _11=="string"){
var _13=$.fn.searchbox.methods[_11];
if(_13){
return _13(this,_12);
}else{
return this.textbox(_11,_12);
}
}
_11=_11||{};
return this.each(function(){
var _14=$.data(this,"searchbox");
if(_14){
$.extend(_14.options,_11);
}else{
$.data(this,"searchbox",{options:$.extend({},$.fn.searchbox.defaults,$.fn.searchbox.parseOptions(this),_11)});
}
_1(this);
});
};
$.fn.searchbox.methods={options:function(jq){
var _15=jq.textbox("options");
return $.extend($.data(jq[0],"searchbox").options,{width:_15.width,value:_15.value,originalValue:_15.originalValue,disabled:_15.disabled,readonly:_15.readonly});
},menu:function(jq){
return $.data(jq[0],"searchbox").menu;
},getName:function(jq){
return $.data(jq[0],"searchbox").searchbox.find("input.textbox-value").attr("name");
},selectName:function(jq,_16){
return jq.each(function(){
var _17=$.data(this,"searchbox").menu;
if(_17){
_17.children("div.menu-item").each(function(){
var _18=_17.menu("getItem",this);
if(_18.name==_16){
$(this).triggerHandler("click");
return false;
}
});
}
});
},destroy:function(jq){
return jq.each(function(){
var _19=$(this).searchbox("menu");
if(_19){
_19.menu("destroy");
}
$(this).textbox("destroy");
});
}};
$.fn.searchbox.parseOptions=function(_1a){
var t=$(_1a);
return $.extend({},$.fn.textbox.parseOptions(_1a),$.parser.parseOptions(_1a,["menu"]),{searcher:(t.attr("searcher")?eval(t.attr("searcher")):undefined)});
};
$.fn.searchbox.defaults=$.extend({},$.fn.textbox.defaults,{inputEvents:$.extend({},$.fn.textbox.defaults.inputEvents,{keydown:function(e){
if(e.keyCode==13){
e.preventDefault();
var t=$(e.data.target);
var _1b=t.searchbox("options");
t.searchbox("setValue",$(this).val());
_1b.searcher.call(e.data.target,t.searchbox("getValue"),t.searchbox("getName"));
return false;
}
}}),buttonAlign:"left",menu:null,searcher:function(_1c,_1d){
}});
})(jQuery);

