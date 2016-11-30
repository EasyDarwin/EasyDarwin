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
function _2(_3,_4){
var _5=$.data(_3,"combobox");
var _6=_5.options;
var _7=_5.data;
for(var i=0;i<_7.length;i++){
if(_7[i][_6.valueField]==_4){
return i;
}
}
return -1;
};
function _8(_9,_a){
var _b=$.data(_9,"combobox").options;
var _c=$(_9).combo("panel");
var _d=_b.finder.getEl(_9,_a);
if(_d.length){
if(_d.position().top<=0){
var h=_c.scrollTop()+_d.position().top;
_c.scrollTop(h);
}else{
if(_d.position().top+_d.outerHeight()>_c.height()){
var h=_c.scrollTop()+_d.position().top+_d.outerHeight()-_c.height();
_c.scrollTop(h);
}
}
}
};
function _e(_f,dir){
var _10=$.data(_f,"combobox").options;
var _11=$(_f).combobox("panel");
var _12=_11.children("div.combobox-item-hover");
if(!_12.length){
_12=_11.children("div.combobox-item-selected");
}
_12.removeClass("combobox-item-hover");
var _13="div.combobox-item:visible:not(.combobox-item-disabled):first";
var _14="div.combobox-item:visible:not(.combobox-item-disabled):last";
if(!_12.length){
_12=_11.children(dir=="next"?_13:_14);
}else{
if(dir=="next"){
_12=_12.nextAll(_13);
if(!_12.length){
_12=_11.children(_13);
}
}else{
_12=_12.prevAll(_13);
if(!_12.length){
_12=_11.children(_14);
}
}
}
if(_12.length){
_12.addClass("combobox-item-hover");
var row=_10.finder.getRow(_f,_12);
if(row){
_8(_f,row[_10.valueField]);
if(_10.selectOnNavigation){
_15(_f,row[_10.valueField]);
}
}
}
};
function _15(_16,_17){
var _18=$.data(_16,"combobox").options;
var _19=$(_16).combo("getValues");
if($.inArray(_17+"",_19)==-1){
if(_18.multiple){
_19.push(_17);
}else{
_19=[_17];
}
_1a(_16,_19);
_18.onSelect.call(_16,_18.finder.getRow(_16,_17));
}
};
function _1b(_1c,_1d){
var _1e=$.data(_1c,"combobox").options;
var _1f=$(_1c).combo("getValues");
var _20=$.inArray(_1d+"",_1f);
if(_20>=0){
_1f.splice(_20,1);
_1a(_1c,_1f);
_1e.onUnselect.call(_1c,_1e.finder.getRow(_1c,_1d));
}
};
function _1a(_21,_22,_23){
var _24=$.data(_21,"combobox").options;
var _25=$(_21).combo("panel");
if(!$.isArray(_22)){
_22=_22.split(_24.separator);
}
_25.find("div.combobox-item-selected").removeClass("combobox-item-selected");
var vv=[],ss=[];
for(var i=0;i<_22.length;i++){
var v=_22[i];
var s=v;
_24.finder.getEl(_21,v).addClass("combobox-item-selected");
var row=_24.finder.getRow(_21,v);
if(row){
s=row[_24.textField];
}
vv.push(v);
ss.push(s);
}
if(!_23){
$(_21).combo("setText",ss.join(_24.separator));
}
$(_21).combo("setValues",vv);
};
function _26(_27,_28,_29){
var _2a=$.data(_27,"combobox");
var _2b=_2a.options;
_2a.data=_2b.loadFilter.call(_27,_28);
_2a.groups=[];
_28=_2a.data;
var _2c=$(_27).combobox("getValues");
var dd=[];
var _2d=undefined;
for(var i=0;i<_28.length;i++){
var row=_28[i];
var v=row[_2b.valueField]+"";
var s=row[_2b.textField];
var g=row[_2b.groupField];
if(g){
if(_2d!=g){
_2d=g;
_2a.groups.push(g);
dd.push("<div id=\""+(_2a.groupIdPrefix+"_"+(_2a.groups.length-1))+"\" class=\"combobox-group\">");
dd.push(_2b.groupFormatter?_2b.groupFormatter.call(_27,g):g);
dd.push("</div>");
}
}else{
_2d=undefined;
}
var cls="combobox-item"+(row.disabled?" combobox-item-disabled":"")+(g?" combobox-gitem":"");
dd.push("<div id=\""+(_2a.itemIdPrefix+"_"+i)+"\" class=\""+cls+"\">");
dd.push(_2b.formatter?_2b.formatter.call(_27,row):s);
dd.push("</div>");
if(row["selected"]&&$.inArray(v,_2c)==-1){
_2c.push(v);
}
}
$(_27).combo("panel").html(dd.join(""));
if(_2b.multiple){
_1a(_27,_2c,_29);
}else{
_1a(_27,_2c.length?[_2c[_2c.length-1]]:[],_29);
}
_2b.onLoadSuccess.call(_27,_28);
};
function _2e(_2f,url,_30,_31){
var _32=$.data(_2f,"combobox").options;
if(url){
_32.url=url;
}
_30=$.extend({},_32.queryParams,_30||{});
if(_32.onBeforeLoad.call(_2f,_30)==false){
return;
}
_32.loader.call(_2f,_30,function(_33){
_26(_2f,_33,_31);
},function(){
_32.onLoadError.apply(this,arguments);
});
};
function _34(_35,q){
var _36=$.data(_35,"combobox");
var _37=_36.options;
var qq=_37.multiple?q.split(_37.separator):[q];
if(_37.mode=="remote"){
_38(qq);
_2e(_35,null,{q:q},true);
}else{
var _39=$(_35).combo("panel");
_39.find("div.combobox-item-selected,div.combobox-item-hover").removeClass("combobox-item-selected combobox-item-hover");
_39.find("div.combobox-item,div.combobox-group").hide();
var _3a=_36.data;
var vv=[];
$.map(qq,function(q){
q=$.trim(q);
var _3b=q;
var _3c=undefined;
for(var i=0;i<_3a.length;i++){
var row=_3a[i];
if(_37.filter.call(_35,q,row)){
var v=row[_37.valueField];
var s=row[_37.textField];
var g=row[_37.groupField];
var _3d=_37.finder.getEl(_35,v).show();
if(s.toLowerCase()==q.toLowerCase()){
_3b=v;
_3d.addClass("combobox-item-selected");
_37.onSelect.call(_35,row);
}
if(_37.groupField&&_3c!=g){
$("#"+_36.groupIdPrefix+"_"+$.inArray(g,_36.groups)).show();
_3c=g;
}
}
}
vv.push(_3b);
});
_38(vv);
}
function _38(vv){
_1a(_35,_37.multiple?(q?vv:[]):vv,true);
};
};
function _3e(_3f){
var t=$(_3f);
var _40=t.combobox("options");
var _41=t.combobox("panel");
var _42=_41.children("div.combobox-item-hover");
if(_42.length){
var row=_40.finder.getRow(_3f,_42);
var _43=row[_40.valueField];
if(_40.multiple){
if(_42.hasClass("combobox-item-selected")){
t.combobox("unselect",_43);
}else{
t.combobox("select",_43);
}
}else{
t.combobox("select",_43);
}
}
var vv=[];
$.map(t.combobox("getValues"),function(v){
if(_2(_3f,v)>=0){
vv.push(v);
}
});
t.combobox("setValues",vv);
if(!_40.multiple){
t.combobox("hidePanel");
}
};
function _44(_45){
var _46=$.data(_45,"combobox");
var _47=_46.options;
_1++;
_46.itemIdPrefix="_easyui_combobox_i"+_1;
_46.groupIdPrefix="_easyui_combobox_g"+_1;
$(_45).addClass("combobox-f");
$(_45).combo($.extend({},_47,{onShowPanel:function(){
$(_45).combo("panel").find("div.combobox-item:hidden,div.combobox-group:hidden").show();
_8(_45,$(_45).combobox("getValue"));
_47.onShowPanel.call(_45);
}}));
$(_45).combo("panel").unbind().bind("mouseover",function(e){
$(this).children("div.combobox-item-hover").removeClass("combobox-item-hover");
var _48=$(e.target).closest("div.combobox-item");
if(!_48.hasClass("combobox-item-disabled")){
_48.addClass("combobox-item-hover");
}
e.stopPropagation();
}).bind("mouseout",function(e){
$(e.target).closest("div.combobox-item").removeClass("combobox-item-hover");
e.stopPropagation();
}).bind("click",function(e){
var _49=$(e.target).closest("div.combobox-item");
if(!_49.length||_49.hasClass("combobox-item-disabled")){
return;
}
var row=_47.finder.getRow(_45,_49);
if(!row){
return;
}
var _4a=row[_47.valueField];
if(_47.multiple){
if(_49.hasClass("combobox-item-selected")){
_1b(_45,_4a);
}else{
_15(_45,_4a);
}
}else{
_15(_45,_4a);
$(_45).combo("hidePanel");
}
e.stopPropagation();
});
};
$.fn.combobox=function(_4b,_4c){
if(typeof _4b=="string"){
var _4d=$.fn.combobox.methods[_4b];
if(_4d){
return _4d(this,_4c);
}else{
return this.combo(_4b,_4c);
}
}
_4b=_4b||{};
return this.each(function(){
var _4e=$.data(this,"combobox");
if(_4e){
$.extend(_4e.options,_4b);
}else{
_4e=$.data(this,"combobox",{options:$.extend({},$.fn.combobox.defaults,$.fn.combobox.parseOptions(this),_4b),data:[]});
}
_44(this);
if(_4e.options.data){
_26(this,_4e.options.data);
}else{
var _4f=$.fn.combobox.parseData(this);
if(_4f.length){
_26(this,_4f);
}
}
_2e(this);
});
};
$.fn.combobox.methods={options:function(jq){
var _50=jq.combo("options");
return $.extend($.data(jq[0],"combobox").options,{width:_50.width,height:_50.height,originalValue:_50.originalValue,disabled:_50.disabled,readonly:_50.readonly});
},getData:function(jq){
return $.data(jq[0],"combobox").data;
},setValues:function(jq,_51){
return jq.each(function(){
_1a(this,_51);
});
},setValue:function(jq,_52){
return jq.each(function(){
_1a(this,[_52]);
});
},clear:function(jq){
return jq.each(function(){
$(this).combo("clear");
var _53=$(this).combo("panel");
_53.find("div.combobox-item-selected").removeClass("combobox-item-selected");
});
},reset:function(jq){
return jq.each(function(){
var _54=$(this).combobox("options");
if(_54.multiple){
$(this).combobox("setValues",_54.originalValue);
}else{
$(this).combobox("setValue",_54.originalValue);
}
});
},loadData:function(jq,_55){
return jq.each(function(){
_26(this,_55);
});
},reload:function(jq,url){
return jq.each(function(){
if(typeof url=="string"){
_2e(this,url);
}else{
if(url){
var _56=$(this).combobox("options");
_56.queryParams=url;
}
_2e(this);
}
});
},select:function(jq,_57){
return jq.each(function(){
_15(this,_57);
});
},unselect:function(jq,_58){
return jq.each(function(){
_1b(this,_58);
});
}};
$.fn.combobox.parseOptions=function(_59){
var t=$(_59);
return $.extend({},$.fn.combo.parseOptions(_59),$.parser.parseOptions(_59,["valueField","textField","groupField","mode","method","url"]));
};
$.fn.combobox.parseData=function(_5a){
var _5b=[];
var _5c=$(_5a).combobox("options");
$(_5a).children().each(function(){
if(this.tagName.toLowerCase()=="optgroup"){
var _5d=$(this).attr("label");
$(this).children().each(function(){
_5e(this,_5d);
});
}else{
_5e(this);
}
});
return _5b;
function _5e(el,_5f){
var t=$(el);
var row={};
row[_5c.valueField]=t.attr("value")!=undefined?t.attr("value"):t.text();
row[_5c.textField]=t.text();
row["selected"]=t.is(":selected");
row["disabled"]=t.is(":disabled");
if(_5f){
_5c.groupField=_5c.groupField||"group";
row[_5c.groupField]=_5f;
}
_5b.push(row);
};
};
$.fn.combobox.defaults=$.extend({},$.fn.combo.defaults,{valueField:"value",textField:"text",groupField:null,groupFormatter:function(_60){
return _60;
},mode:"local",method:"post",url:null,data:null,queryParams:{},keyHandler:{up:function(e){
_e(this,"prev");
e.preventDefault();
},down:function(e){
_e(this,"next");
e.preventDefault();
},left:function(e){
},right:function(e){
},enter:function(e){
_3e(this);
},query:function(q,e){
_34(this,q);
}},filter:function(q,row){
var _61=$(this).combobox("options");
return row[_61.textField].toLowerCase().indexOf(q.toLowerCase())==0;
},formatter:function(row){
var _62=$(this).combobox("options");
return row[_62.textField];
},loader:function(_63,_64,_65){
var _66=$(this).combobox("options");
if(!_66.url){
return false;
}
$.ajax({type:_66.method,url:_66.url,data:_63,dataType:"json",success:function(_67){
_64(_67);
},error:function(){
_65.apply(this,arguments);
}});
},loadFilter:function(_68){
return _68;
},finder:{getEl:function(_69,_6a){
var _6b=_2(_69,_6a);
var id=$.data(_69,"combobox").itemIdPrefix+"_"+_6b;
return $("#"+id);
},getRow:function(_6c,p){
var _6d=$.data(_6c,"combobox");
var _6e=(p instanceof jQuery)?p.attr("id").substr(_6d.itemIdPrefix.length+1):_2(_6c,p);
return _6d.data[parseInt(_6e)];
}},onBeforeLoad:function(_6f){
},onLoadSuccess:function(){
},onLoadError:function(){
},onSelect:function(_70){
},onUnselect:function(_71){
}});
})(jQuery);

