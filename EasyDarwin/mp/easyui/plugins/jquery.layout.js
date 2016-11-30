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
var _1=false;
function _2(_3,_4){
var _5=$.data(_3,"layout");
var _6=_5.options;
var _7=_5.panels;
var cc=$(_3);
if(_4){
$.extend(_6,{width:_4.width,height:_4.height});
}
if(_3.tagName.toLowerCase()=="body"){
cc._size("fit");
}else{
cc._size(_6);
}
var _8={top:0,left:0,width:cc.width(),height:cc.height()};
_9(_a(_7.expandNorth)?_7.expandNorth:_7.north,"n");
_9(_a(_7.expandSouth)?_7.expandSouth:_7.south,"s");
_b(_a(_7.expandEast)?_7.expandEast:_7.east,"e");
_b(_a(_7.expandWest)?_7.expandWest:_7.west,"w");
_7.center.panel("resize",_8);
function _9(pp,_c){
if(!pp.length||!_a(pp)){
return;
}
var _d=pp.panel("options");
pp.panel("resize",{width:cc.width(),height:_d.height});
var _e=pp.panel("panel").outerHeight();
pp.panel("move",{left:0,top:(_c=="n"?0:cc.height()-_e)});
_8.height-=_e;
if(_c=="n"){
_8.top+=_e;
if(!_d.split&&_d.border){
_8.top--;
}
}
if(!_d.split&&_d.border){
_8.height++;
}
};
function _b(pp,_f){
if(!pp.length||!_a(pp)){
return;
}
var _10=pp.panel("options");
pp.panel("resize",{width:_10.width,height:_8.height});
var _11=pp.panel("panel").outerWidth();
pp.panel("move",{left:(_f=="e"?cc.width()-_11:0),top:_8.top});
_8.width-=_11;
if(_f=="w"){
_8.left+=_11;
if(!_10.split&&_10.border){
_8.left--;
}
}
if(!_10.split&&_10.border){
_8.width++;
}
};
};
function _12(_13){
var cc=$(_13);
cc.addClass("layout");
function _14(cc){
var _15=cc.layout("options");
var _16=_15.onAdd;
_15.onAdd=function(){
};
cc.children("div").each(function(){
var _17=$.fn.layout.parsePanelOptions(this);
if("north,south,east,west,center".indexOf(_17.region)>=0){
_19(_13,_17,this);
}
});
_15.onAdd=_16;
};
cc.children("form").length?_14(cc.children("form")):_14(cc);
cc.append("<div class=\"layout-split-proxy-h\"></div><div class=\"layout-split-proxy-v\"></div>");
cc.bind("_resize",function(e,_18){
if($(this).hasClass("easyui-fluid")||_18){
_2(_13);
}
return false;
});
};
function _19(_1a,_1b,el){
_1b.region=_1b.region||"center";
var _1c=$.data(_1a,"layout").panels;
var cc=$(_1a);
var dir=_1b.region;
if(_1c[dir].length){
return;
}
var pp=$(el);
if(!pp.length){
pp=$("<div></div>").appendTo(cc);
}
var _1d=$.extend({},$.fn.layout.paneldefaults,{width:(pp.length?parseInt(pp[0].style.width)||pp.outerWidth():"auto"),height:(pp.length?parseInt(pp[0].style.height)||pp.outerHeight():"auto"),doSize:false,collapsible:true,onOpen:function(){
var _1e=$(this).panel("header").children("div.panel-tool");
_1e.children("a.panel-tool-collapse").hide();
var _1f={north:"up",south:"down",east:"right",west:"left"};
if(!_1f[dir]){
return;
}
var _20="layout-button-"+_1f[dir];
var t=_1e.children("a."+_20);
if(!t.length){
t=$("<a href=\"javascript:void(0)\"></a>").addClass(_20).appendTo(_1e);
t.bind("click",{dir:dir},function(e){
_2d(_1a,e.data.dir);
return false;
});
}
$(this).panel("options").collapsible?t.show():t.hide();
}},_1b,{cls:((_1b.cls||"")+" layout-panel layout-panel-"+dir),bodyCls:((_1b.bodyCls||"")+" layout-body")});
pp.panel(_1d);
_1c[dir]=pp;
var _21={north:"s",south:"n",east:"w",west:"e"};
var _22=pp.panel("panel");
if(pp.panel("options").split){
_22.addClass("layout-split-"+dir);
}
_22.resizable($.extend({},{handles:(_21[dir]||""),disabled:(!pp.panel("options").split),onStartResize:function(e){
_1=true;
if(dir=="north"||dir=="south"){
var _23=$(">div.layout-split-proxy-v",_1a);
}else{
var _23=$(">div.layout-split-proxy-h",_1a);
}
var top=0,_24=0,_25=0,_26=0;
var pos={display:"block"};
if(dir=="north"){
pos.top=parseInt(_22.css("top"))+_22.outerHeight()-_23.height();
pos.left=parseInt(_22.css("left"));
pos.width=_22.outerWidth();
pos.height=_23.height();
}else{
if(dir=="south"){
pos.top=parseInt(_22.css("top"));
pos.left=parseInt(_22.css("left"));
pos.width=_22.outerWidth();
pos.height=_23.height();
}else{
if(dir=="east"){
pos.top=parseInt(_22.css("top"))||0;
pos.left=parseInt(_22.css("left"))||0;
pos.width=_23.width();
pos.height=_22.outerHeight();
}else{
if(dir=="west"){
pos.top=parseInt(_22.css("top"))||0;
pos.left=_22.outerWidth()-_23.width();
pos.width=_23.width();
pos.height=_22.outerHeight();
}
}
}
}
_23.css(pos);
$("<div class=\"layout-mask\"></div>").css({left:0,top:0,width:cc.width(),height:cc.height()}).appendTo(cc);
},onResize:function(e){
if(dir=="north"||dir=="south"){
var _27=$(">div.layout-split-proxy-v",_1a);
_27.css("top",e.pageY-$(_1a).offset().top-_27.height()/2);
}else{
var _27=$(">div.layout-split-proxy-h",_1a);
_27.css("left",e.pageX-$(_1a).offset().left-_27.width()/2);
}
return false;
},onStopResize:function(e){
cc.children("div.layout-split-proxy-v,div.layout-split-proxy-h").hide();
pp.panel("resize",e.data);
_2(_1a);
_1=false;
cc.find(">div.layout-mask").remove();
}},_1b));
cc.layout("options").onAdd.call(_1a,dir);
};
function _28(_29,_2a){
var _2b=$.data(_29,"layout").panels;
if(_2b[_2a].length){
_2b[_2a].panel("destroy");
_2b[_2a]=$();
var _2c="expand"+_2a.substring(0,1).toUpperCase()+_2a.substring(1);
if(_2b[_2c]){
_2b[_2c].panel("destroy");
_2b[_2c]=undefined;
}
$(_29).layout("options").onRemove.call(_29,_2a);
}
};
function _2d(_2e,_2f,_30){
if(_30==undefined){
_30="normal";
}
var _31=$.data(_2e,"layout").panels;
var p=_31[_2f];
var _32=p.panel("options");
if(_32.onBeforeCollapse.call(p)==false){
return;
}
var _33="expand"+_2f.substring(0,1).toUpperCase()+_2f.substring(1);
if(!_31[_33]){
_31[_33]=_34(_2f);
var ep=_31[_33].panel("panel");
if(!_32.expandMode){
ep.css("cursor","default");
}else{
ep.bind("click",function(){
if(_32.expandMode=="dock"){
_41(_2e,_2f);
}else{
p.panel("expand",false).panel("open");
var _35=_36();
p.panel("resize",_35.collapse);
p.panel("panel").animate(_35.expand,function(){
$(this).unbind(".layout").bind("mouseleave.layout",{region:_2f},function(e){
if(_1==true){
return;
}
if($("body>div.combo-p>div.combo-panel:visible").length){
return;
}
_2d(_2e,e.data.region);
});
$(_2e).layout("options").onExpand.call(_2e,_2f);
});
}
return false;
});
}
}
var _37=_36();
if(!_a(_31[_33])){
_31.center.panel("resize",_37.resizeC);
}
p.panel("panel").animate(_37.collapse,_30,function(){
p.panel("collapse",false).panel("close");
_31[_33].panel("open").panel("resize",_37.expandP);
$(this).unbind(".layout");
$(_2e).layout("options").onCollapse.call(_2e,_2f);
});
function _34(dir){
var _38={"east":"left","west":"right","north":"down","south":"up"};
var _39=(_32.region=="north"||_32.region=="south");
var _3a="layout-button-"+_38[dir];
var p=$("<div></div>").appendTo(_2e);
p.panel($.extend({},$.fn.layout.paneldefaults,{cls:("layout-expand layout-expand-"+dir),title:"&nbsp;",iconCls:(_32.hideCollapsedContent?null:_32.iconCls),closed:true,minWidth:0,minHeight:0,doSize:false,region:_32.region,collapsedSize:_32.collapsedSize,noheader:(!_39&&_32.hideExpandTool),tools:((_39&&_32.hideExpandTool)?null:[{iconCls:_3a,handler:function(){
_41(_2e,_2f);
return false;
}}])}));
if(!_32.hideCollapsedContent){
var _3b=typeof _32.collapsedContent=="function"?_32.collapsedContent.call(p[0],_32.title):_32.collapsedContent;
_39?p.panel("setTitle",_3b):p.html(_3b);
}
p.panel("panel").hover(function(){
$(this).addClass("layout-expand-over");
},function(){
$(this).removeClass("layout-expand-over");
});
return p;
};
function _36(){
var cc=$(_2e);
var _3c=_31.center.panel("options");
var _3d=_32.collapsedSize;
if(_2f=="east"){
var _3e=p.panel("panel")._outerWidth();
var _3f=_3c.width+_3e-_3d;
if(_32.split||!_32.border){
_3f++;
}
return {resizeC:{width:_3f},expand:{left:cc.width()-_3e},expandP:{top:_3c.top,left:cc.width()-_3d,width:_3d,height:_3c.height},collapse:{left:cc.width(),top:_3c.top,height:_3c.height}};
}else{
if(_2f=="west"){
var _3e=p.panel("panel")._outerWidth();
var _3f=_3c.width+_3e-_3d;
if(_32.split||!_32.border){
_3f++;
}
return {resizeC:{width:_3f,left:_3d-1},expand:{left:0},expandP:{left:0,top:_3c.top,width:_3d,height:_3c.height},collapse:{left:-_3e,top:_3c.top,height:_3c.height}};
}else{
if(_2f=="north"){
var _40=p.panel("panel")._outerHeight();
var hh=_3c.height;
if(!_a(_31.expandNorth)){
hh+=_40-_3d+((_32.split||!_32.border)?1:0);
}
_31.east.add(_31.west).add(_31.expandEast).add(_31.expandWest).panel("resize",{top:_3d-1,height:hh});
return {resizeC:{top:_3d-1,height:hh},expand:{top:0},expandP:{top:0,left:0,width:cc.width(),height:_3d},collapse:{top:-_40,width:cc.width()}};
}else{
if(_2f=="south"){
var _40=p.panel("panel")._outerHeight();
var hh=_3c.height;
if(!_a(_31.expandSouth)){
hh+=_40-_3d+((_32.split||!_32.border)?1:0);
}
_31.east.add(_31.west).add(_31.expandEast).add(_31.expandWest).panel("resize",{height:hh});
return {resizeC:{height:hh},expand:{top:cc.height()-_40},expandP:{top:cc.height()-_3d,left:0,width:cc.width(),height:_3d},collapse:{top:cc.height(),width:cc.width()}};
}
}
}
}
};
};
function _41(_42,_43){
var _44=$.data(_42,"layout").panels;
var p=_44[_43];
var _45=p.panel("options");
if(_45.onBeforeExpand.call(p)==false){
return;
}
var _46="expand"+_43.substring(0,1).toUpperCase()+_43.substring(1);
if(_44[_46]){
_44[_46].panel("close");
p.panel("panel").stop(true,true);
p.panel("expand",false).panel("open");
var _47=_48();
p.panel("resize",_47.collapse);
p.panel("panel").animate(_47.expand,function(){
_2(_42);
$(_42).layout("options").onExpand.call(_42,_43);
});
}
function _48(){
var cc=$(_42);
var _49=_44.center.panel("options");
if(_43=="east"&&_44.expandEast){
return {collapse:{left:cc.width(),top:_49.top,height:_49.height},expand:{left:cc.width()-p.panel("panel")._outerWidth()}};
}else{
if(_43=="west"&&_44.expandWest){
return {collapse:{left:-p.panel("panel")._outerWidth(),top:_49.top,height:_49.height},expand:{left:0}};
}else{
if(_43=="north"&&_44.expandNorth){
return {collapse:{top:-p.panel("panel")._outerHeight(),width:cc.width()},expand:{top:0}};
}else{
if(_43=="south"&&_44.expandSouth){
return {collapse:{top:cc.height(),width:cc.width()},expand:{top:cc.height()-p.panel("panel")._outerHeight()}};
}
}
}
}
};
};
function _a(pp){
if(!pp){
return false;
}
if(pp.length){
return pp.panel("panel").is(":visible");
}else{
return false;
}
};
function _4a(_4b){
var _4c=$.data(_4b,"layout");
var _4d=_4c.options;
var _4e=_4c.panels;
var _4f=_4d.onCollapse;
_4d.onCollapse=function(){
};
_50("east");
_50("west");
_50("north");
_50("south");
_4d.onCollapse=_4f;
function _50(_51){
var p=_4e[_51];
if(p.length&&p.panel("options").collapsed){
_2d(_4b,_51,0);
}
};
};
function _52(_53,_54,_55){
var p=$(_53).layout("panel",_54);
p.panel("options").split=_55;
var cls="layout-split-"+_54;
var _56=p.panel("panel").removeClass(cls);
if(_55){
_56.addClass(cls);
}
_56.resizable({disabled:(!_55)});
_2(_53);
};
$.fn.layout=function(_57,_58){
if(typeof _57=="string"){
return $.fn.layout.methods[_57](this,_58);
}
_57=_57||{};
return this.each(function(){
var _59=$.data(this,"layout");
if(_59){
$.extend(_59.options,_57);
}else{
var _5a=$.extend({},$.fn.layout.defaults,$.fn.layout.parseOptions(this),_57);
$.data(this,"layout",{options:_5a,panels:{center:$(),north:$(),south:$(),east:$(),west:$()}});
_12(this);
}
_2(this);
_4a(this);
});
};
$.fn.layout.methods={options:function(jq){
return $.data(jq[0],"layout").options;
},resize:function(jq,_5b){
return jq.each(function(){
_2(this,_5b);
});
},panel:function(jq,_5c){
return $.data(jq[0],"layout").panels[_5c];
},collapse:function(jq,_5d){
return jq.each(function(){
_2d(this,_5d);
});
},expand:function(jq,_5e){
return jq.each(function(){
_41(this,_5e);
});
},add:function(jq,_5f){
return jq.each(function(){
_19(this,_5f);
_2(this);
if($(this).layout("panel",_5f.region).panel("options").collapsed){
_2d(this,_5f.region,0);
}
});
},remove:function(jq,_60){
return jq.each(function(){
_28(this,_60);
_2(this);
});
},split:function(jq,_61){
return jq.each(function(){
_52(this,_61,true);
});
},unsplit:function(jq,_62){
return jq.each(function(){
_52(this,_62,false);
});
}};
$.fn.layout.parseOptions=function(_63){
return $.extend({},$.parser.parseOptions(_63,[{fit:"boolean"}]));
};
$.fn.layout.defaults={fit:false,onExpand:function(_64){
},onCollapse:function(_65){
},onAdd:function(_66){
},onRemove:function(_67){
}};
$.fn.layout.parsePanelOptions=function(_68){
var t=$(_68);
return $.extend({},$.fn.panel.parseOptions(_68),$.parser.parseOptions(_68,["region",{split:"boolean",collpasedSize:"number",minWidth:"number",minHeight:"number",maxWidth:"number",maxHeight:"number"}]));
};
$.fn.layout.paneldefaults=$.extend({},$.fn.panel.defaults,{region:null,split:false,collapsedSize:28,expandMode:"float",hideExpandTool:false,hideCollapsedContent:true,collapsedContent:function(_69){
var p=$(this);
var _6a=p.panel("options");
if(_6a.region=="north"||_6a.region=="south"){
return _69;
}
var _6b=_6a.collapsedSize-2;
var _6c=(_6b-16)/2;
_6c=_6b-_6c;
var cc=[];
if(_6a.iconCls){
cc.push("<div class=\"panel-icon "+_6a.iconCls+"\"></div>");
}
cc.push("<div class=\"panel-title layout-expand-title");
cc.push(_6a.iconCls?" layout-expand-with-icon":"");
cc.push("\" style=\"left:"+_6c+"px\">");
cc.push(_69);
cc.push("</div>");
return cc.join("");
},minWidth:10,minHeight:10,maxWidth:10000,maxHeight:10000});
})(jQuery);

