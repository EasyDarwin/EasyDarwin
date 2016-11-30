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
function _1(c){
var w=0;
$(c).children().each(function(){
w+=$(this).outerWidth(true);
});
return w;
};
function _2(_3){
var _4=$.data(_3,"tabs").options;
if(_4.tabPosition=="left"||_4.tabPosition=="right"||!_4.showHeader){
return;
}
var _5=$(_3).children("div.tabs-header");
var _6=_5.children("div.tabs-tool:not(.tabs-tool-hidden)");
var _7=_5.children("div.tabs-scroller-left");
var _8=_5.children("div.tabs-scroller-right");
var _9=_5.children("div.tabs-wrap");
var _a=_5.outerHeight();
if(_4.plain){
_a-=_a-_5.height();
}
_6._outerHeight(_a);
var _b=_1(_5.find("ul.tabs"));
var _c=_5.width()-_6._outerWidth();
if(_b>_c){
_7.add(_8).show()._outerHeight(_a);
if(_4.toolPosition=="left"){
_6.css({left:_7.outerWidth(),right:""});
_9.css({marginLeft:_7.outerWidth()+_6._outerWidth(),marginRight:_8._outerWidth(),width:_c-_7.outerWidth()-_8.outerWidth()});
}else{
_6.css({left:"",right:_8.outerWidth()});
_9.css({marginLeft:_7.outerWidth(),marginRight:_8.outerWidth()+_6._outerWidth(),width:_c-_7.outerWidth()-_8.outerWidth()});
}
}else{
_7.add(_8).hide();
if(_4.toolPosition=="left"){
_6.css({left:0,right:""});
_9.css({marginLeft:_6._outerWidth(),marginRight:0,width:_c});
}else{
_6.css({left:"",right:0});
_9.css({marginLeft:0,marginRight:_6._outerWidth(),width:_c});
}
}
};
function _d(_e){
var _f=$.data(_e,"tabs").options;
var _10=$(_e).children("div.tabs-header");
if(_f.tools){
if(typeof _f.tools=="string"){
$(_f.tools).addClass("tabs-tool").appendTo(_10);
$(_f.tools).show();
}else{
_10.children("div.tabs-tool").remove();
var _11=$("<div class=\"tabs-tool\"><table cellspacing=\"0\" cellpadding=\"0\" style=\"height:100%\"><tr></tr></table></div>").appendTo(_10);
var tr=_11.find("tr");
for(var i=0;i<_f.tools.length;i++){
var td=$("<td></td>").appendTo(tr);
var _12=$("<a href=\"javascript:void(0);\"></a>").appendTo(td);
_12[0].onclick=eval(_f.tools[i].handler||function(){
});
_12.linkbutton($.extend({},_f.tools[i],{plain:true}));
}
}
}else{
_10.children("div.tabs-tool").remove();
}
};
function _13(_14,_15){
var _16=$.data(_14,"tabs");
var _17=_16.options;
var cc=$(_14);
if(!_17.doSize){
return;
}
if(_15){
$.extend(_17,{width:_15.width,height:_15.height});
}
cc._size(_17);
var _18=cc.children("div.tabs-header");
var _19=cc.children("div.tabs-panels");
var _1a=_18.find("div.tabs-wrap");
var ul=_1a.find(".tabs");
ul.children("li").removeClass("tabs-first tabs-last");
ul.children("li:first").addClass("tabs-first");
ul.children("li:last").addClass("tabs-last");
if(_17.tabPosition=="left"||_17.tabPosition=="right"){
_18._outerWidth(_17.showHeader?_17.headerWidth:0);
_19._outerWidth(cc.width()-_18.outerWidth());
_18.add(_19)._size("height",isNaN(parseInt(_17.height))?"":cc.height());
_1a._outerWidth(_18.width());
ul._outerWidth(_1a.width()).css("height","");
}else{
_18.children("div.tabs-scroller-left,div.tabs-scroller-right,div.tabs-tool:not(.tabs-tool-hidden)").css("display",_17.showHeader?"block":"none");
_18._outerWidth(cc.width()).css("height","");
if(_17.showHeader){
_18.css("background-color","");
_1a.css("height","");
}else{
_18.css("background-color","transparent");
_18._outerHeight(0);
_1a._outerHeight(0);
}
ul._outerHeight(_17.tabHeight).css("width","");
ul._outerHeight(ul.outerHeight()-ul.height()-1+_17.tabHeight).css("width","");
_19._size("height",isNaN(parseInt(_17.height))?"":(cc.height()-_18.outerHeight()));
_19._size("width",cc.width());
}
if(_16.tabs.length){
var d1=ul.outerWidth(true)-ul.width();
var li=ul.children("li:first");
var d2=li.outerWidth(true)-li.width();
var _1b=_18.width()-_18.children(".tabs-tool:not(.tabs-tool-hidden)")._outerWidth();
var _1c=Math.floor((_1b-d1-d2*_16.tabs.length)/_16.tabs.length);
$.map(_16.tabs,function(p){
_1d(p,(_17.justified&&$.inArray(_17.tabPosition,["top","bottom"])>=0)?_1c:undefined);
});
if(_17.justified&&$.inArray(_17.tabPosition,["top","bottom"])>=0){
var _1e=_1b-d1-_1(ul);
_1d(_16.tabs[_16.tabs.length-1],_1c+_1e);
}
}
_2(_14);
function _1d(p,_1f){
var _20=p.panel("options");
var p_t=_20.tab.find("a.tabs-inner");
var _1f=_1f?_1f:(parseInt(_20.tabWidth||_17.tabWidth||undefined));
if(_1f){
p_t._outerWidth(_1f);
}else{
p_t.css("width","");
}
p_t._outerHeight(_17.tabHeight);
p_t.css("lineHeight",p_t.height()+"px");
p_t.find(".easyui-fluid:visible").triggerHandler("_resize");
};
};
function _21(_22){
var _23=$.data(_22,"tabs").options;
var tab=_24(_22);
if(tab){
var _25=$(_22).children("div.tabs-panels");
var _26=_23.width=="auto"?"auto":_25.width();
var _27=_23.height=="auto"?"auto":_25.height();
tab.panel("resize",{width:_26,height:_27});
}
};
function _28(_29){
var _2a=$.data(_29,"tabs").tabs;
var cc=$(_29).addClass("tabs-container");
var _2b=$("<div class=\"tabs-panels\"></div>").insertBefore(cc);
cc.children("div").each(function(){
_2b[0].appendChild(this);
});
cc[0].appendChild(_2b[0]);
$("<div class=\"tabs-header\">"+"<div class=\"tabs-scroller-left\"></div>"+"<div class=\"tabs-scroller-right\"></div>"+"<div class=\"tabs-wrap\">"+"<ul class=\"tabs\"></ul>"+"</div>"+"</div>").prependTo(_29);
cc.children("div.tabs-panels").children("div").each(function(i){
var _2c=$.extend({},$.parser.parseOptions(this),{disabled:($(this).attr("disabled")?true:undefined),selected:($(this).attr("selected")?true:undefined)});
_3c(_29,_2c,$(this));
});
cc.children("div.tabs-header").find(".tabs-scroller-left, .tabs-scroller-right").hover(function(){
$(this).addClass("tabs-scroller-over");
},function(){
$(this).removeClass("tabs-scroller-over");
});
cc.bind("_resize",function(e,_2d){
if($(this).hasClass("easyui-fluid")||_2d){
_13(_29);
_21(_29);
}
return false;
});
};
function _2e(_2f){
var _30=$.data(_2f,"tabs");
var _31=_30.options;
$(_2f).children("div.tabs-header").unbind().bind("click",function(e){
if($(e.target).hasClass("tabs-scroller-left")){
$(_2f).tabs("scrollBy",-_31.scrollIncrement);
}else{
if($(e.target).hasClass("tabs-scroller-right")){
$(_2f).tabs("scrollBy",_31.scrollIncrement);
}else{
var li=$(e.target).closest("li");
if(li.hasClass("tabs-disabled")){
return false;
}
var a=$(e.target).closest("a.tabs-close");
if(a.length){
_5a(_2f,_32(li));
}else{
if(li.length){
var _33=_32(li);
var _34=_30.tabs[_33].panel("options");
if(_34.collapsible){
_34.closed?_50(_2f,_33):_75(_2f,_33);
}else{
_50(_2f,_33);
}
}
}
return false;
}
}
}).bind("contextmenu",function(e){
var li=$(e.target).closest("li");
if(li.hasClass("tabs-disabled")){
return;
}
if(li.length){
_31.onContextMenu.call(_2f,e,li.find("span.tabs-title").html(),_32(li));
}
});
function _32(li){
var _35=0;
li.parent().children("li").each(function(i){
if(li[0]==this){
_35=i;
return false;
}
});
return _35;
};
};
function _36(_37){
var _38=$.data(_37,"tabs").options;
var _39=$(_37).children("div.tabs-header");
var _3a=$(_37).children("div.tabs-panels");
_39.removeClass("tabs-header-top tabs-header-bottom tabs-header-left tabs-header-right");
_3a.removeClass("tabs-panels-top tabs-panels-bottom tabs-panels-left tabs-panels-right");
if(_38.tabPosition=="top"){
_39.insertBefore(_3a);
}else{
if(_38.tabPosition=="bottom"){
_39.insertAfter(_3a);
_39.addClass("tabs-header-bottom");
_3a.addClass("tabs-panels-top");
}else{
if(_38.tabPosition=="left"){
_39.addClass("tabs-header-left");
_3a.addClass("tabs-panels-right");
}else{
if(_38.tabPosition=="right"){
_39.addClass("tabs-header-right");
_3a.addClass("tabs-panels-left");
}
}
}
}
if(_38.plain==true){
_39.addClass("tabs-header-plain");
}else{
_39.removeClass("tabs-header-plain");
}
_39.removeClass("tabs-header-narrow").addClass(_38.narrow?"tabs-header-narrow":"");
var _3b=_39.find(".tabs");
_3b.removeClass("tabs-pill").addClass(_38.pill?"tabs-pill":"");
_3b.removeClass("tabs-narrow").addClass(_38.narrow?"tabs-narrow":"");
_3b.removeClass("tabs-justified").addClass(_38.justified?"tabs-justified":"");
if(_38.border==true){
_39.removeClass("tabs-header-noborder");
_3a.removeClass("tabs-panels-noborder");
}else{
_39.addClass("tabs-header-noborder");
_3a.addClass("tabs-panels-noborder");
}
_38.doSize=true;
};
function _3c(_3d,_3e,pp){
_3e=_3e||{};
var _3f=$.data(_3d,"tabs");
var _40=_3f.tabs;
if(_3e.index==undefined||_3e.index>_40.length){
_3e.index=_40.length;
}
if(_3e.index<0){
_3e.index=0;
}
var ul=$(_3d).children("div.tabs-header").find("ul.tabs");
var _41=$(_3d).children("div.tabs-panels");
var tab=$("<li>"+"<a href=\"javascript:void(0)\" class=\"tabs-inner\">"+"<span class=\"tabs-title\"></span>"+"<span class=\"tabs-icon\"></span>"+"</a>"+"</li>");
if(!pp){
pp=$("<div></div>");
}
if(_3e.index>=_40.length){
tab.appendTo(ul);
pp.appendTo(_41);
_40.push(pp);
}else{
tab.insertBefore(ul.children("li:eq("+_3e.index+")"));
pp.insertBefore(_41.children("div.panel:eq("+_3e.index+")"));
_40.splice(_3e.index,0,pp);
}
pp.panel($.extend({},_3e,{tab:tab,border:false,noheader:true,closed:true,doSize:false,iconCls:(_3e.icon?_3e.icon:undefined),onLoad:function(){
if(_3e.onLoad){
_3e.onLoad.call(this,arguments);
}
_3f.options.onLoad.call(_3d,$(this));
},onBeforeOpen:function(){
if(_3e.onBeforeOpen){
if(_3e.onBeforeOpen.call(this)==false){
return false;
}
}
var p=$(_3d).tabs("getSelected");
if(p){
if(p[0]!=this){
$(_3d).tabs("unselect",_4a(_3d,p));
p=$(_3d).tabs("getSelected");
if(p){
return false;
}
}else{
_21(_3d);
return false;
}
}
var _42=$(this).panel("options");
_42.tab.addClass("tabs-selected");
var _43=$(_3d).find(">div.tabs-header>div.tabs-wrap");
var _44=_42.tab.position().left;
var _45=_44+_42.tab.outerWidth();
if(_44<0||_45>_43.width()){
var _46=_44-(_43.width()-_42.tab.width())/2;
$(_3d).tabs("scrollBy",_46);
}else{
$(_3d).tabs("scrollBy",0);
}
var _47=$(this).panel("panel");
_47.css("display","block");
_21(_3d);
_47.css("display","none");
},onOpen:function(){
if(_3e.onOpen){
_3e.onOpen.call(this);
}
var _48=$(this).panel("options");
_3f.selectHis.push(_48.title);
_3f.options.onSelect.call(_3d,_48.title,_4a(_3d,this));
},onBeforeClose:function(){
if(_3e.onBeforeClose){
if(_3e.onBeforeClose.call(this)==false){
return false;
}
}
$(this).panel("options").tab.removeClass("tabs-selected");
},onClose:function(){
if(_3e.onClose){
_3e.onClose.call(this);
}
var _49=$(this).panel("options");
_3f.options.onUnselect.call(_3d,_49.title,_4a(_3d,this));
}}));
$(_3d).tabs("update",{tab:pp,options:pp.panel("options"),type:"header"});
};
function _4b(_4c,_4d){
var _4e=$.data(_4c,"tabs");
var _4f=_4e.options;
if(_4d.selected==undefined){
_4d.selected=true;
}
_3c(_4c,_4d);
_4f.onAdd.call(_4c,_4d.title,_4d.index);
if(_4d.selected){
_50(_4c,_4d.index);
}
};
function _51(_52,_53){
_53.type=_53.type||"all";
var _54=$.data(_52,"tabs").selectHis;
var pp=_53.tab;
var _55=pp.panel("options");
var _56=_55.title;
$.extend(_55,_53.options,{iconCls:(_53.options.icon?_53.options.icon:undefined)});
if(_53.type=="all"||_53.type=="body"){
pp.panel();
}
if(_53.type=="all"||_53.type=="header"){
var tab=_55.tab;
if(_55.header){
tab.find(".tabs-inner").html($(_55.header));
}else{
var _57=tab.find("span.tabs-title");
var _58=tab.find("span.tabs-icon");
_57.html(_55.title);
_58.attr("class","tabs-icon");
tab.find("a.tabs-close").remove();
if(_55.closable){
_57.addClass("tabs-closable");
$("<a href=\"javascript:void(0)\" class=\"tabs-close\"></a>").appendTo(tab);
}else{
_57.removeClass("tabs-closable");
}
if(_55.iconCls){
_57.addClass("tabs-with-icon");
_58.addClass(_55.iconCls);
}else{
_57.removeClass("tabs-with-icon");
}
if(_55.tools){
var _59=tab.find("span.tabs-p-tool");
if(!_59.length){
var _59=$("<span class=\"tabs-p-tool\"></span>").insertAfter(tab.find("a.tabs-inner"));
}
if($.isArray(_55.tools)){
_59.empty();
for(var i=0;i<_55.tools.length;i++){
var t=$("<a href=\"javascript:void(0)\"></a>").appendTo(_59);
t.addClass(_55.tools[i].iconCls);
if(_55.tools[i].handler){
t.bind("click",{handler:_55.tools[i].handler},function(e){
if($(this).parents("li").hasClass("tabs-disabled")){
return;
}
e.data.handler.call(this);
});
}
}
}else{
$(_55.tools).children().appendTo(_59);
}
var pr=_59.children().length*12;
if(_55.closable){
pr+=8;
}else{
pr-=3;
_59.css("right","5px");
}
_57.css("padding-right",pr+"px");
}else{
tab.find("span.tabs-p-tool").remove();
_57.css("padding-right","");
}
}
if(_56!=_55.title){
for(var i=0;i<_54.length;i++){
if(_54[i]==_56){
_54[i]=_55.title;
}
}
}
}
if(_55.disabled){
_55.tab.addClass("tabs-disabled");
}else{
_55.tab.removeClass("tabs-disabled");
}
_13(_52);
$.data(_52,"tabs").options.onUpdate.call(_52,_55.title,_4a(_52,pp));
};
function _5a(_5b,_5c){
var _5d=$.data(_5b,"tabs").options;
var _5e=$.data(_5b,"tabs").tabs;
var _5f=$.data(_5b,"tabs").selectHis;
if(!_60(_5b,_5c)){
return;
}
var tab=_61(_5b,_5c);
var _62=tab.panel("options").title;
var _63=_4a(_5b,tab);
if(_5d.onBeforeClose.call(_5b,_62,_63)==false){
return;
}
var tab=_61(_5b,_5c,true);
tab.panel("options").tab.remove();
tab.panel("destroy");
_5d.onClose.call(_5b,_62,_63);
_13(_5b);
for(var i=0;i<_5f.length;i++){
if(_5f[i]==_62){
_5f.splice(i,1);
i--;
}
}
var _64=_5f.pop();
if(_64){
_50(_5b,_64);
}else{
if(_5e.length){
_50(_5b,0);
}
}
};
function _61(_65,_66,_67){
var _68=$.data(_65,"tabs").tabs;
if(typeof _66=="number"){
if(_66<0||_66>=_68.length){
return null;
}else{
var tab=_68[_66];
if(_67){
_68.splice(_66,1);
}
return tab;
}
}
for(var i=0;i<_68.length;i++){
var tab=_68[i];
if(tab.panel("options").title==_66){
if(_67){
_68.splice(i,1);
}
return tab;
}
}
return null;
};
function _4a(_69,tab){
var _6a=$.data(_69,"tabs").tabs;
for(var i=0;i<_6a.length;i++){
if(_6a[i][0]==$(tab)[0]){
return i;
}
}
return -1;
};
function _24(_6b){
var _6c=$.data(_6b,"tabs").tabs;
for(var i=0;i<_6c.length;i++){
var tab=_6c[i];
if(tab.panel("options").tab.hasClass("tabs-selected")){
return tab;
}
}
return null;
};
function _6d(_6e){
var _6f=$.data(_6e,"tabs");
var _70=_6f.tabs;
for(var i=0;i<_70.length;i++){
var _71=_70[i].panel("options");
if(_71.selected&&!_71.disabled){
_50(_6e,i);
return;
}
}
_50(_6e,_6f.options.selected);
};
function _50(_72,_73){
var p=_61(_72,_73);
if(p&&!p.is(":visible")){
_74(_72);
if(!p.panel("options").disabled){
p.panel("open");
}
}
};
function _75(_76,_77){
var p=_61(_76,_77);
if(p&&p.is(":visible")){
_74(_76);
p.panel("close");
}
};
function _74(_78){
$(_78).children("div.tabs-panels").each(function(){
$(this).stop(true,true);
});
};
function _60(_79,_7a){
return _61(_79,_7a)!=null;
};
function _7b(_7c,_7d){
var _7e=$.data(_7c,"tabs").options;
_7e.showHeader=_7d;
$(_7c).tabs("resize");
};
function _7f(_80,_81){
var _82=$(_80).find(">.tabs-header>.tabs-tool");
if(_81){
_82.removeClass("tabs-tool-hidden").show();
}else{
_82.addClass("tabs-tool-hidden").hide();
}
$(_80).tabs("resize").tabs("scrollBy",0);
};
$.fn.tabs=function(_83,_84){
if(typeof _83=="string"){
return $.fn.tabs.methods[_83](this,_84);
}
_83=_83||{};
return this.each(function(){
var _85=$.data(this,"tabs");
if(_85){
$.extend(_85.options,_83);
}else{
$.data(this,"tabs",{options:$.extend({},$.fn.tabs.defaults,$.fn.tabs.parseOptions(this),_83),tabs:[],selectHis:[]});
_28(this);
}
_d(this);
_36(this);
_13(this);
_2e(this);
_6d(this);
});
};
$.fn.tabs.methods={options:function(jq){
var cc=jq[0];
var _86=$.data(cc,"tabs").options;
var s=_24(cc);
_86.selected=s?_4a(cc,s):-1;
return _86;
},tabs:function(jq){
return $.data(jq[0],"tabs").tabs;
},resize:function(jq,_87){
return jq.each(function(){
_13(this,_87);
_21(this);
});
},add:function(jq,_88){
return jq.each(function(){
_4b(this,_88);
});
},close:function(jq,_89){
return jq.each(function(){
_5a(this,_89);
});
},getTab:function(jq,_8a){
return _61(jq[0],_8a);
},getTabIndex:function(jq,tab){
return _4a(jq[0],tab);
},getSelected:function(jq){
return _24(jq[0]);
},select:function(jq,_8b){
return jq.each(function(){
_50(this,_8b);
});
},unselect:function(jq,_8c){
return jq.each(function(){
_75(this,_8c);
});
},exists:function(jq,_8d){
return _60(jq[0],_8d);
},update:function(jq,_8e){
return jq.each(function(){
_51(this,_8e);
});
},enableTab:function(jq,_8f){
return jq.each(function(){
var _90=$(this).tabs("getTab",_8f).panel("options");
_90.tab.removeClass("tabs-disabled");
_90.disabled=false;
});
},disableTab:function(jq,_91){
return jq.each(function(){
var _92=$(this).tabs("getTab",_91).panel("options");
_92.tab.addClass("tabs-disabled");
_92.disabled=true;
});
},showHeader:function(jq){
return jq.each(function(){
_7b(this,true);
});
},hideHeader:function(jq){
return jq.each(function(){
_7b(this,false);
});
},showTool:function(jq){
return jq.each(function(){
_7f(this,true);
});
},hideTool:function(jq){
return jq.each(function(){
_7f(this,false);
});
},scrollBy:function(jq,_93){
return jq.each(function(){
var _94=$(this).tabs("options");
var _95=$(this).find(">div.tabs-header>div.tabs-wrap");
var pos=Math.min(_95._scrollLeft()+_93,_96());
_95.animate({scrollLeft:pos},_94.scrollDuration);
function _96(){
var w=0;
var ul=_95.children("ul");
ul.children("li").each(function(){
w+=$(this).outerWidth(true);
});
return w-_95.width()+(ul.outerWidth()-ul.width());
};
});
}};
$.fn.tabs.parseOptions=function(_97){
return $.extend({},$.parser.parseOptions(_97,["tools","toolPosition","tabPosition",{fit:"boolean",border:"boolean",plain:"boolean"},{headerWidth:"number",tabWidth:"number",tabHeight:"number",selected:"number"},{showHeader:"boolean",justified:"boolean",narrow:"boolean",pill:"boolean"}]));
};
$.fn.tabs.defaults={width:"auto",height:"auto",headerWidth:150,tabWidth:"auto",tabHeight:27,selected:0,showHeader:true,plain:false,fit:false,border:true,justified:false,narrow:false,pill:false,tools:null,toolPosition:"right",tabPosition:"top",scrollIncrement:100,scrollDuration:400,onLoad:function(_98){
},onSelect:function(_99,_9a){
},onUnselect:function(_9b,_9c){
},onBeforeClose:function(_9d,_9e){
},onClose:function(_9f,_a0){
},onAdd:function(_a1,_a2){
},onUpdate:function(_a3,_a4){
},onContextMenu:function(e,_a5,_a6){
}};
})(jQuery);

