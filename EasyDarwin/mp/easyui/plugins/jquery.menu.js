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
$(function(){
$(document).unbind(".menu").bind("mousedown.menu",function(e){
var m=$(e.target).closest("div.menu,div.combo-p");
if(m.length){
return;
}
$("body>div.menu-top:visible").not(".menu-inline").menu("hide");
_1($("body>div.menu:visible").not(".menu-inline"));
});
});
function _2(_3){
var _4=$.data(_3,"menu").options;
$(_3).addClass("menu-top");
_4.inline?$(_3).addClass("menu-inline"):$(_3).appendTo("body");
$(_3).bind("_resize",function(e,_5){
if($(this).hasClass("easyui-fluid")||_5){
$(_3).menu("resize",_3);
}
return false;
});
var _6=_7($(_3));
for(var i=0;i<_6.length;i++){
_8(_6[i]);
}
function _7(_9){
var _a=[];
_9.addClass("menu");
_a.push(_9);
if(!_9.hasClass("menu-content")){
_9.children("div").each(function(){
var _b=$(this).children("div");
if(_b.length){
_b.appendTo("body");
this.submenu=_b;
var mm=_7(_b);
_a=_a.concat(mm);
}
});
}
return _a;
};
function _8(_c){
var wh=$.parser.parseOptions(_c[0],["width","height"]);
_c[0].originalHeight=wh.height||0;
if(_c.hasClass("menu-content")){
_c[0].originalWidth=wh.width||_c._outerWidth();
}else{
_c[0].originalWidth=wh.width||0;
_c.children("div").each(function(){
var _d=$(this);
var _e=$.extend({},$.parser.parseOptions(this,["name","iconCls","href",{separator:"boolean"}]),{disabled:(_d.attr("disabled")?true:undefined)});
if(_e.separator){
_d.addClass("menu-sep");
}
if(!_d.hasClass("menu-sep")){
_d[0].itemName=_e.name||"";
_d[0].itemHref=_e.href||"";
var _f=_d.addClass("menu-item").html();
_d.empty().append($("<div class=\"menu-text\"></div>").html(_f));
if(_e.iconCls){
$("<div class=\"menu-icon\"></div>").addClass(_e.iconCls).appendTo(_d);
}
if(_e.disabled){
_10(_3,_d[0],true);
}
if(_d[0].submenu){
$("<div class=\"menu-rightarrow\"></div>").appendTo(_d);
}
_11(_3,_d);
}
});
$("<div class=\"menu-line\"></div>").prependTo(_c);
}
_12(_3,_c);
if(!_c.hasClass("menu-inline")){
_c.hide();
}
_13(_3,_c);
};
};
function _12(_14,_15){
var _16=$.data(_14,"menu").options;
var _17=_15.attr("style")||"";
_15.css({display:"block",left:-10000,height:"auto",overflow:"hidden"});
_15.find(".menu-item").each(function(){
$(this)._outerHeight(_16.itemHeight);
$(this).find(".menu-text").css({height:(_16.itemHeight-2)+"px",lineHeight:(_16.itemHeight-2)+"px"});
});
_15.removeClass("menu-noline").addClass(_16.noline?"menu-noline":"");
var _18=_15[0].originalWidth||"auto";
if(isNaN(parseInt(_18))){
_18=0;
_15.find("div.menu-text").each(function(){
if(_18<$(this)._outerWidth()){
_18=$(this)._outerWidth();
}
});
_18+=40;
}
var _19=_15.outerHeight();
var _1a=_15[0].originalHeight||"auto";
if(isNaN(parseInt(_1a))){
_1a=_19;
if(_15.hasClass("menu-top")&&_16.alignTo){
var at=$(_16.alignTo);
var h1=at.offset().top-$(document).scrollTop();
var h2=$(window)._outerHeight()+$(document).scrollTop()-at.offset().top-at._outerHeight();
_1a=Math.min(_1a,Math.max(h1,h2));
}else{
if(_1a>$(window)._outerHeight()){
_1a=$(window).height();
}
}
}
_15.attr("style",_17);
_15._size({fit:(_15[0]==_14?_16.fit:false),width:_18,minWidth:_16.minWidth,height:_1a});
_15.css("overflow",_15.outerHeight()<_19?"auto":"hidden");
_15.children("div.menu-line")._outerHeight(_19-2);
};
function _13(_1b,_1c){
if(_1c.hasClass("menu-inline")){
return;
}
var _1d=$.data(_1b,"menu");
_1c.unbind(".menu").bind("mouseenter.menu",function(){
if(_1d.timer){
clearTimeout(_1d.timer);
_1d.timer=null;
}
}).bind("mouseleave.menu",function(){
if(_1d.options.hideOnUnhover){
_1d.timer=setTimeout(function(){
_1e(_1b,$(_1b).hasClass("menu-inline"));
},_1d.options.duration);
}
});
};
function _11(_1f,_20){
if(!_20.hasClass("menu-item")){
return;
}
_20.unbind(".menu");
_20.bind("click.menu",function(){
if($(this).hasClass("menu-item-disabled")){
return;
}
if(!this.submenu){
_1e(_1f,$(_1f).hasClass("menu-inline"));
var _21=this.itemHref;
if(_21){
location.href=_21;
}
}
$(this).trigger("mouseenter");
var _22=$(_1f).menu("getItem",this);
$.data(_1f,"menu").options.onClick.call(_1f,_22);
}).bind("mouseenter.menu",function(e){
_20.siblings().each(function(){
if(this.submenu){
_1(this.submenu);
}
$(this).removeClass("menu-active");
});
_20.addClass("menu-active");
if($(this).hasClass("menu-item-disabled")){
_20.addClass("menu-active-disabled");
return;
}
var _23=_20[0].submenu;
if(_23){
$(_1f).menu("show",{menu:_23,parent:_20});
}
}).bind("mouseleave.menu",function(e){
_20.removeClass("menu-active menu-active-disabled");
var _24=_20[0].submenu;
if(_24){
if(e.pageX>=parseInt(_24.css("left"))){
_20.addClass("menu-active");
}else{
_1(_24);
}
}else{
_20.removeClass("menu-active");
}
});
};
function _1e(_25,_26){
var _27=$.data(_25,"menu");
if(_27){
if($(_25).is(":visible")){
_1($(_25));
if(_26){
$(_25).show();
}else{
_27.options.onHide.call(_25);
}
}
}
return false;
};
function _28(_29,_2a){
var _2b,top;
_2a=_2a||{};
var _2c=$(_2a.menu||_29);
$(_29).menu("resize",_2c[0]);
if(_2c.hasClass("menu-top")){
var _2d=$.data(_29,"menu").options;
$.extend(_2d,_2a);
_2b=_2d.left;
top=_2d.top;
if(_2d.alignTo){
var at=$(_2d.alignTo);
_2b=at.offset().left;
top=at.offset().top+at._outerHeight();
if(_2d.align=="right"){
_2b+=at.outerWidth()-_2c.outerWidth();
}
}
if(_2b+_2c.outerWidth()>$(window)._outerWidth()+$(document)._scrollLeft()){
_2b=$(window)._outerWidth()+$(document).scrollLeft()-_2c.outerWidth()-5;
}
if(_2b<0){
_2b=0;
}
top=_2e(top,_2d.alignTo);
}else{
var _2f=_2a.parent;
_2b=_2f.offset().left+_2f.outerWidth()-2;
if(_2b+_2c.outerWidth()+5>$(window)._outerWidth()+$(document).scrollLeft()){
_2b=_2f.offset().left-_2c.outerWidth()+2;
}
top=_2e(_2f.offset().top-3);
}
function _2e(top,_30){
if(top+_2c.outerHeight()>$(window)._outerHeight()+$(document).scrollTop()){
if(_30){
top=$(_30).offset().top-_2c._outerHeight();
}else{
top=$(window)._outerHeight()+$(document).scrollTop()-_2c.outerHeight();
}
}
if(top<0){
top=0;
}
return top;
};
_2c.css({left:_2b,top:top});
_2c.show(0,function(){
if(!_2c[0].shadow){
_2c[0].shadow=$("<div class=\"menu-shadow\"></div>").insertAfter(_2c);
}
_2c[0].shadow.css({display:(_2c.hasClass("menu-inline")?"none":"block"),zIndex:$.fn.menu.defaults.zIndex++,left:_2c.css("left"),top:_2c.css("top"),width:_2c.outerWidth(),height:_2c.outerHeight()});
_2c.css("z-index",$.fn.menu.defaults.zIndex++);
if(_2c.hasClass("menu-top")){
$.data(_2c[0],"menu").options.onShow.call(_2c[0]);
}
});
};
function _1(_31){
if(_31&&_31.length){
_32(_31);
_31.find("div.menu-item").each(function(){
if(this.submenu){
_1(this.submenu);
}
$(this).removeClass("menu-active");
});
}
function _32(m){
m.stop(true,true);
if(m[0].shadow){
m[0].shadow.hide();
}
m.hide();
};
};
function _33(_34,_35){
var _36=null;
var tmp=$("<div></div>");
function _37(_38){
_38.children("div.menu-item").each(function(){
var _39=$(_34).menu("getItem",this);
var s=tmp.empty().html(_39.text).text();
if(_35==$.trim(s)){
_36=_39;
}else{
if(this.submenu&&!_36){
_37(this.submenu);
}
}
});
};
_37($(_34));
tmp.remove();
return _36;
};
function _10(_3a,_3b,_3c){
var t=$(_3b);
if(!t.hasClass("menu-item")){
return;
}
if(_3c){
t.addClass("menu-item-disabled");
if(_3b.onclick){
_3b.onclick1=_3b.onclick;
_3b.onclick=null;
}
}else{
t.removeClass("menu-item-disabled");
if(_3b.onclick1){
_3b.onclick=_3b.onclick1;
_3b.onclick1=null;
}
}
};
function _3d(_3e,_3f){
var _40=$.data(_3e,"menu").options;
var _41=$(_3e);
if(_3f.parent){
if(!_3f.parent.submenu){
var _42=$("<div class=\"menu\"><div class=\"menu-line\"></div></div>").appendTo("body");
_42.hide();
_3f.parent.submenu=_42;
$("<div class=\"menu-rightarrow\"></div>").appendTo(_3f.parent);
}
_41=_3f.parent.submenu;
}
if(_3f.separator){
var _43=$("<div class=\"menu-sep\"></div>").appendTo(_41);
}else{
var _43=$("<div class=\"menu-item\"></div>").appendTo(_41);
$("<div class=\"menu-text\"></div>").html(_3f.text).appendTo(_43);
}
if(_3f.iconCls){
$("<div class=\"menu-icon\"></div>").addClass(_3f.iconCls).appendTo(_43);
}
if(_3f.id){
_43.attr("id",_3f.id);
}
if(_3f.name){
_43[0].itemName=_3f.name;
}
if(_3f.href){
_43[0].itemHref=_3f.href;
}
if(_3f.onclick){
if(typeof _3f.onclick=="string"){
_43.attr("onclick",_3f.onclick);
}else{
_43[0].onclick=eval(_3f.onclick);
}
}
if(_3f.handler){
_43[0].onclick=eval(_3f.handler);
}
if(_3f.disabled){
_10(_3e,_43[0],true);
}
_11(_3e,_43);
_13(_3e,_41);
_12(_3e,_41);
};
function _44(_45,_46){
function _47(el){
if(el.submenu){
el.submenu.children("div.menu-item").each(function(){
_47(this);
});
var _48=el.submenu[0].shadow;
if(_48){
_48.remove();
}
el.submenu.remove();
}
$(el).remove();
};
var _49=$(_46).parent();
_47(_46);
_12(_45,_49);
};
function _4a(_4b,_4c,_4d){
var _4e=$(_4c).parent();
if(_4d){
$(_4c).show();
}else{
$(_4c).hide();
}
_12(_4b,_4e);
};
function _4f(_50){
$(_50).children("div.menu-item").each(function(){
_44(_50,this);
});
if(_50.shadow){
_50.shadow.remove();
}
$(_50).remove();
};
$.fn.menu=function(_51,_52){
if(typeof _51=="string"){
return $.fn.menu.methods[_51](this,_52);
}
_51=_51||{};
return this.each(function(){
var _53=$.data(this,"menu");
if(_53){
$.extend(_53.options,_51);
}else{
_53=$.data(this,"menu",{options:$.extend({},$.fn.menu.defaults,$.fn.menu.parseOptions(this),_51)});
_2(this);
}
$(this).css({left:_53.options.left,top:_53.options.top});
});
};
$.fn.menu.methods={options:function(jq){
return $.data(jq[0],"menu").options;
},show:function(jq,pos){
return jq.each(function(){
_28(this,pos);
});
},hide:function(jq){
return jq.each(function(){
_1e(this);
});
},destroy:function(jq){
return jq.each(function(){
_4f(this);
});
},setText:function(jq,_54){
return jq.each(function(){
$(_54.target).children("div.menu-text").html(_54.text);
});
},setIcon:function(jq,_55){
return jq.each(function(){
$(_55.target).children("div.menu-icon").remove();
if(_55.iconCls){
$("<div class=\"menu-icon\"></div>").addClass(_55.iconCls).appendTo(_55.target);
}
});
},getItem:function(jq,_56){
var t=$(_56);
var _57={target:_56,id:t.attr("id"),text:$.trim(t.children("div.menu-text").html()),disabled:t.hasClass("menu-item-disabled"),name:_56.itemName,href:_56.itemHref,onclick:_56.onclick};
var _58=t.children("div.menu-icon");
if(_58.length){
var cc=[];
var aa=_58.attr("class").split(" ");
for(var i=0;i<aa.length;i++){
if(aa[i]!="menu-icon"){
cc.push(aa[i]);
}
}
_57.iconCls=cc.join(" ");
}
return _57;
},findItem:function(jq,_59){
return _33(jq[0],_59);
},appendItem:function(jq,_5a){
return jq.each(function(){
_3d(this,_5a);
});
},removeItem:function(jq,_5b){
return jq.each(function(){
_44(this,_5b);
});
},enableItem:function(jq,_5c){
return jq.each(function(){
_10(this,_5c,false);
});
},disableItem:function(jq,_5d){
return jq.each(function(){
_10(this,_5d,true);
});
},showItem:function(jq,_5e){
return jq.each(function(){
_4a(this,_5e,true);
});
},hideItem:function(jq,_5f){
return jq.each(function(){
_4a(this,_5f,false);
});
},resize:function(jq,_60){
return jq.each(function(){
_12(this,$(_60));
});
}};
$.fn.menu.parseOptions=function(_61){
return $.extend({},$.parser.parseOptions(_61,[{minWidth:"number",itemHeight:"number",duration:"number",hideOnUnhover:"boolean"},{fit:"boolean",inline:"boolean",noline:"boolean"}]));
};
$.fn.menu.defaults={zIndex:110000,left:0,top:0,alignTo:null,align:"left",minWidth:120,itemHeight:22,duration:100,hideOnUnhover:true,inline:false,fit:false,noline:false,onShow:function(){
},onHide:function(){
},onClick:function(_62){
}};
})(jQuery);

