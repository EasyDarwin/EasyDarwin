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
$.fn._remove=function(){
return this.each(function(){
$(this).remove();
try{
this.outerHTML="";
}
catch(err){
}
});
};
function _1(_2){
_2._remove();
};
function _3(_4,_5){
var _6=$.data(_4,"panel");
var _7=_6.options;
var _8=_6.panel;
var _9=_8.children(".panel-header");
var _a=_8.children(".panel-body");
var _b=_8.children(".panel-footer");
if(_5){
$.extend(_7,{width:_5.width,height:_5.height,minWidth:_5.minWidth,maxWidth:_5.maxWidth,minHeight:_5.minHeight,maxHeight:_5.maxHeight,left:_5.left,top:_5.top});
}
_8._size(_7);
_9.add(_a)._outerWidth(_8.width());
if(!isNaN(parseInt(_7.height))){
_a._outerHeight(_8.height()-_9._outerHeight()-_b._outerHeight());
}else{
_a.css("height","");
var _c=$.parser.parseValue("minHeight",_7.minHeight,_8.parent());
var _d=$.parser.parseValue("maxHeight",_7.maxHeight,_8.parent());
var _e=_9._outerHeight()+_b._outerHeight()+_8._outerHeight()-_8.height();
_a._size("minHeight",_c?(_c-_e):"");
_a._size("maxHeight",_d?(_d-_e):"");
}
_8.css({height:"",minHeight:"",maxHeight:"",left:_7.left,top:_7.top});
_7.onResize.apply(_4,[_7.width,_7.height]);
$(_4).panel("doLayout");
};
function _f(_10,_11){
var _12=$.data(_10,"panel").options;
var _13=$.data(_10,"panel").panel;
if(_11){
if(_11.left!=null){
_12.left=_11.left;
}
if(_11.top!=null){
_12.top=_11.top;
}
}
_13.css({left:_12.left,top:_12.top});
_12.onMove.apply(_10,[_12.left,_12.top]);
};
function _14(_15){
$(_15).addClass("panel-body")._size("clear");
var _16=$("<div class=\"panel\"></div>").insertBefore(_15);
_16[0].appendChild(_15);
_16.bind("_resize",function(e,_17){
if($(this).hasClass("easyui-fluid")||_17){
_3(_15);
}
return false;
});
return _16;
};
function _18(_19){
var _1a=$.data(_19,"panel");
var _1b=_1a.options;
var _1c=_1a.panel;
_1c.css(_1b.style);
_1c.addClass(_1b.cls);
_1d();
_1e();
var _1f=$(_19).panel("header");
var _20=$(_19).panel("body");
var _21=$(_19).siblings(".panel-footer");
if(_1b.border){
_1f.removeClass("panel-header-noborder");
_20.removeClass("panel-body-noborder");
_21.removeClass("panel-footer-noborder");
}else{
_1f.addClass("panel-header-noborder");
_20.addClass("panel-body-noborder");
_21.addClass("panel-footer-noborder");
}
_1f.addClass(_1b.headerCls);
_20.addClass(_1b.bodyCls);
$(_19).attr("id",_1b.id||"");
if(_1b.content){
$(_19).panel("clear");
$(_19).html(_1b.content);
$.parser.parse($(_19));
}
function _1d(){
if(_1b.noheader||(!_1b.title&&!_1b.header)){
_1(_1c.children(".panel-header"));
_1c.children(".panel-body").addClass("panel-body-noheader");
}else{
if(_1b.header){
$(_1b.header).addClass("panel-header").prependTo(_1c);
}else{
var _22=_1c.children(".panel-header");
if(!_22.length){
_22=$("<div class=\"panel-header\"></div>").prependTo(_1c);
}
if(!$.isArray(_1b.tools)){
_22.find("div.panel-tool .panel-tool-a").appendTo(_1b.tools);
}
_22.empty();
var _23=$("<div class=\"panel-title\"></div>").html(_1b.title).appendTo(_22);
if(_1b.iconCls){
_23.addClass("panel-with-icon");
$("<div class=\"panel-icon\"></div>").addClass(_1b.iconCls).appendTo(_22);
}
var _24=$("<div class=\"panel-tool\"></div>").appendTo(_22);
_24.bind("click",function(e){
e.stopPropagation();
});
if(_1b.tools){
if($.isArray(_1b.tools)){
$.map(_1b.tools,function(t){
_25(_24,t.iconCls,eval(t.handler));
});
}else{
$(_1b.tools).children().each(function(){
$(this).addClass($(this).attr("iconCls")).addClass("panel-tool-a").appendTo(_24);
});
}
}
if(_1b.collapsible){
_25(_24,"panel-tool-collapse",function(){
if(_1b.collapsed==true){
_4d(_19,true);
}else{
_3b(_19,true);
}
});
}
if(_1b.minimizable){
_25(_24,"panel-tool-min",function(){
_58(_19);
});
}
if(_1b.maximizable){
_25(_24,"panel-tool-max",function(){
if(_1b.maximized==true){
_5c(_19);
}else{
_3a(_19);
}
});
}
if(_1b.closable){
_25(_24,"panel-tool-close",function(){
_3c(_19);
});
}
}
_1c.children("div.panel-body").removeClass("panel-body-noheader");
}
};
function _25(c,_26,_27){
var a=$("<a href=\"javascript:void(0)\"></a>").addClass(_26).appendTo(c);
a.bind("click",_27);
};
function _1e(){
if(_1b.footer){
$(_1b.footer).addClass("panel-footer").appendTo(_1c);
$(_19).addClass("panel-body-nobottom");
}else{
_1c.children(".panel-footer").remove();
$(_19).removeClass("panel-body-nobottom");
}
};
};
function _28(_29,_2a){
var _2b=$.data(_29,"panel");
var _2c=_2b.options;
if(_2d){
_2c.queryParams=_2a;
}
if(!_2c.href){
return;
}
if(!_2b.isLoaded||!_2c.cache){
var _2d=$.extend({},_2c.queryParams);
if(_2c.onBeforeLoad.call(_29,_2d)==false){
return;
}
_2b.isLoaded=false;
$(_29).panel("clear");
if(_2c.loadingMessage){
$(_29).html($("<div class=\"panel-loading\"></div>").html(_2c.loadingMessage));
}
_2c.loader.call(_29,_2d,function(_2e){
var _2f=_2c.extractor.call(_29,_2e);
$(_29).html(_2f);
$.parser.parse($(_29));
_2c.onLoad.apply(_29,arguments);
_2b.isLoaded=true;
},function(){
_2c.onLoadError.apply(_29,arguments);
});
}
};
function _30(_31){
var t=$(_31);
t.find(".combo-f").each(function(){
$(this).combo("destroy");
});
t.find(".m-btn").each(function(){
$(this).menubutton("destroy");
});
t.find(".s-btn").each(function(){
$(this).splitbutton("destroy");
});
t.find(".tooltip-f").each(function(){
$(this).tooltip("destroy");
});
t.children("div").each(function(){
$(this)._size("unfit");
});
t.empty();
};
function _32(_33){
$(_33).panel("doLayout",true);
};
function _34(_35,_36){
var _37=$.data(_35,"panel").options;
var _38=$.data(_35,"panel").panel;
if(_36!=true){
if(_37.onBeforeOpen.call(_35)==false){
return;
}
}
_38.stop(true,true);
if($.isFunction(_37.openAnimation)){
_37.openAnimation.call(_35,cb);
}else{
switch(_37.openAnimation){
case "slide":
_38.slideDown(_37.openDuration,cb);
break;
case "fade":
_38.fadeIn(_37.openDuration,cb);
break;
case "show":
_38.show(_37.openDuration,cb);
break;
default:
_38.show();
cb();
}
}
function cb(){
_37.closed=false;
_37.minimized=false;
var _39=_38.children(".panel-header").find("a.panel-tool-restore");
if(_39.length){
_37.maximized=true;
}
_37.onOpen.call(_35);
if(_37.maximized==true){
_37.maximized=false;
_3a(_35);
}
if(_37.collapsed==true){
_37.collapsed=false;
_3b(_35);
}
if(!_37.collapsed){
_28(_35);
_32(_35);
}
};
};
function _3c(_3d,_3e){
var _3f=$.data(_3d,"panel").options;
var _40=$.data(_3d,"panel").panel;
if(_3e!=true){
if(_3f.onBeforeClose.call(_3d)==false){
return;
}
}
_40.stop(true,true);
_40._size("unfit");
if($.isFunction(_3f.closeAnimation)){
_3f.closeAnimation.call(_3d,cb);
}else{
switch(_3f.closeAnimation){
case "slide":
_40.slideUp(_3f.closeDuration,cb);
break;
case "fade":
_40.fadeOut(_3f.closeDuration,cb);
break;
case "hide":
_40.hide(_3f.closeDuration,cb);
break;
default:
_40.hide();
cb();
}
}
function cb(){
_3f.closed=true;
_3f.onClose.call(_3d);
};
};
function _41(_42,_43){
var _44=$.data(_42,"panel");
var _45=_44.options;
var _46=_44.panel;
if(_43!=true){
if(_45.onBeforeDestroy.call(_42)==false){
return;
}
}
$(_42).panel("clear").panel("clear","footer");
_1(_46);
_45.onDestroy.call(_42);
};
function _3b(_47,_48){
var _49=$.data(_47,"panel").options;
var _4a=$.data(_47,"panel").panel;
var _4b=_4a.children(".panel-body");
var _4c=_4a.children(".panel-header").find("a.panel-tool-collapse");
if(_49.collapsed==true){
return;
}
_4b.stop(true,true);
if(_49.onBeforeCollapse.call(_47)==false){
return;
}
_4c.addClass("panel-tool-expand");
if(_48==true){
_4b.slideUp("normal",function(){
_49.collapsed=true;
_49.onCollapse.call(_47);
});
}else{
_4b.hide();
_49.collapsed=true;
_49.onCollapse.call(_47);
}
};
function _4d(_4e,_4f){
var _50=$.data(_4e,"panel").options;
var _51=$.data(_4e,"panel").panel;
var _52=_51.children(".panel-body");
var _53=_51.children(".panel-header").find("a.panel-tool-collapse");
if(_50.collapsed==false){
return;
}
_52.stop(true,true);
if(_50.onBeforeExpand.call(_4e)==false){
return;
}
_53.removeClass("panel-tool-expand");
if(_4f==true){
_52.slideDown("normal",function(){
_50.collapsed=false;
_50.onExpand.call(_4e);
_28(_4e);
_32(_4e);
});
}else{
_52.show();
_50.collapsed=false;
_50.onExpand.call(_4e);
_28(_4e);
_32(_4e);
}
};
function _3a(_54){
var _55=$.data(_54,"panel").options;
var _56=$.data(_54,"panel").panel;
var _57=_56.children(".panel-header").find("a.panel-tool-max");
if(_55.maximized==true){
return;
}
_57.addClass("panel-tool-restore");
if(!$.data(_54,"panel").original){
$.data(_54,"panel").original={width:_55.width,height:_55.height,left:_55.left,top:_55.top,fit:_55.fit};
}
_55.left=0;
_55.top=0;
_55.fit=true;
_3(_54);
_55.minimized=false;
_55.maximized=true;
_55.onMaximize.call(_54);
};
function _58(_59){
var _5a=$.data(_59,"panel").options;
var _5b=$.data(_59,"panel").panel;
_5b._size("unfit");
_5b.hide();
_5a.minimized=true;
_5a.maximized=false;
_5a.onMinimize.call(_59);
};
function _5c(_5d){
var _5e=$.data(_5d,"panel").options;
var _5f=$.data(_5d,"panel").panel;
var _60=_5f.children(".panel-header").find("a.panel-tool-max");
if(_5e.maximized==false){
return;
}
_5f.show();
_60.removeClass("panel-tool-restore");
$.extend(_5e,$.data(_5d,"panel").original);
_3(_5d);
_5e.minimized=false;
_5e.maximized=false;
$.data(_5d,"panel").original=null;
_5e.onRestore.call(_5d);
};
function _61(_62,_63){
$.data(_62,"panel").options.title=_63;
$(_62).panel("header").find("div.panel-title").html(_63);
};
var _64=null;
$(window).unbind(".panel").bind("resize.panel",function(){
if(_64){
clearTimeout(_64);
}
_64=setTimeout(function(){
var _65=$("body.layout");
if(_65.length){
_65.layout("resize");
$("body").children(".easyui-fluid:visible").each(function(){
$(this).triggerHandler("_resize");
});
}else{
$("body").panel("doLayout");
}
_64=null;
},100);
});
$.fn.panel=function(_66,_67){
if(typeof _66=="string"){
return $.fn.panel.methods[_66](this,_67);
}
_66=_66||{};
return this.each(function(){
var _68=$.data(this,"panel");
var _69;
if(_68){
_69=$.extend(_68.options,_66);
_68.isLoaded=false;
}else{
_69=$.extend({},$.fn.panel.defaults,$.fn.panel.parseOptions(this),_66);
$(this).attr("title","");
_68=$.data(this,"panel",{options:_69,panel:_14(this),isLoaded:false});
}
_18(this);
if(_69.doSize==true){
_68.panel.css("display","block");
_3(this);
}
if(_69.closed==true||_69.minimized==true){
_68.panel.hide();
}else{
_34(this);
}
});
};
$.fn.panel.methods={options:function(jq){
return $.data(jq[0],"panel").options;
},panel:function(jq){
return $.data(jq[0],"panel").panel;
},header:function(jq){
return $.data(jq[0],"panel").panel.children(".panel-header");
},footer:function(jq){
return jq.panel("panel").children(".panel-footer");
},body:function(jq){
return $.data(jq[0],"panel").panel.children(".panel-body");
},setTitle:function(jq,_6a){
return jq.each(function(){
_61(this,_6a);
});
},open:function(jq,_6b){
return jq.each(function(){
_34(this,_6b);
});
},close:function(jq,_6c){
return jq.each(function(){
_3c(this,_6c);
});
},destroy:function(jq,_6d){
return jq.each(function(){
_41(this,_6d);
});
},clear:function(jq,_6e){
return jq.each(function(){
_30(_6e=="footer"?$(this).panel("footer"):this);
});
},refresh:function(jq,_6f){
return jq.each(function(){
var _70=$.data(this,"panel");
_70.isLoaded=false;
if(_6f){
if(typeof _6f=="string"){
_70.options.href=_6f;
}else{
_70.options.queryParams=_6f;
}
}
_28(this);
});
},resize:function(jq,_71){
return jq.each(function(){
_3(this,_71);
});
},doLayout:function(jq,all){
return jq.each(function(){
_72(this,"body");
_72($(this).siblings(".panel-footer")[0],"footer");
function _72(_73,_74){
if(!_73){
return;
}
var _75=_73==$("body")[0];
var s=$(_73).find("div.panel:visible,div.accordion:visible,div.tabs-container:visible,div.layout:visible,.easyui-fluid:visible").filter(function(_76,el){
var p=$(el).parents(".panel-"+_74+":first");
return _75?p.length==0:p[0]==_73;
});
s.each(function(){
$(this).triggerHandler("_resize",[all||false]);
});
};
});
},move:function(jq,_77){
return jq.each(function(){
_f(this,_77);
});
},maximize:function(jq){
return jq.each(function(){
_3a(this);
});
},minimize:function(jq){
return jq.each(function(){
_58(this);
});
},restore:function(jq){
return jq.each(function(){
_5c(this);
});
},collapse:function(jq,_78){
return jq.each(function(){
_3b(this,_78);
});
},expand:function(jq,_79){
return jq.each(function(){
_4d(this,_79);
});
}};
$.fn.panel.parseOptions=function(_7a){
var t=$(_7a);
var hh=t.children(".panel-header,header");
var ff=t.children(".panel-footer,footer");
return $.extend({},$.parser.parseOptions(_7a,["id","width","height","left","top","title","iconCls","cls","headerCls","bodyCls","tools","href","method","header","footer",{cache:"boolean",fit:"boolean",border:"boolean",noheader:"boolean"},{collapsible:"boolean",minimizable:"boolean",maximizable:"boolean"},{closable:"boolean",collapsed:"boolean",minimized:"boolean",maximized:"boolean",closed:"boolean"},"openAnimation","closeAnimation",{openDuration:"number",closeDuration:"number"},]),{loadingMessage:(t.attr("loadingMessage")!=undefined?t.attr("loadingMessage"):undefined),header:(hh.length?hh.removeClass("panel-header"):undefined),footer:(ff.length?ff.removeClass("panel-footer"):undefined)});
};
$.fn.panel.defaults={id:null,title:null,iconCls:null,width:"auto",height:"auto",left:null,top:null,cls:null,headerCls:null,bodyCls:null,style:{},href:null,cache:true,fit:false,border:true,doSize:true,noheader:false,content:null,collapsible:false,minimizable:false,maximizable:false,closable:false,collapsed:false,minimized:false,maximized:false,closed:false,openAnimation:false,openDuration:400,closeAnimation:false,closeDuration:400,tools:null,footer:null,header:null,queryParams:{},method:"get",href:null,loadingMessage:"Loading...",loader:function(_7b,_7c,_7d){
var _7e=$(this).panel("options");
if(!_7e.href){
return false;
}
$.ajax({type:_7e.method,url:_7e.href,cache:false,data:_7b,dataType:"html",success:function(_7f){
_7c(_7f);
},error:function(){
_7d.apply(this,arguments);
}});
},extractor:function(_80){
var _81=/<body[^>]*>((.|[\n\r])*)<\/body>/im;
var _82=_81.exec(_80);
if(_82){
return _82[1];
}else{
return _80;
}
},onBeforeLoad:function(_83){
},onLoad:function(){
},onLoadError:function(){
},onBeforeOpen:function(){
},onOpen:function(){
},onBeforeClose:function(){
},onClose:function(){
},onBeforeDestroy:function(){
},onDestroy:function(){
},onResize:function(_84,_85){
},onMove:function(_86,top){
},onMaximize:function(){
},onRestore:function(){
},onMinimize:function(){
},onBeforeCollapse:function(){
},onBeforeExpand:function(){
},onCollapse:function(){
},onExpand:function(){
}};
})(jQuery);

