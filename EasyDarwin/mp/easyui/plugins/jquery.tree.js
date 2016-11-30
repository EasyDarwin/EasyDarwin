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
var _3=$(_2);
_3.addClass("tree");
return _3;
};
function _4(_5){
var _6=$.data(_5,"tree").options;
$(_5).unbind().bind("mouseover",function(e){
var tt=$(e.target);
var _7=tt.closest("div.tree-node");
if(!_7.length){
return;
}
_7.addClass("tree-node-hover");
if(tt.hasClass("tree-hit")){
if(tt.hasClass("tree-expanded")){
tt.addClass("tree-expanded-hover");
}else{
tt.addClass("tree-collapsed-hover");
}
}
e.stopPropagation();
}).bind("mouseout",function(e){
var tt=$(e.target);
var _8=tt.closest("div.tree-node");
if(!_8.length){
return;
}
_8.removeClass("tree-node-hover");
if(tt.hasClass("tree-hit")){
if(tt.hasClass("tree-expanded")){
tt.removeClass("tree-expanded-hover");
}else{
tt.removeClass("tree-collapsed-hover");
}
}
e.stopPropagation();
}).bind("click",function(e){
var tt=$(e.target);
var _9=tt.closest("div.tree-node");
if(!_9.length){
return;
}
if(tt.hasClass("tree-hit")){
_8f(_5,_9[0]);
return false;
}else{
if(tt.hasClass("tree-checkbox")){
_34(_5,_9[0]);
return false;
}else{
_e9(_5,_9[0]);
_6.onClick.call(_5,_c(_5,_9[0]));
}
}
e.stopPropagation();
}).bind("dblclick",function(e){
var _a=$(e.target).closest("div.tree-node");
if(!_a.length){
return;
}
_e9(_5,_a[0]);
_6.onDblClick.call(_5,_c(_5,_a[0]));
e.stopPropagation();
}).bind("contextmenu",function(e){
var _b=$(e.target).closest("div.tree-node");
if(!_b.length){
return;
}
_6.onContextMenu.call(_5,e,_c(_5,_b[0]));
e.stopPropagation();
});
};
function _d(_e){
var _f=$.data(_e,"tree").options;
_f.dnd=false;
var _10=$(_e).find("div.tree-node");
_10.draggable("disable");
_10.css("cursor","pointer");
};
function _11(_12){
var _13=$.data(_12,"tree");
var _14=_13.options;
var _15=_13.tree;
_13.disabledNodes=[];
_14.dnd=true;
_15.find("div.tree-node").draggable({disabled:false,revert:true,cursor:"pointer",proxy:function(_16){
var p=$("<div class=\"tree-node-proxy\"></div>").appendTo("body");
p.html("<span class=\"tree-dnd-icon tree-dnd-no\">&nbsp;</span>"+$(_16).find(".tree-title").html());
p.hide();
return p;
},deltaX:15,deltaY:15,onBeforeDrag:function(e){
if(_14.onBeforeDrag.call(_12,_c(_12,this))==false){
return false;
}
if($(e.target).hasClass("tree-hit")||$(e.target).hasClass("tree-checkbox")){
return false;
}
if(e.which!=1){
return false;
}
var _17=$(this).find("span.tree-indent");
if(_17.length){
e.data.offsetWidth-=_17.length*_17.width();
}
},onStartDrag:function(e){
$(this).next("ul").find("div.tree-node").each(function(){
$(this).droppable("disable");
_13.disabledNodes.push(this);
});
$(this).draggable("proxy").css({left:-10000,top:-10000});
_14.onStartDrag.call(_12,_c(_12,this));
var _18=_c(_12,this);
if(_18.id==undefined){
_18.id="easyui_tree_node_id_temp";
_64(_12,_18);
}
_13.draggingNodeId=_18.id;
},onDrag:function(e){
var x1=e.pageX,y1=e.pageY,x2=e.data.startX,y2=e.data.startY;
var d=Math.sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
if(d>3){
$(this).draggable("proxy").show();
}
this.pageY=e.pageY;
},onStopDrag:function(){
for(var i=0;i<_13.disabledNodes.length;i++){
$(_13.disabledNodes[i]).droppable("enable");
}
_13.disabledNodes=[];
var _19=_dc(_12,_13.draggingNodeId);
if(_19&&_19.id=="easyui_tree_node_id_temp"){
_19.id="";
_64(_12,_19);
}
_14.onStopDrag.call(_12,_19);
}}).droppable({accept:"div.tree-node",onDragEnter:function(e,_1a){
if(_14.onDragEnter.call(_12,this,_1b(_1a))==false){
_1c(_1a,false);
$(this).removeClass("tree-node-append tree-node-top tree-node-bottom");
$(this).droppable("disable");
_13.disabledNodes.push(this);
}
},onDragOver:function(e,_1d){
if($(this).droppable("options").disabled){
return;
}
var _1e=_1d.pageY;
var top=$(this).offset().top;
var _1f=top+$(this).outerHeight();
_1c(_1d,true);
$(this).removeClass("tree-node-append tree-node-top tree-node-bottom");
if(_1e>top+(_1f-top)/2){
if(_1f-_1e<5){
$(this).addClass("tree-node-bottom");
}else{
$(this).addClass("tree-node-append");
}
}else{
if(_1e-top<5){
$(this).addClass("tree-node-top");
}else{
$(this).addClass("tree-node-append");
}
}
if(_14.onDragOver.call(_12,this,_1b(_1d))==false){
_1c(_1d,false);
$(this).removeClass("tree-node-append tree-node-top tree-node-bottom");
$(this).droppable("disable");
_13.disabledNodes.push(this);
}
},onDragLeave:function(e,_20){
_1c(_20,false);
$(this).removeClass("tree-node-append tree-node-top tree-node-bottom");
_14.onDragLeave.call(_12,this,_1b(_20));
},onDrop:function(e,_21){
var _22=this;
var _23,_24;
if($(this).hasClass("tree-node-append")){
_23=_25;
_24="append";
}else{
_23=_26;
_24=$(this).hasClass("tree-node-top")?"top":"bottom";
}
if(_14.onBeforeDrop.call(_12,_22,_1b(_21),_24)==false){
$(this).removeClass("tree-node-append tree-node-top tree-node-bottom");
return;
}
_23(_21,_22,_24);
$(this).removeClass("tree-node-append tree-node-top tree-node-bottom");
}});
function _1b(_27,pop){
return $(_27).closest("ul.tree").tree(pop?"pop":"getData",_27);
};
function _1c(_28,_29){
var _2a=$(_28).draggable("proxy").find("span.tree-dnd-icon");
_2a.removeClass("tree-dnd-yes tree-dnd-no").addClass(_29?"tree-dnd-yes":"tree-dnd-no");
};
function _25(_2b,_2c){
if(_c(_12,_2c).state=="closed"){
_83(_12,_2c,function(){
_2d();
});
}else{
_2d();
}
function _2d(){
var _2e=_1b(_2b,true);
$(_12).tree("append",{parent:_2c,data:[_2e]});
_14.onDrop.call(_12,_2c,_2e,"append");
};
};
function _26(_2f,_30,_31){
var _32={};
if(_31=="top"){
_32.before=_30;
}else{
_32.after=_30;
}
var _33=_1b(_2f,true);
_32.data=_33;
$(_12).tree("insert",_32);
_14.onDrop.call(_12,_30,_33,_31);
};
};
function _34(_35,_36,_37){
var _38=$.data(_35,"tree");
var _39=_38.options;
if(!_39.checkbox){
return;
}
var _3a=_c(_35,_36);
if(_37==undefined){
var ck=$(_36).find(".tree-checkbox");
if(ck.hasClass("tree-checkbox1")){
_37=false;
}else{
if(ck.hasClass("tree-checkbox0")){
_37=true;
}else{
if(_3a._checked==undefined){
_3a._checked=$(_36).find(".tree-checkbox").hasClass("tree-checkbox1");
}
_37=!_3a._checked;
}
}
}
_3a._checked=_37;
if(_39.onBeforeCheck.call(_35,_3a,_37)==false){
return;
}
if(_39.cascadeCheck){
_3b(_3a,_37);
_3c(_3a,_37);
}else{
_3d($(_3a.target),_37?"1":"0");
}
_39.onCheck.call(_35,_3a,_37);
function _3d(_3e,_3f){
var ck=_3e.find(".tree-checkbox");
ck.removeClass("tree-checkbox0 tree-checkbox1 tree-checkbox2");
ck.addClass("tree-checkbox"+_3f);
};
function _3b(_40,_41){
if(_39.deepCheck){
var _42=$("#"+_40.domId);
var _43=_41?"1":"0";
_3d(_42,_43);
_3d(_42.next(),_43);
}else{
_44(_40,_41);
_68(_40.children||[],function(n){
_44(n,_41);
});
}
};
function _44(_45,_46){
if(_45.hidden){
return;
}
var cls="tree-checkbox"+(_46?"1":"0");
var _47=$("#"+_45.domId);
_3d(_47,_46?"1":"0");
if(_45.children){
for(var i=0;i<_45.children.length;i++){
if(_45.children[i].hidden){
if(!$("#"+_45.children[i].domId).find("."+cls).length){
_3d(_47,"2");
var _48=_9a(_35,_47[0]);
while(_48){
_3d($(_48.target),"2");
_48=_9a(_35,_48[0]);
}
return;
}
}
}
}
};
function _3c(_49,_4a){
var _4b=$("#"+_49.domId);
var _4c=_9a(_35,_4b[0]);
if(_4c){
var _4d="";
if(_4e(_4b,true)){
_4d="1";
}else{
if(_4e(_4b,false)){
_4d="0";
}else{
_4d="2";
}
}
_3d($(_4c.target),_4d);
_3c(_4c,_4a);
}
};
function _4e(_4f,_50){
var cls="tree-checkbox"+(_50?"1":"0");
var ck=_4f.find(".tree-checkbox");
if(!ck.hasClass(cls)){
return false;
}
var b=true;
_4f.parent().siblings().each(function(){
var ck=$(this).children("div.tree-node").children(".tree-checkbox");
if(ck.length&&!ck.hasClass(cls)){
b=false;
return false;
}
});
return b;
};
};
function _51(_52,_53){
var _54=$.data(_52,"tree").options;
if(!_54.checkbox){
return;
}
var _55=$(_53);
if(_56(_52,_53)){
var ck=_55.find(".tree-checkbox");
if(ck.length){
if(ck.hasClass("tree-checkbox1")){
_34(_52,_53,true);
}else{
_34(_52,_53,false);
}
}else{
if(_54.onlyLeafCheck){
$("<span class=\"tree-checkbox tree-checkbox0\"></span>").insertBefore(_55.find(".tree-title"));
}
}
}else{
var ck=_55.find(".tree-checkbox");
if(_54.onlyLeafCheck){
ck.remove();
}else{
if(ck.hasClass("tree-checkbox1")){
_34(_52,_53,true);
}else{
if(ck.hasClass("tree-checkbox2")){
var _57=true;
var _58=true;
var _59=_5a(_52,_53);
for(var i=0;i<_59.length;i++){
if(_59[i].checked){
_58=false;
}else{
_57=false;
}
}
if(_57){
_34(_52,_53,true);
}
if(_58){
_34(_52,_53,false);
}
}
}
}
}
};
function _5b(_5c,ul,_5d,_5e){
var _5f=$.data(_5c,"tree");
var _60=_5f.options;
var _61=$(ul).prevAll("div.tree-node:first");
_5d=_60.loadFilter.call(_5c,_5d,_61[0]);
var _62=_63(_5c,"domId",_61.attr("id"));
if(!_5e){
_62?_62.children=_5d:_5f.data=_5d;
$(ul).empty();
}else{
if(_62){
_62.children?_62.children=_62.children.concat(_5d):_62.children=_5d;
}else{
_5f.data=_5f.data.concat(_5d);
}
}
_60.view.render.call(_60.view,_5c,ul,_5d);
if(_60.dnd){
_11(_5c);
}
if(_62){
_64(_5c,_62);
}
var _65=[];
var _66=[];
for(var i=0;i<_5d.length;i++){
var _67=_5d[i];
if(!_67.checked){
_65.push(_67);
}
}
_68(_5d,function(_69){
if(_69.checked){
_66.push(_69);
}
});
var _6a=_60.onCheck;
_60.onCheck=function(){
};
if(_65.length){
_34(_5c,$("#"+_65[0].domId)[0],false);
}
for(var i=0;i<_66.length;i++){
_34(_5c,$("#"+_66[i].domId)[0],true);
}
_60.onCheck=_6a;
setTimeout(function(){
_6b(_5c,_5c);
},0);
_60.onLoadSuccess.call(_5c,_62,_5d);
};
function _6b(_6c,ul,_6d){
var _6e=$.data(_6c,"tree").options;
if(_6e.lines){
$(_6c).addClass("tree-lines");
}else{
$(_6c).removeClass("tree-lines");
return;
}
if(!_6d){
_6d=true;
$(_6c).find("span.tree-indent").removeClass("tree-line tree-join tree-joinbottom");
$(_6c).find("div.tree-node").removeClass("tree-node-last tree-root-first tree-root-one");
var _6f=$(_6c).tree("getRoots");
if(_6f.length>1){
$(_6f[0].target).addClass("tree-root-first");
}else{
if(_6f.length==1){
$(_6f[0].target).addClass("tree-root-one");
}
}
}
$(ul).children("li").each(function(){
var _70=$(this).children("div.tree-node");
var ul=_70.next("ul");
if(ul.length){
if($(this).next().length){
_71(_70);
}
_6b(_6c,ul,_6d);
}else{
_72(_70);
}
});
var _73=$(ul).children("li:last").children("div.tree-node").addClass("tree-node-last");
_73.children("span.tree-join").removeClass("tree-join").addClass("tree-joinbottom");
function _72(_74,_75){
var _76=_74.find("span.tree-icon");
_76.prev("span.tree-indent").addClass("tree-join");
};
function _71(_77){
var _78=_77.find("span.tree-indent, span.tree-hit").length;
_77.next().find("div.tree-node").each(function(){
$(this).children("span:eq("+(_78-1)+")").addClass("tree-line");
});
};
};
function _79(_7a,ul,_7b,_7c){
var _7d=$.data(_7a,"tree").options;
_7b=$.extend({},_7d.queryParams,_7b||{});
var _7e=null;
if(_7a!=ul){
var _7f=$(ul).prev();
_7e=_c(_7a,_7f[0]);
}
if(_7d.onBeforeLoad.call(_7a,_7e,_7b)==false){
return;
}
var _80=$(ul).prev().children("span.tree-folder");
_80.addClass("tree-loading");
var _81=_7d.loader.call(_7a,_7b,function(_82){
_80.removeClass("tree-loading");
_5b(_7a,ul,_82);
if(_7c){
_7c();
}
},function(){
_80.removeClass("tree-loading");
_7d.onLoadError.apply(_7a,arguments);
if(_7c){
_7c();
}
});
if(_81==false){
_80.removeClass("tree-loading");
}
};
function _83(_84,_85,_86){
var _87=$.data(_84,"tree").options;
var hit=$(_85).children("span.tree-hit");
if(hit.length==0){
return;
}
if(hit.hasClass("tree-expanded")){
return;
}
var _88=_c(_84,_85);
if(_87.onBeforeExpand.call(_84,_88)==false){
return;
}
hit.removeClass("tree-collapsed tree-collapsed-hover").addClass("tree-expanded");
hit.next().addClass("tree-folder-open");
var ul=$(_85).next();
if(ul.length){
if(_87.animate){
ul.slideDown("normal",function(){
_88.state="open";
_87.onExpand.call(_84,_88);
if(_86){
_86();
}
});
}else{
ul.css("display","block");
_88.state="open";
_87.onExpand.call(_84,_88);
if(_86){
_86();
}
}
}else{
var _89=$("<ul style=\"display:none\"></ul>").insertAfter(_85);
_79(_84,_89[0],{id:_88.id},function(){
if(_89.is(":empty")){
_89.remove();
}
if(_87.animate){
_89.slideDown("normal",function(){
_88.state="open";
_87.onExpand.call(_84,_88);
if(_86){
_86();
}
});
}else{
_89.css("display","block");
_88.state="open";
_87.onExpand.call(_84,_88);
if(_86){
_86();
}
}
});
}
};
function _8a(_8b,_8c){
var _8d=$.data(_8b,"tree").options;
var hit=$(_8c).children("span.tree-hit");
if(hit.length==0){
return;
}
if(hit.hasClass("tree-collapsed")){
return;
}
var _8e=_c(_8b,_8c);
if(_8d.onBeforeCollapse.call(_8b,_8e)==false){
return;
}
hit.removeClass("tree-expanded tree-expanded-hover").addClass("tree-collapsed");
hit.next().removeClass("tree-folder-open");
var ul=$(_8c).next();
if(_8d.animate){
ul.slideUp("normal",function(){
_8e.state="closed";
_8d.onCollapse.call(_8b,_8e);
});
}else{
ul.css("display","none");
_8e.state="closed";
_8d.onCollapse.call(_8b,_8e);
}
};
function _8f(_90,_91){
var hit=$(_91).children("span.tree-hit");
if(hit.length==0){
return;
}
if(hit.hasClass("tree-expanded")){
_8a(_90,_91);
}else{
_83(_90,_91);
}
};
function _92(_93,_94){
var _95=_5a(_93,_94);
if(_94){
_95.unshift(_c(_93,_94));
}
for(var i=0;i<_95.length;i++){
_83(_93,_95[i].target);
}
};
function _96(_97,_98){
var _99=[];
var p=_9a(_97,_98);
while(p){
_99.unshift(p);
p=_9a(_97,p.target);
}
for(var i=0;i<_99.length;i++){
_83(_97,_99[i].target);
}
};
function _9b(_9c,_9d){
var c=$(_9c).parent();
while(c[0].tagName!="BODY"&&c.css("overflow-y")!="auto"){
c=c.parent();
}
var n=$(_9d);
var _9e=n.offset().top;
if(c[0].tagName!="BODY"){
var _9f=c.offset().top;
if(_9e<_9f){
c.scrollTop(c.scrollTop()+_9e-_9f);
}else{
if(_9e+n.outerHeight()>_9f+c.outerHeight()-18){
c.scrollTop(c.scrollTop()+_9e+n.outerHeight()-_9f-c.outerHeight()+18);
}
}
}else{
c.scrollTop(_9e);
}
};
function _a0(_a1,_a2){
var _a3=_5a(_a1,_a2);
if(_a2){
_a3.unshift(_c(_a1,_a2));
}
for(var i=0;i<_a3.length;i++){
_8a(_a1,_a3[i].target);
}
};
function _a4(_a5,_a6){
var _a7=$(_a6.parent);
var _a8=_a6.data;
if(!_a8){
return;
}
_a8=$.isArray(_a8)?_a8:[_a8];
if(!_a8.length){
return;
}
var ul;
if(_a7.length==0){
ul=$(_a5);
}else{
if(_56(_a5,_a7[0])){
var _a9=_a7.find("span.tree-icon");
_a9.removeClass("tree-file").addClass("tree-folder tree-folder-open");
var hit=$("<span class=\"tree-hit tree-expanded\"></span>").insertBefore(_a9);
if(hit.prev().length){
hit.prev().remove();
}
}
ul=_a7.next();
if(!ul.length){
ul=$("<ul></ul>").insertAfter(_a7);
}
}
_5b(_a5,ul[0],_a8,true);
_51(_a5,ul.prev());
};
function _aa(_ab,_ac){
var ref=_ac.before||_ac.after;
var _ad=_9a(_ab,ref);
var _ae=_ac.data;
if(!_ae){
return;
}
_ae=$.isArray(_ae)?_ae:[_ae];
if(!_ae.length){
return;
}
_a4(_ab,{parent:(_ad?_ad.target:null),data:_ae});
var _af=_ad?_ad.children:$(_ab).tree("getRoots");
for(var i=0;i<_af.length;i++){
if(_af[i].domId==$(ref).attr("id")){
for(var j=_ae.length-1;j>=0;j--){
_af.splice((_ac.before?i:(i+1)),0,_ae[j]);
}
_af.splice(_af.length-_ae.length,_ae.length);
break;
}
}
var li=$();
for(var i=0;i<_ae.length;i++){
li=li.add($("#"+_ae[i].domId).parent());
}
if(_ac.before){
li.insertBefore($(ref).parent());
}else{
li.insertAfter($(ref).parent());
}
};
function _b0(_b1,_b2){
var _b3=del(_b2);
$(_b2).parent().remove();
if(_b3){
if(!_b3.children||!_b3.children.length){
var _b4=$(_b3.target);
_b4.find(".tree-icon").removeClass("tree-folder").addClass("tree-file");
_b4.find(".tree-hit").remove();
$("<span class=\"tree-indent\"></span>").prependTo(_b4);
_b4.next().remove();
}
_64(_b1,_b3);
_51(_b1,_b3.target);
}
_6b(_b1,_b1);
function del(_b5){
var id=$(_b5).attr("id");
var _b6=_9a(_b1,_b5);
var cc=_b6?_b6.children:$.data(_b1,"tree").data;
for(var i=0;i<cc.length;i++){
if(cc[i].domId==id){
cc.splice(i,1);
break;
}
}
return _b6;
};
};
function _64(_b7,_b8){
var _b9=$.data(_b7,"tree").options;
var _ba=$(_b8.target);
var _bb=_c(_b7,_b8.target);
var _bc=_bb.checked;
if(_bb.iconCls){
_ba.find(".tree-icon").removeClass(_bb.iconCls);
}
$.extend(_bb,_b8);
_ba.find(".tree-title").html(_b9.formatter.call(_b7,_bb));
if(_bb.iconCls){
_ba.find(".tree-icon").addClass(_bb.iconCls);
}
if(_bc!=_bb.checked){
_34(_b7,_b8.target,_bb.checked);
}
};
function _bd(_be,_bf){
if(_bf){
var p=_9a(_be,_bf);
while(p){
_bf=p.target;
p=_9a(_be,_bf);
}
return _c(_be,_bf);
}else{
var _c0=_c1(_be);
return _c0.length?_c0[0]:null;
}
};
function _c1(_c2){
var _c3=$.data(_c2,"tree").data;
for(var i=0;i<_c3.length;i++){
_c4(_c3[i]);
}
return _c3;
};
function _5a(_c5,_c6){
var _c7=[];
var n=_c(_c5,_c6);
var _c8=n?(n.children||[]):$.data(_c5,"tree").data;
_68(_c8,function(_c9){
_c7.push(_c4(_c9));
});
return _c7;
};
function _9a(_ca,_cb){
var p=$(_cb).closest("ul").prevAll("div.tree-node:first");
return _c(_ca,p[0]);
};
function _cc(_cd,_ce){
_ce=_ce||"checked";
if(!$.isArray(_ce)){
_ce=[_ce];
}
var _cf=[];
for(var i=0;i<_ce.length;i++){
var s=_ce[i];
if(s=="checked"){
_cf.push("span.tree-checkbox1");
}else{
if(s=="unchecked"){
_cf.push("span.tree-checkbox0");
}else{
if(s=="indeterminate"){
_cf.push("span.tree-checkbox2");
}
}
}
}
var _d0=[];
$(_cd).find(_cf.join(",")).each(function(){
var _d1=$(this).parent();
_d0.push(_c(_cd,_d1[0]));
});
return _d0;
};
function _d2(_d3){
var _d4=$(_d3).find("div.tree-node-selected");
return _d4.length?_c(_d3,_d4[0]):null;
};
function _d5(_d6,_d7){
var _d8=_c(_d6,_d7);
if(_d8&&_d8.children){
_68(_d8.children,function(_d9){
_c4(_d9);
});
}
return _d8;
};
function _c(_da,_db){
return _63(_da,"domId",$(_db).attr("id"));
};
function _dc(_dd,id){
return _63(_dd,"id",id);
};
function _63(_de,_df,_e0){
var _e1=$.data(_de,"tree").data;
var _e2=null;
_68(_e1,function(_e3){
if(_e3[_df]==_e0){
_e2=_c4(_e3);
return false;
}
});
return _e2;
};
function _c4(_e4){
var d=$("#"+_e4.domId);
_e4.target=d[0];
_e4.checked=d.find(".tree-checkbox").hasClass("tree-checkbox1");
return _e4;
};
function _68(_e5,_e6){
var _e7=[];
for(var i=0;i<_e5.length;i++){
_e7.push(_e5[i]);
}
while(_e7.length){
var _e8=_e7.shift();
if(_e6(_e8)==false){
return;
}
if(_e8.children){
for(var i=_e8.children.length-1;i>=0;i--){
_e7.unshift(_e8.children[i]);
}
}
}
};
function _e9(_ea,_eb){
var _ec=$.data(_ea,"tree").options;
var _ed=_c(_ea,_eb);
if(_ec.onBeforeSelect.call(_ea,_ed)==false){
return;
}
$(_ea).find("div.tree-node-selected").removeClass("tree-node-selected");
$(_eb).addClass("tree-node-selected");
_ec.onSelect.call(_ea,_ed);
};
function _56(_ee,_ef){
return $(_ef).children("span.tree-hit").length==0;
};
function _f0(_f1,_f2){
var _f3=$.data(_f1,"tree").options;
var _f4=_c(_f1,_f2);
if(_f3.onBeforeEdit.call(_f1,_f4)==false){
return;
}
$(_f2).css("position","relative");
var nt=$(_f2).find(".tree-title");
var _f5=nt.outerWidth();
nt.empty();
var _f6=$("<input class=\"tree-editor\">").appendTo(nt);
_f6.val(_f4.text).focus();
_f6.width(_f5+20);
_f6.height(document.compatMode=="CSS1Compat"?(18-(_f6.outerHeight()-_f6.height())):18);
_f6.bind("click",function(e){
return false;
}).bind("mousedown",function(e){
e.stopPropagation();
}).bind("mousemove",function(e){
e.stopPropagation();
}).bind("keydown",function(e){
if(e.keyCode==13){
_f7(_f1,_f2);
return false;
}else{
if(e.keyCode==27){
_fd(_f1,_f2);
return false;
}
}
}).bind("blur",function(e){
e.stopPropagation();
_f7(_f1,_f2);
});
};
function _f7(_f8,_f9){
var _fa=$.data(_f8,"tree").options;
$(_f9).css("position","");
var _fb=$(_f9).find("input.tree-editor");
var val=_fb.val();
_fb.remove();
var _fc=_c(_f8,_f9);
_fc.text=val;
_64(_f8,_fc);
_fa.onAfterEdit.call(_f8,_fc);
};
function _fd(_fe,_ff){
var opts=$.data(_fe,"tree").options;
$(_ff).css("position","");
$(_ff).find("input.tree-editor").remove();
var node=_c(_fe,_ff);
_64(_fe,node);
opts.onCancelEdit.call(_fe,node);
};
function _100(_101,q){
var _102=$.data(_101,"tree");
var opts=_102.options;
var ids={};
_68(_102.data,function(node){
if(opts.filter.call(_101,q,node)){
$("#"+node.domId).removeClass("tree-node-hidden");
ids[node.domId]=1;
node.hidden=false;
}else{
$("#"+node.domId).addClass("tree-node-hidden");
node.hidden=true;
}
});
for(var id in ids){
_103(id);
}
function _103(_104){
var p=$(_101).tree("getParent",$("#"+_104)[0]);
while(p){
$(p.target).removeClass("tree-node-hidden");
p.hidden=false;
p=$(_101).tree("getParent",p.target);
}
};
};
$.fn.tree=function(_105,_106){
if(typeof _105=="string"){
return $.fn.tree.methods[_105](this,_106);
}
var _105=_105||{};
return this.each(function(){
var _107=$.data(this,"tree");
var opts;
if(_107){
opts=$.extend(_107.options,_105);
_107.options=opts;
}else{
opts=$.extend({},$.fn.tree.defaults,$.fn.tree.parseOptions(this),_105);
$.data(this,"tree",{options:opts,tree:_1(this),data:[]});
var data=$.fn.tree.parseData(this);
if(data.length){
_5b(this,this,data);
}
}
_4(this);
if(opts.data){
_5b(this,this,$.extend(true,[],opts.data));
}
_79(this,this);
});
};
$.fn.tree.methods={options:function(jq){
return $.data(jq[0],"tree").options;
},loadData:function(jq,data){
return jq.each(function(){
_5b(this,this,data);
});
},getNode:function(jq,_108){
return _c(jq[0],_108);
},getData:function(jq,_109){
return _d5(jq[0],_109);
},reload:function(jq,_10a){
return jq.each(function(){
if(_10a){
var node=$(_10a);
var hit=node.children("span.tree-hit");
hit.removeClass("tree-expanded tree-expanded-hover").addClass("tree-collapsed");
node.next().remove();
_83(this,_10a);
}else{
$(this).empty();
_79(this,this);
}
});
},getRoot:function(jq,_10b){
return _bd(jq[0],_10b);
},getRoots:function(jq){
return _c1(jq[0]);
},getParent:function(jq,_10c){
return _9a(jq[0],_10c);
},getChildren:function(jq,_10d){
return _5a(jq[0],_10d);
},getChecked:function(jq,_10e){
return _cc(jq[0],_10e);
},getSelected:function(jq){
return _d2(jq[0]);
},isLeaf:function(jq,_10f){
return _56(jq[0],_10f);
},find:function(jq,id){
return _dc(jq[0],id);
},select:function(jq,_110){
return jq.each(function(){
_e9(this,_110);
});
},check:function(jq,_111){
return jq.each(function(){
_34(this,_111,true);
});
},uncheck:function(jq,_112){
return jq.each(function(){
_34(this,_112,false);
});
},collapse:function(jq,_113){
return jq.each(function(){
_8a(this,_113);
});
},expand:function(jq,_114){
return jq.each(function(){
_83(this,_114);
});
},collapseAll:function(jq,_115){
return jq.each(function(){
_a0(this,_115);
});
},expandAll:function(jq,_116){
return jq.each(function(){
_92(this,_116);
});
},expandTo:function(jq,_117){
return jq.each(function(){
_96(this,_117);
});
},scrollTo:function(jq,_118){
return jq.each(function(){
_9b(this,_118);
});
},toggle:function(jq,_119){
return jq.each(function(){
_8f(this,_119);
});
},append:function(jq,_11a){
return jq.each(function(){
_a4(this,_11a);
});
},insert:function(jq,_11b){
return jq.each(function(){
_aa(this,_11b);
});
},remove:function(jq,_11c){
return jq.each(function(){
_b0(this,_11c);
});
},pop:function(jq,_11d){
var node=jq.tree("getData",_11d);
jq.tree("remove",_11d);
return node;
},update:function(jq,_11e){
return jq.each(function(){
_64(this,_11e);
});
},enableDnd:function(jq){
return jq.each(function(){
_11(this);
});
},disableDnd:function(jq){
return jq.each(function(){
_d(this);
});
},beginEdit:function(jq,_11f){
return jq.each(function(){
_f0(this,_11f);
});
},endEdit:function(jq,_120){
return jq.each(function(){
_f7(this,_120);
});
},cancelEdit:function(jq,_121){
return jq.each(function(){
_fd(this,_121);
});
},doFilter:function(jq,q){
return jq.each(function(){
_100(this,q);
});
}};
$.fn.tree.parseOptions=function(_122){
var t=$(_122);
return $.extend({},$.parser.parseOptions(_122,["url","method",{checkbox:"boolean",cascadeCheck:"boolean",onlyLeafCheck:"boolean"},{animate:"boolean",lines:"boolean",dnd:"boolean"}]));
};
$.fn.tree.parseData=function(_123){
var data=[];
_124(data,$(_123));
return data;
function _124(aa,tree){
tree.children("li").each(function(){
var node=$(this);
var item=$.extend({},$.parser.parseOptions(this,["id","iconCls","state"]),{checked:(node.attr("checked")?true:undefined)});
item.text=node.children("span").html();
if(!item.text){
item.text=node.html();
}
var _125=node.children("ul");
if(_125.length){
item.children=[];
_124(item.children,_125);
}
aa.push(item);
});
};
};
var _126=1;
var _127={render:function(_128,ul,data){
var opts=$.data(_128,"tree").options;
var _129=$(ul).prev("div.tree-node").find("span.tree-indent, span.tree-hit").length;
var cc=_12a(_129,data);
$(ul).append(cc.join(""));
function _12a(_12b,_12c){
var cc=[];
for(var i=0;i<_12c.length;i++){
var item=_12c[i];
if(item.state!="open"&&item.state!="closed"){
item.state="open";
}
item.domId="_easyui_tree_"+_126++;
cc.push("<li>");
cc.push("<div id=\""+item.domId+"\" class=\"tree-node\">");
for(var j=0;j<_12b;j++){
cc.push("<span class=\"tree-indent\"></span>");
}
var _12d=false;
if(item.state=="closed"){
cc.push("<span class=\"tree-hit tree-collapsed\"></span>");
cc.push("<span class=\"tree-icon tree-folder "+(item.iconCls?item.iconCls:"")+"\"></span>");
}else{
if(item.children&&item.children.length){
cc.push("<span class=\"tree-hit tree-expanded\"></span>");
cc.push("<span class=\"tree-icon tree-folder tree-folder-open "+(item.iconCls?item.iconCls:"")+"\"></span>");
}else{
cc.push("<span class=\"tree-indent\"></span>");
cc.push("<span class=\"tree-icon tree-file "+(item.iconCls?item.iconCls:"")+"\"></span>");
_12d=true;
}
}
if(opts.checkbox){
if((!opts.onlyLeafCheck)||_12d){
cc.push("<span class=\"tree-checkbox tree-checkbox0\"></span>");
}
}
cc.push("<span class=\"tree-title\">"+opts.formatter.call(_128,item)+"</span>");
cc.push("</div>");
if(item.children&&item.children.length){
var tmp=_12a(_12b+1,item.children);
cc.push("<ul style=\"display:"+(item.state=="closed"?"none":"block")+"\">");
cc=cc.concat(tmp);
cc.push("</ul>");
}
cc.push("</li>");
}
return cc;
};
}};
$.fn.tree.defaults={url:null,method:"post",animate:false,checkbox:false,cascadeCheck:true,onlyLeafCheck:false,lines:false,dnd:false,data:null,queryParams:{},formatter:function(node){
return node.text;
},filter:function(q,node){
return node.text.toLowerCase().indexOf(q.toLowerCase())>=0;
},loader:function(_12e,_12f,_130){
var opts=$(this).tree("options");
if(!opts.url){
return false;
}
$.ajax({type:opts.method,url:opts.url,data:_12e,dataType:"json",success:function(data){
_12f(data);
},error:function(){
_130.apply(this,arguments);
}});
},loadFilter:function(data,_131){
return data;
},view:_127,onBeforeLoad:function(node,_132){
},onLoadSuccess:function(node,data){
},onLoadError:function(){
},onClick:function(node){
},onDblClick:function(node){
},onBeforeExpand:function(node){
},onExpand:function(node){
},onBeforeCollapse:function(node){
},onCollapse:function(node){
},onBeforeCheck:function(node,_133){
},onCheck:function(node,_134){
},onBeforeSelect:function(node){
},onSelect:function(node){
},onContextMenu:function(e,node){
},onBeforeDrag:function(node){
},onStartDrag:function(node){
},onStopDrag:function(node){
},onDragEnter:function(_135,_136){
},onDragOver:function(_137,_138){
},onDragLeave:function(_139,_13a){
},onBeforeDrop:function(_13b,_13c,_13d){
},onDrop:function(_13e,_13f,_140){
},onBeforeEdit:function(node){
},onAfterEdit:function(node){
},onCancelEdit:function(node){
}};
})(jQuery);

