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
function _1(_2,_3){
var _4=$.data(_2,"accordion");
var _5=_4.options;
var _6=_4.panels;
var cc=$(_2);
if(_3){
$.extend(_5,{width:_3.width,height:_3.height});
}
cc._size(_5);
var _7=0;
var _8="auto";
var _9=cc.find(">.panel>.accordion-header");
if(_9.length){
_7=$(_9[0]).css("height","")._outerHeight();
}
if(!isNaN(parseInt(_5.height))){
_8=cc.height()-_7*_9.length;
}
_a(true,_8-_a(false)+1);
function _a(_b,_c){
var _d=0;
for(var i=0;i<_6.length;i++){
var p=_6[i];
var h=p.panel("header")._outerHeight(_7);
if(p.panel("options").collapsible==_b){
var _e=isNaN(_c)?undefined:(_c+_7*h.length);
p.panel("resize",{width:cc.width(),height:(_b?_e:undefined)});
_d+=p.panel("panel").outerHeight()-_7*h.length;
}
}
return _d;
};
};
function _f(_10,_11,_12,all){
var _13=$.data(_10,"accordion").panels;
var pp=[];
for(var i=0;i<_13.length;i++){
var p=_13[i];
if(_11){
if(p.panel("options")[_11]==_12){
pp.push(p);
}
}else{
if(p[0]==$(_12)[0]){
return i;
}
}
}
if(_11){
return all?pp:(pp.length?pp[0]:null);
}else{
return -1;
}
};
function _14(_15){
return _f(_15,"collapsed",false,true);
};
function _16(_17){
var pp=_14(_17);
return pp.length?pp[0]:null;
};
function _18(_19,_1a){
return _f(_19,null,_1a);
};
function _1b(_1c,_1d){
var _1e=$.data(_1c,"accordion").panels;
if(typeof _1d=="number"){
if(_1d<0||_1d>=_1e.length){
return null;
}else{
return _1e[_1d];
}
}
return _f(_1c,"title",_1d);
};
function _1f(_20){
var _21=$.data(_20,"accordion").options;
var cc=$(_20);
if(_21.border){
cc.removeClass("accordion-noborder");
}else{
cc.addClass("accordion-noborder");
}
};
function _22(_23){
var _24=$.data(_23,"accordion");
var cc=$(_23);
cc.addClass("accordion");
_24.panels=[];
cc.children("div").each(function(){
var _25=$.extend({},$.parser.parseOptions(this),{selected:($(this).attr("selected")?true:undefined)});
var pp=$(this);
_24.panels.push(pp);
_27(_23,pp,_25);
});
cc.bind("_resize",function(e,_26){
if($(this).hasClass("easyui-fluid")||_26){
_1(_23);
}
return false;
});
};
function _27(_28,pp,_29){
var _2a=$.data(_28,"accordion").options;
pp.panel($.extend({},{collapsible:true,minimizable:false,maximizable:false,closable:false,doSize:false,collapsed:true,headerCls:"accordion-header",bodyCls:"accordion-body"},_29,{onBeforeExpand:function(){
if(_29.onBeforeExpand){
if(_29.onBeforeExpand.call(this)==false){
return false;
}
}
if(!_2a.multiple){
var all=$.grep(_14(_28),function(p){
return p.panel("options").collapsible;
});
for(var i=0;i<all.length;i++){
_33(_28,_18(_28,all[i]));
}
}
var _2b=$(this).panel("header");
_2b.addClass("accordion-header-selected");
_2b.find(".accordion-collapse").removeClass("accordion-expand");
},onExpand:function(){
if(_29.onExpand){
_29.onExpand.call(this);
}
_2a.onSelect.call(_28,$(this).panel("options").title,_18(_28,this));
},onBeforeCollapse:function(){
if(_29.onBeforeCollapse){
if(_29.onBeforeCollapse.call(this)==false){
return false;
}
}
var _2c=$(this).panel("header");
_2c.removeClass("accordion-header-selected");
_2c.find(".accordion-collapse").addClass("accordion-expand");
},onCollapse:function(){
if(_29.onCollapse){
_29.onCollapse.call(this);
}
_2a.onUnselect.call(_28,$(this).panel("options").title,_18(_28,this));
}}));
var _2d=pp.panel("header");
var _2e=_2d.children("div.panel-tool");
_2e.children("a.panel-tool-collapse").hide();
var t=$("<a href=\"javascript:void(0)\"></a>").addClass("accordion-collapse accordion-expand").appendTo(_2e);
t.bind("click",function(){
_2f(pp);
return false;
});
pp.panel("options").collapsible?t.show():t.hide();
_2d.click(function(){
_2f(pp);
return false;
});
function _2f(p){
var _30=p.panel("options");
if(_30.collapsible){
var _31=_18(_28,p);
if(_30.collapsed){
_32(_28,_31);
}else{
_33(_28,_31);
}
}
};
};
function _32(_34,_35){
var p=_1b(_34,_35);
if(!p){
return;
}
_36(_34);
var _37=$.data(_34,"accordion").options;
p.panel("expand",_37.animate);
};
function _33(_38,_39){
var p=_1b(_38,_39);
if(!p){
return;
}
_36(_38);
var _3a=$.data(_38,"accordion").options;
p.panel("collapse",_3a.animate);
};
function _3b(_3c){
var _3d=$.data(_3c,"accordion").options;
var p=_f(_3c,"selected",true);
if(p){
_3e(_18(_3c,p));
}else{
_3e(_3d.selected);
}
function _3e(_3f){
var _40=_3d.animate;
_3d.animate=false;
_32(_3c,_3f);
_3d.animate=_40;
};
};
function _36(_41){
var _42=$.data(_41,"accordion").panels;
for(var i=0;i<_42.length;i++){
_42[i].stop(true,true);
}
};
function add(_43,_44){
var _45=$.data(_43,"accordion");
var _46=_45.options;
var _47=_45.panels;
if(_44.selected==undefined){
_44.selected=true;
}
_36(_43);
var pp=$("<div></div>").appendTo(_43);
_47.push(pp);
_27(_43,pp,_44);
_1(_43);
_46.onAdd.call(_43,_44.title,_47.length-1);
if(_44.selected){
_32(_43,_47.length-1);
}
};
function _48(_49,_4a){
var _4b=$.data(_49,"accordion");
var _4c=_4b.options;
var _4d=_4b.panels;
_36(_49);
var _4e=_1b(_49,_4a);
var _4f=_4e.panel("options").title;
var _50=_18(_49,_4e);
if(!_4e){
return;
}
if(_4c.onBeforeRemove.call(_49,_4f,_50)==false){
return;
}
_4d.splice(_50,1);
_4e.panel("destroy");
if(_4d.length){
_1(_49);
var _51=_16(_49);
if(!_51){
_32(_49,0);
}
}
_4c.onRemove.call(_49,_4f,_50);
};
$.fn.accordion=function(_52,_53){
if(typeof _52=="string"){
return $.fn.accordion.methods[_52](this,_53);
}
_52=_52||{};
return this.each(function(){
var _54=$.data(this,"accordion");
if(_54){
$.extend(_54.options,_52);
}else{
$.data(this,"accordion",{options:$.extend({},$.fn.accordion.defaults,$.fn.accordion.parseOptions(this),_52),accordion:$(this).addClass("accordion"),panels:[]});
_22(this);
}
_1f(this);
_1(this);
_3b(this);
});
};
$.fn.accordion.methods={options:function(jq){
return $.data(jq[0],"accordion").options;
},panels:function(jq){
return $.data(jq[0],"accordion").panels;
},resize:function(jq,_55){
return jq.each(function(){
_1(this,_55);
});
},getSelections:function(jq){
return _14(jq[0]);
},getSelected:function(jq){
return _16(jq[0]);
},getPanel:function(jq,_56){
return _1b(jq[0],_56);
},getPanelIndex:function(jq,_57){
return _18(jq[0],_57);
},select:function(jq,_58){
return jq.each(function(){
_32(this,_58);
});
},unselect:function(jq,_59){
return jq.each(function(){
_33(this,_59);
});
},add:function(jq,_5a){
return jq.each(function(){
add(this,_5a);
});
},remove:function(jq,_5b){
return jq.each(function(){
_48(this,_5b);
});
}};
$.fn.accordion.parseOptions=function(_5c){
var t=$(_5c);
return $.extend({},$.parser.parseOptions(_5c,["width","height",{fit:"boolean",border:"boolean",animate:"boolean",multiple:"boolean",selected:"number"}]));
};
$.fn.accordion.defaults={width:"auto",height:"auto",fit:false,border:true,animate:true,multiple:false,selected:0,onSelect:function(_5d,_5e){
},onUnselect:function(_5f,_60){
},onAdd:function(_61,_62){
},onBeforeRemove:function(_63,_64){
},onRemove:function(_65,_66){
}};
})(jQuery);

