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
var _3=$.data(_2,"treegrid");
var _4=_3.options;
$(_2).datagrid($.extend({},_4,{url:null,data:null,loader:function(){
return false;
},onBeforeLoad:function(){
return false;
},onLoadSuccess:function(){
},onResizeColumn:function(_5,_6){
_16(_2);
_4.onResizeColumn.call(_2,_5,_6);
},onBeforeSortColumn:function(_7,_8){
if(_4.onBeforeSortColumn.call(_2,_7,_8)==false){
return false;
}
},onSortColumn:function(_9,_a){
_4.sortName=_9;
_4.sortOrder=_a;
if(_4.remoteSort){
_15(_2);
}else{
var _b=$(_2).treegrid("getData");
_2f(_2,0,_b);
}
_4.onSortColumn.call(_2,_9,_a);
},onClickCell:function(_c,_d){
_4.onClickCell.call(_2,_d,_37(_2,_c));
},onDblClickCell:function(_e,_f){
_4.onDblClickCell.call(_2,_f,_37(_2,_e));
},onRowContextMenu:function(e,_10){
_4.onContextMenu.call(_2,e,_37(_2,_10));
}}));
var _11=$.data(_2,"datagrid").options;
_4.columns=_11.columns;
_4.frozenColumns=_11.frozenColumns;
_3.dc=$.data(_2,"datagrid").dc;
if(_4.pagination){
var _12=$(_2).datagrid("getPager");
_12.pagination({pageNumber:_4.pageNumber,pageSize:_4.pageSize,pageList:_4.pageList,onSelectPage:function(_13,_14){
_4.pageNumber=_13;
_4.pageSize=_14;
_15(_2);
}});
_4.pageSize=_12.pagination("options").pageSize;
}
};
function _16(_17,_18){
var _19=$.data(_17,"datagrid").options;
var dc=$.data(_17,"datagrid").dc;
if(!dc.body1.is(":empty")&&(!_19.nowrap||_19.autoRowHeight)){
if(_18!=undefined){
var _1a=_1b(_17,_18);
for(var i=0;i<_1a.length;i++){
_1c(_1a[i][_19.idField]);
}
}
}
$(_17).datagrid("fixRowHeight",_18);
function _1c(_1d){
var tr1=_19.finder.getTr(_17,_1d,"body",1);
var tr2=_19.finder.getTr(_17,_1d,"body",2);
tr1.css("height","");
tr2.css("height","");
var _1e=Math.max(tr1.height(),tr2.height());
tr1.css("height",_1e);
tr2.css("height",_1e);
};
};
function _1f(_20){
var dc=$.data(_20,"datagrid").dc;
var _21=$.data(_20,"treegrid").options;
if(!_21.rownumbers){
return;
}
dc.body1.find("div.datagrid-cell-rownumber").each(function(i){
$(this).html(i+1);
});
};
function _22(_23){
return function(e){
$.fn.datagrid.defaults.rowEvents[_23?"mouseover":"mouseout"](e);
var tt=$(e.target);
var fn=_23?"addClass":"removeClass";
if(tt.hasClass("tree-hit")){
tt.hasClass("tree-expanded")?tt[fn]("tree-expanded-hover"):tt[fn]("tree-collapsed-hover");
}
};
};
function _24(e){
var tt=$(e.target);
if(tt.hasClass("tree-hit")){
var tr=tt.closest("tr.datagrid-row");
var _25=tr.closest("div.datagrid-view").children(".datagrid-f")[0];
_26(_25,tr.attr("node-id"));
}else{
$.fn.datagrid.defaults.rowEvents.click(e);
}
};
function _27(_28,_29){
var _2a=$.data(_28,"treegrid").options;
var tr1=_2a.finder.getTr(_28,_29,"body",1);
var tr2=_2a.finder.getTr(_28,_29,"body",2);
var _2b=$(_28).datagrid("getColumnFields",true).length+(_2a.rownumbers?1:0);
var _2c=$(_28).datagrid("getColumnFields",false).length;
_2d(tr1,_2b);
_2d(tr2,_2c);
function _2d(tr,_2e){
$("<tr class=\"treegrid-tr-tree\">"+"<td style=\"border:0px\" colspan=\""+_2e+"\">"+"<div></div>"+"</td>"+"</tr>").insertAfter(tr);
};
};
function _2f(_30,_31,_32,_33){
var _34=$.data(_30,"treegrid");
var _35=_34.options;
var dc=_34.dc;
_32=_35.loadFilter.call(_30,_32,_31);
var _36=_37(_30,_31);
if(_36){
var _38=_35.finder.getTr(_30,_31,"body",1);
var _39=_35.finder.getTr(_30,_31,"body",2);
var cc1=_38.next("tr.treegrid-tr-tree").children("td").children("div");
var cc2=_39.next("tr.treegrid-tr-tree").children("td").children("div");
if(!_33){
_36.children=[];
}
}else{
var cc1=dc.body1;
var cc2=dc.body2;
if(!_33){
_34.data=[];
}
}
if(!_33){
cc1.empty();
cc2.empty();
}
if(_35.view.onBeforeRender){
_35.view.onBeforeRender.call(_35.view,_30,_31,_32);
}
_35.view.render.call(_35.view,_30,cc1,true);
_35.view.render.call(_35.view,_30,cc2,false);
if(_35.showFooter){
_35.view.renderFooter.call(_35.view,_30,dc.footer1,true);
_35.view.renderFooter.call(_35.view,_30,dc.footer2,false);
}
if(_35.view.onAfterRender){
_35.view.onAfterRender.call(_35.view,_30);
}
if(!_31&&_35.pagination){
var _3a=$.data(_30,"treegrid").total;
var _3b=$(_30).datagrid("getPager");
if(_3b.pagination("options").total!=_3a){
_3b.pagination({total:_3a});
}
}
_16(_30);
_1f(_30);
$(_30).treegrid("showLines");
$(_30).treegrid("setSelectionState");
$(_30).treegrid("autoSizeColumn");
_35.onLoadSuccess.call(_30,_36,_32);
};
function _15(_3c,_3d,_3e,_3f,_40){
var _41=$.data(_3c,"treegrid").options;
var _42=$(_3c).datagrid("getPanel").find("div.datagrid-body");
if(_3e){
_41.queryParams=_3e;
}
var _43=$.extend({},_41.queryParams);
if(_41.pagination){
$.extend(_43,{page:_41.pageNumber,rows:_41.pageSize});
}
if(_41.sortName){
$.extend(_43,{sort:_41.sortName,order:_41.sortOrder});
}
var row=_37(_3c,_3d);
if(_41.onBeforeLoad.call(_3c,row,_43)==false){
return;
}
var _44=_42.find("tr[node-id=\""+_3d+"\"] span.tree-folder");
_44.addClass("tree-loading");
$(_3c).treegrid("loading");
var _45=_41.loader.call(_3c,_43,function(_46){
_44.removeClass("tree-loading");
$(_3c).treegrid("loaded");
_2f(_3c,_3d,_46,_3f);
if(_40){
_40();
}
},function(){
_44.removeClass("tree-loading");
$(_3c).treegrid("loaded");
_41.onLoadError.apply(_3c,arguments);
if(_40){
_40();
}
});
if(_45==false){
_44.removeClass("tree-loading");
$(_3c).treegrid("loaded");
}
};
function _47(_48){
var _49=_4a(_48);
if(_49.length){
return _49[0];
}else{
return null;
}
};
function _4a(_4b){
return $.data(_4b,"treegrid").data;
};
function _4c(_4d,_4e){
var row=_37(_4d,_4e);
if(row._parentId){
return _37(_4d,row._parentId);
}else{
return null;
}
};
function _1b(_4f,_50){
var _51=$.data(_4f,"treegrid").options;
var _52=$(_4f).datagrid("getPanel").find("div.datagrid-view2 div.datagrid-body");
var _53=[];
if(_50){
_54(_50);
}else{
var _55=_4a(_4f);
for(var i=0;i<_55.length;i++){
_53.push(_55[i]);
_54(_55[i][_51.idField]);
}
}
function _54(_56){
var _57=_37(_4f,_56);
if(_57&&_57.children){
for(var i=0,len=_57.children.length;i<len;i++){
var _58=_57.children[i];
_53.push(_58);
_54(_58[_51.idField]);
}
}
};
return _53;
};
function _59(_5a,_5b){
var _5c=$.data(_5a,"treegrid").options;
var tr=_5c.finder.getTr(_5a,_5b);
var _5d=tr.children("td[field=\""+_5c.treeField+"\"]");
return _5d.find("span.tree-indent,span.tree-hit").length;
};
function _37(_5e,_5f){
var _60=$.data(_5e,"treegrid").options;
var _61=$.data(_5e,"treegrid").data;
var cc=[_61];
while(cc.length){
var c=cc.shift();
for(var i=0;i<c.length;i++){
var _62=c[i];
if(_62[_60.idField]==_5f){
return _62;
}else{
if(_62["children"]){
cc.push(_62["children"]);
}
}
}
}
return null;
};
function _63(_64,_65){
var _66=$.data(_64,"treegrid").options;
var row=_37(_64,_65);
var tr=_66.finder.getTr(_64,_65);
var hit=tr.find("span.tree-hit");
if(hit.length==0){
return;
}
if(hit.hasClass("tree-collapsed")){
return;
}
if(_66.onBeforeCollapse.call(_64,row)==false){
return;
}
hit.removeClass("tree-expanded tree-expanded-hover").addClass("tree-collapsed");
hit.next().removeClass("tree-folder-open");
row.state="closed";
tr=tr.next("tr.treegrid-tr-tree");
var cc=tr.children("td").children("div");
if(_66.animate){
cc.slideUp("normal",function(){
$(_64).treegrid("autoSizeColumn");
_16(_64,_65);
_66.onCollapse.call(_64,row);
});
}else{
cc.hide();
$(_64).treegrid("autoSizeColumn");
_16(_64,_65);
_66.onCollapse.call(_64,row);
}
};
function _67(_68,_69){
var _6a=$.data(_68,"treegrid").options;
var tr=_6a.finder.getTr(_68,_69);
var hit=tr.find("span.tree-hit");
var row=_37(_68,_69);
if(hit.length==0){
return;
}
if(hit.hasClass("tree-expanded")){
return;
}
if(_6a.onBeforeExpand.call(_68,row)==false){
return;
}
hit.removeClass("tree-collapsed tree-collapsed-hover").addClass("tree-expanded");
hit.next().addClass("tree-folder-open");
var _6b=tr.next("tr.treegrid-tr-tree");
if(_6b.length){
var cc=_6b.children("td").children("div");
_6c(cc);
}else{
_27(_68,row[_6a.idField]);
var _6b=tr.next("tr.treegrid-tr-tree");
var cc=_6b.children("td").children("div");
cc.hide();
var _6d=$.extend({},_6a.queryParams||{});
_6d.id=row[_6a.idField];
_15(_68,row[_6a.idField],_6d,true,function(){
if(cc.is(":empty")){
_6b.remove();
}else{
_6c(cc);
}
});
}
function _6c(cc){
row.state="open";
if(_6a.animate){
cc.slideDown("normal",function(){
$(_68).treegrid("autoSizeColumn");
_16(_68,_69);
_6a.onExpand.call(_68,row);
});
}else{
cc.show();
$(_68).treegrid("autoSizeColumn");
_16(_68,_69);
_6a.onExpand.call(_68,row);
}
};
};
function _26(_6e,_6f){
var _70=$.data(_6e,"treegrid").options;
var tr=_70.finder.getTr(_6e,_6f);
var hit=tr.find("span.tree-hit");
if(hit.hasClass("tree-expanded")){
_63(_6e,_6f);
}else{
_67(_6e,_6f);
}
};
function _71(_72,_73){
var _74=$.data(_72,"treegrid").options;
var _75=_1b(_72,_73);
if(_73){
_75.unshift(_37(_72,_73));
}
for(var i=0;i<_75.length;i++){
_63(_72,_75[i][_74.idField]);
}
};
function _76(_77,_78){
var _79=$.data(_77,"treegrid").options;
var _7a=_1b(_77,_78);
if(_78){
_7a.unshift(_37(_77,_78));
}
for(var i=0;i<_7a.length;i++){
_67(_77,_7a[i][_79.idField]);
}
};
function _7b(_7c,_7d){
var _7e=$.data(_7c,"treegrid").options;
var ids=[];
var p=_4c(_7c,_7d);
while(p){
var id=p[_7e.idField];
ids.unshift(id);
p=_4c(_7c,id);
}
for(var i=0;i<ids.length;i++){
_67(_7c,ids[i]);
}
};
function _7f(_80,_81){
var _82=$.data(_80,"treegrid").options;
if(_81.parent){
var tr=_82.finder.getTr(_80,_81.parent);
if(tr.next("tr.treegrid-tr-tree").length==0){
_27(_80,_81.parent);
}
var _83=tr.children("td[field=\""+_82.treeField+"\"]").children("div.datagrid-cell");
var _84=_83.children("span.tree-icon");
if(_84.hasClass("tree-file")){
_84.removeClass("tree-file").addClass("tree-folder tree-folder-open");
var hit=$("<span class=\"tree-hit tree-expanded\"></span>").insertBefore(_84);
if(hit.prev().length){
hit.prev().remove();
}
}
}
_2f(_80,_81.parent,_81.data,true);
};
function _85(_86,_87){
var ref=_87.before||_87.after;
var _88=$.data(_86,"treegrid").options;
var _89=_4c(_86,ref);
_7f(_86,{parent:(_89?_89[_88.idField]:null),data:[_87.data]});
var _8a=_89?_89.children:$(_86).treegrid("getRoots");
for(var i=0;i<_8a.length;i++){
if(_8a[i][_88.idField]==ref){
var _8b=_8a[_8a.length-1];
_8a.splice(_87.before?i:(i+1),0,_8b);
_8a.splice(_8a.length-1,1);
break;
}
}
_8c(true);
_8c(false);
_1f(_86);
$(_86).treegrid("showLines");
function _8c(_8d){
var _8e=_8d?1:2;
var tr=_88.finder.getTr(_86,_87.data[_88.idField],"body",_8e);
var _8f=tr.closest("table.datagrid-btable");
tr=tr.parent().children();
var _90=_88.finder.getTr(_86,ref,"body",_8e);
if(_87.before){
tr.insertBefore(_90);
}else{
var sub=_90.next("tr.treegrid-tr-tree");
tr.insertAfter(sub.length?sub:_90);
}
_8f.remove();
};
};
function _91(_92,_93){
var _94=$.data(_92,"treegrid");
$(_92).datagrid("deleteRow",_93);
_1f(_92);
_94.total-=1;
$(_92).datagrid("getPager").pagination("refresh",{total:_94.total});
$(_92).treegrid("showLines");
};
function _95(_96){
var t=$(_96);
var _97=t.treegrid("options");
if(_97.lines){
t.treegrid("getPanel").addClass("tree-lines");
}else{
t.treegrid("getPanel").removeClass("tree-lines");
return;
}
t.treegrid("getPanel").find("span.tree-indent").removeClass("tree-line tree-join tree-joinbottom");
t.treegrid("getPanel").find("div.datagrid-cell").removeClass("tree-node-last tree-root-first tree-root-one");
var _98=t.treegrid("getRoots");
if(_98.length>1){
_99(_98[0]).addClass("tree-root-first");
}else{
if(_98.length==1){
_99(_98[0]).addClass("tree-root-one");
}
}
_9a(_98);
_9b(_98);
function _9a(_9c){
$.map(_9c,function(_9d){
if(_9d.children&&_9d.children.length){
_9a(_9d.children);
}else{
var _9e=_99(_9d);
_9e.find(".tree-icon").prev().addClass("tree-join");
}
});
if(_9c.length){
var _9f=_99(_9c[_9c.length-1]);
_9f.addClass("tree-node-last");
_9f.find(".tree-join").removeClass("tree-join").addClass("tree-joinbottom");
}
};
function _9b(_a0){
$.map(_a0,function(_a1){
if(_a1.children&&_a1.children.length){
_9b(_a1.children);
}
});
for(var i=0;i<_a0.length-1;i++){
var _a2=_a0[i];
var _a3=t.treegrid("getLevel",_a2[_97.idField]);
var tr=_97.finder.getTr(_96,_a2[_97.idField]);
var cc=tr.next().find("tr.datagrid-row td[field=\""+_97.treeField+"\"] div.datagrid-cell");
cc.find("span:eq("+(_a3-1)+")").addClass("tree-line");
}
};
function _99(_a4){
var tr=_97.finder.getTr(_96,_a4[_97.idField]);
var _a5=tr.find("td[field=\""+_97.treeField+"\"] div.datagrid-cell");
return _a5;
};
};
$.fn.treegrid=function(_a6,_a7){
if(typeof _a6=="string"){
var _a8=$.fn.treegrid.methods[_a6];
if(_a8){
return _a8(this,_a7);
}else{
return this.datagrid(_a6,_a7);
}
}
_a6=_a6||{};
return this.each(function(){
var _a9=$.data(this,"treegrid");
if(_a9){
$.extend(_a9.options,_a6);
}else{
_a9=$.data(this,"treegrid",{options:$.extend({},$.fn.treegrid.defaults,$.fn.treegrid.parseOptions(this),_a6),data:[]});
}
_1(this);
if(_a9.options.data){
$(this).treegrid("loadData",_a9.options.data);
}
_15(this);
});
};
$.fn.treegrid.methods={options:function(jq){
return $.data(jq[0],"treegrid").options;
},resize:function(jq,_aa){
return jq.each(function(){
$(this).datagrid("resize",_aa);
});
},fixRowHeight:function(jq,_ab){
return jq.each(function(){
_16(this,_ab);
});
},loadData:function(jq,_ac){
return jq.each(function(){
_2f(this,_ac.parent,_ac);
});
},load:function(jq,_ad){
return jq.each(function(){
$(this).treegrid("options").pageNumber=1;
$(this).treegrid("getPager").pagination({pageNumber:1});
$(this).treegrid("reload",_ad);
});
},reload:function(jq,id){
return jq.each(function(){
var _ae=$(this).treegrid("options");
var _af={};
if(typeof id=="object"){
_af=id;
}else{
_af=$.extend({},_ae.queryParams);
_af.id=id;
}
if(_af.id){
var _b0=$(this).treegrid("find",_af.id);
if(_b0.children){
_b0.children.splice(0,_b0.children.length);
}
_ae.queryParams=_af;
var tr=_ae.finder.getTr(this,_af.id);
tr.next("tr.treegrid-tr-tree").remove();
tr.find("span.tree-hit").removeClass("tree-expanded tree-expanded-hover").addClass("tree-collapsed");
_67(this,_af.id);
}else{
_15(this,null,_af);
}
});
},reloadFooter:function(jq,_b1){
return jq.each(function(){
var _b2=$.data(this,"treegrid").options;
var dc=$.data(this,"datagrid").dc;
if(_b1){
$.data(this,"treegrid").footer=_b1;
}
if(_b2.showFooter){
_b2.view.renderFooter.call(_b2.view,this,dc.footer1,true);
_b2.view.renderFooter.call(_b2.view,this,dc.footer2,false);
if(_b2.view.onAfterRender){
_b2.view.onAfterRender.call(_b2.view,this);
}
$(this).treegrid("fixRowHeight");
}
});
},getData:function(jq){
return $.data(jq[0],"treegrid").data;
},getFooterRows:function(jq){
return $.data(jq[0],"treegrid").footer;
},getRoot:function(jq){
return _47(jq[0]);
},getRoots:function(jq){
return _4a(jq[0]);
},getParent:function(jq,id){
return _4c(jq[0],id);
},getChildren:function(jq,id){
return _1b(jq[0],id);
},getLevel:function(jq,id){
return _59(jq[0],id);
},find:function(jq,id){
return _37(jq[0],id);
},isLeaf:function(jq,id){
var _b3=$.data(jq[0],"treegrid").options;
var tr=_b3.finder.getTr(jq[0],id);
var hit=tr.find("span.tree-hit");
return hit.length==0;
},select:function(jq,id){
return jq.each(function(){
$(this).datagrid("selectRow",id);
});
},unselect:function(jq,id){
return jq.each(function(){
$(this).datagrid("unselectRow",id);
});
},collapse:function(jq,id){
return jq.each(function(){
_63(this,id);
});
},expand:function(jq,id){
return jq.each(function(){
_67(this,id);
});
},toggle:function(jq,id){
return jq.each(function(){
_26(this,id);
});
},collapseAll:function(jq,id){
return jq.each(function(){
_71(this,id);
});
},expandAll:function(jq,id){
return jq.each(function(){
_76(this,id);
});
},expandTo:function(jq,id){
return jq.each(function(){
_7b(this,id);
});
},append:function(jq,_b4){
return jq.each(function(){
_7f(this,_b4);
});
},insert:function(jq,_b5){
return jq.each(function(){
_85(this,_b5);
});
},remove:function(jq,id){
return jq.each(function(){
_91(this,id);
});
},pop:function(jq,id){
var row=jq.treegrid("find",id);
jq.treegrid("remove",id);
return row;
},refresh:function(jq,id){
return jq.each(function(){
var _b6=$.data(this,"treegrid").options;
_b6.view.refreshRow.call(_b6.view,this,id);
});
},update:function(jq,_b7){
return jq.each(function(){
var _b8=$.data(this,"treegrid").options;
_b8.view.updateRow.call(_b8.view,this,_b7.id,_b7.row);
});
},beginEdit:function(jq,id){
return jq.each(function(){
$(this).datagrid("beginEdit",id);
$(this).treegrid("fixRowHeight",id);
});
},endEdit:function(jq,id){
return jq.each(function(){
$(this).datagrid("endEdit",id);
});
},cancelEdit:function(jq,id){
return jq.each(function(){
$(this).datagrid("cancelEdit",id);
});
},showLines:function(jq){
return jq.each(function(){
_95(this);
});
}};
$.fn.treegrid.parseOptions=function(_b9){
return $.extend({},$.fn.datagrid.parseOptions(_b9),$.parser.parseOptions(_b9,["treeField",{animate:"boolean"}]));
};
var _ba=$.extend({},$.fn.datagrid.defaults.view,{render:function(_bb,_bc,_bd){
var _be=$.data(_bb,"treegrid").options;
var _bf=$(_bb).datagrid("getColumnFields",_bd);
var _c0=$.data(_bb,"datagrid").rowIdPrefix;
if(_bd){
if(!(_be.rownumbers||(_be.frozenColumns&&_be.frozenColumns.length))){
return;
}
}
var _c1=this;
if(this.treeNodes&&this.treeNodes.length){
var _c2=_c3(_bd,this.treeLevel,this.treeNodes);
$(_bc).append(_c2.join(""));
}
function _c3(_c4,_c5,_c6){
var _c7=$(_bb).treegrid("getParent",_c6[0][_be.idField]);
var _c8=(_c7?_c7.children.length:$(_bb).treegrid("getRoots").length)-_c6.length;
var _c9=["<table class=\"datagrid-btable\" cellspacing=\"0\" cellpadding=\"0\" border=\"0\"><tbody>"];
for(var i=0;i<_c6.length;i++){
var row=_c6[i];
if(row.state!="open"&&row.state!="closed"){
row.state="open";
}
var css=_be.rowStyler?_be.rowStyler.call(_bb,row):"";
var _ca="";
var _cb="";
if(typeof css=="string"){
_cb=css;
}else{
if(css){
_ca=css["class"]||"";
_cb=css["style"]||"";
}
}
var cls="class=\"datagrid-row "+(_c8++%2&&_be.striped?"datagrid-row-alt ":" ")+_ca+"\"";
var _cc=_cb?"style=\""+_cb+"\"":"";
var _cd=_c0+"-"+(_c4?1:2)+"-"+row[_be.idField];
_c9.push("<tr id=\""+_cd+"\" node-id=\""+row[_be.idField]+"\" "+cls+" "+_cc+">");
_c9=_c9.concat(_c1.renderRow.call(_c1,_bb,_bf,_c4,_c5,row));
_c9.push("</tr>");
if(row.children&&row.children.length){
var tt=_c3(_c4,_c5+1,row.children);
var v=row.state=="closed"?"none":"block";
_c9.push("<tr class=\"treegrid-tr-tree\"><td style=\"border:0px\" colspan="+(_bf.length+(_be.rownumbers?1:0))+"><div style=\"display:"+v+"\">");
_c9=_c9.concat(tt);
_c9.push("</div></td></tr>");
}
}
_c9.push("</tbody></table>");
return _c9;
};
},renderFooter:function(_ce,_cf,_d0){
var _d1=$.data(_ce,"treegrid").options;
var _d2=$.data(_ce,"treegrid").footer||[];
var _d3=$(_ce).datagrid("getColumnFields",_d0);
var _d4=["<table class=\"datagrid-ftable\" cellspacing=\"0\" cellpadding=\"0\" border=\"0\"><tbody>"];
for(var i=0;i<_d2.length;i++){
var row=_d2[i];
row[_d1.idField]=row[_d1.idField]||("foot-row-id"+i);
_d4.push("<tr class=\"datagrid-row\" node-id=\""+row[_d1.idField]+"\">");
_d4.push(this.renderRow.call(this,_ce,_d3,_d0,0,row));
_d4.push("</tr>");
}
_d4.push("</tbody></table>");
$(_cf).html(_d4.join(""));
},renderRow:function(_d5,_d6,_d7,_d8,row){
var _d9=$.data(_d5,"treegrid").options;
var cc=[];
if(_d7&&_d9.rownumbers){
cc.push("<td class=\"datagrid-td-rownumber\"><div class=\"datagrid-cell-rownumber\">0</div></td>");
}
for(var i=0;i<_d6.length;i++){
var _da=_d6[i];
var col=$(_d5).datagrid("getColumnOption",_da);
if(col){
var css=col.styler?(col.styler(row[_da],row)||""):"";
var _db="";
var _dc="";
if(typeof css=="string"){
_dc=css;
}else{
if(cc){
_db=css["class"]||"";
_dc=css["style"]||"";
}
}
var cls=_db?"class=\""+_db+"\"":"";
var _dd=col.hidden?"style=\"display:none;"+_dc+"\"":(_dc?"style=\""+_dc+"\"":"");
cc.push("<td field=\""+_da+"\" "+cls+" "+_dd+">");
var _dd="";
if(!col.checkbox){
if(col.align){
_dd+="text-align:"+col.align+";";
}
if(!_d9.nowrap){
_dd+="white-space:normal;height:auto;";
}else{
if(_d9.autoRowHeight){
_dd+="height:auto;";
}
}
}
cc.push("<div style=\""+_dd+"\" ");
if(col.checkbox){
cc.push("class=\"datagrid-cell-check ");
}else{
cc.push("class=\"datagrid-cell "+col.cellClass);
}
cc.push("\">");
if(col.checkbox){
if(row.checked){
cc.push("<input type=\"checkbox\" checked=\"checked\"");
}else{
cc.push("<input type=\"checkbox\"");
}
cc.push(" name=\""+_da+"\" value=\""+(row[_da]!=undefined?row[_da]:"")+"\">");
}else{
var val=null;
if(col.formatter){
val=col.formatter(row[_da],row);
}else{
val=row[_da];
}
if(_da==_d9.treeField){
for(var j=0;j<_d8;j++){
cc.push("<span class=\"tree-indent\"></span>");
}
if(row.state=="closed"){
cc.push("<span class=\"tree-hit tree-collapsed\"></span>");
cc.push("<span class=\"tree-icon tree-folder "+(row.iconCls?row.iconCls:"")+"\"></span>");
}else{
if(row.children&&row.children.length){
cc.push("<span class=\"tree-hit tree-expanded\"></span>");
cc.push("<span class=\"tree-icon tree-folder tree-folder-open "+(row.iconCls?row.iconCls:"")+"\"></span>");
}else{
cc.push("<span class=\"tree-indent\"></span>");
cc.push("<span class=\"tree-icon tree-file "+(row.iconCls?row.iconCls:"")+"\"></span>");
}
}
cc.push("<span class=\"tree-title\">"+val+"</span>");
}else{
cc.push(val);
}
}
cc.push("</div>");
cc.push("</td>");
}
}
return cc.join("");
},refreshRow:function(_de,id){
this.updateRow.call(this,_de,id,{});
},updateRow:function(_df,id,row){
var _e0=$.data(_df,"treegrid").options;
var _e1=$(_df).treegrid("find",id);
$.extend(_e1,row);
var _e2=$(_df).treegrid("getLevel",id)-1;
var _e3=_e0.rowStyler?_e0.rowStyler.call(_df,_e1):"";
var _e4=$.data(_df,"datagrid").rowIdPrefix;
var _e5=_e1[_e0.idField];
function _e6(_e7){
var _e8=$(_df).treegrid("getColumnFields",_e7);
var tr=_e0.finder.getTr(_df,id,"body",(_e7?1:2));
var _e9=tr.find("div.datagrid-cell-rownumber").html();
var _ea=tr.find("div.datagrid-cell-check input[type=checkbox]").is(":checked");
tr.html(this.renderRow(_df,_e8,_e7,_e2,_e1));
tr.attr("style",_e3||"");
tr.find("div.datagrid-cell-rownumber").html(_e9);
if(_ea){
tr.find("div.datagrid-cell-check input[type=checkbox]")._propAttr("checked",true);
}
if(_e5!=id){
tr.attr("id",_e4+"-"+(_e7?1:2)+"-"+_e5);
tr.attr("node-id",_e5);
}
};
_e6.call(this,true);
_e6.call(this,false);
$(_df).treegrid("fixRowHeight",id);
},deleteRow:function(_eb,id){
var _ec=$.data(_eb,"treegrid").options;
var tr=_ec.finder.getTr(_eb,id);
tr.next("tr.treegrid-tr-tree").remove();
tr.remove();
var _ed=del(id);
if(_ed){
if(_ed.children.length==0){
tr=_ec.finder.getTr(_eb,_ed[_ec.idField]);
tr.next("tr.treegrid-tr-tree").remove();
var _ee=tr.children("td[field=\""+_ec.treeField+"\"]").children("div.datagrid-cell");
_ee.find(".tree-icon").removeClass("tree-folder").addClass("tree-file");
_ee.find(".tree-hit").remove();
$("<span class=\"tree-indent\"></span>").prependTo(_ee);
}
}
function del(id){
var cc;
var _ef=$(_eb).treegrid("getParent",id);
if(_ef){
cc=_ef.children;
}else{
cc=$(_eb).treegrid("getData");
}
for(var i=0;i<cc.length;i++){
if(cc[i][_ec.idField]==id){
cc.splice(i,1);
break;
}
}
return _ef;
};
},onBeforeRender:function(_f0,_f1,_f2){
if($.isArray(_f1)){
_f2={total:_f1.length,rows:_f1};
_f1=null;
}
if(!_f2){
return false;
}
var _f3=$.data(_f0,"treegrid");
var _f4=_f3.options;
if(_f2.length==undefined){
if(_f2.footer){
_f3.footer=_f2.footer;
}
if(_f2.total){
_f3.total=_f2.total;
}
_f2=this.transfer(_f0,_f1,_f2.rows);
}else{
function _f5(_f6,_f7){
for(var i=0;i<_f6.length;i++){
var row=_f6[i];
row._parentId=_f7;
if(row.children&&row.children.length){
_f5(row.children,row[_f4.idField]);
}
}
};
_f5(_f2,_f1);
}
var _f8=_37(_f0,_f1);
if(_f8){
if(_f8.children){
_f8.children=_f8.children.concat(_f2);
}else{
_f8.children=_f2;
}
}else{
_f3.data=_f3.data.concat(_f2);
}
this.sort(_f0,_f2);
this.treeNodes=_f2;
this.treeLevel=$(_f0).treegrid("getLevel",_f1);
},sort:function(_f9,_fa){
var _fb=$.data(_f9,"treegrid").options;
if(!_fb.remoteSort&&_fb.sortName){
var _fc=_fb.sortName.split(",");
var _fd=_fb.sortOrder.split(",");
_fe(_fa);
}
function _fe(_ff){
_ff.sort(function(r1,r2){
var r=0;
for(var i=0;i<_fc.length;i++){
var sn=_fc[i];
var so=_fd[i];
var col=$(_f9).treegrid("getColumnOption",sn);
var _100=col.sorter||function(a,b){
return a==b?0:(a>b?1:-1);
};
r=_100(r1[sn],r2[sn])*(so=="asc"?1:-1);
if(r!=0){
return r;
}
}
return r;
});
for(var i=0;i<_ff.length;i++){
var _101=_ff[i].children;
if(_101&&_101.length){
_fe(_101);
}
}
};
},transfer:function(_102,_103,data){
var opts=$.data(_102,"treegrid").options;
var rows=[];
for(var i=0;i<data.length;i++){
rows.push(data[i]);
}
var _104=[];
for(var i=0;i<rows.length;i++){
var row=rows[i];
if(!_103){
if(!row._parentId){
_104.push(row);
rows.splice(i,1);
i--;
}
}else{
if(row._parentId==_103){
_104.push(row);
rows.splice(i,1);
i--;
}
}
}
var toDo=[];
for(var i=0;i<_104.length;i++){
toDo.push(_104[i]);
}
while(toDo.length){
var node=toDo.shift();
for(var i=0;i<rows.length;i++){
var row=rows[i];
if(row._parentId==node[opts.idField]){
if(node.children){
node.children.push(row);
}else{
node.children=[row];
}
toDo.push(row);
rows.splice(i,1);
i--;
}
}
}
return _104;
}});
$.fn.treegrid.defaults=$.extend({},$.fn.datagrid.defaults,{treeField:null,lines:false,animate:false,singleSelect:true,view:_ba,rowEvents:$.extend({},$.fn.datagrid.defaults.rowEvents,{mouseover:_22(true),mouseout:_22(false),click:_24}),loader:function(_105,_106,_107){
var opts=$(this).treegrid("options");
if(!opts.url){
return false;
}
$.ajax({type:opts.method,url:opts.url,data:_105,dataType:"json",success:function(data){
_106(data);
},error:function(){
_107.apply(this,arguments);
}});
},loadFilter:function(data,_108){
return data;
},finder:{getTr:function(_109,id,type,_10a){
type=type||"body";
_10a=_10a||0;
var dc=$.data(_109,"datagrid").dc;
if(_10a==0){
var opts=$.data(_109,"treegrid").options;
var tr1=opts.finder.getTr(_109,id,type,1);
var tr2=opts.finder.getTr(_109,id,type,2);
return tr1.add(tr2);
}else{
if(type=="body"){
var tr=$("#"+$.data(_109,"datagrid").rowIdPrefix+"-"+_10a+"-"+id);
if(!tr.length){
tr=(_10a==1?dc.body1:dc.body2).find("tr[node-id=\""+id+"\"]");
}
return tr;
}else{
if(type=="footer"){
return (_10a==1?dc.footer1:dc.footer2).find("tr[node-id=\""+id+"\"]");
}else{
if(type=="selected"){
return (_10a==1?dc.body1:dc.body2).find("tr.datagrid-row-selected");
}else{
if(type=="highlight"){
return (_10a==1?dc.body1:dc.body2).find("tr.datagrid-row-over");
}else{
if(type=="checked"){
return (_10a==1?dc.body1:dc.body2).find("tr.datagrid-row-checked");
}else{
if(type=="last"){
return (_10a==1?dc.body1:dc.body2).find("tr:last[node-id]");
}else{
if(type=="allbody"){
return (_10a==1?dc.body1:dc.body2).find("tr[node-id]");
}else{
if(type=="allfooter"){
return (_10a==1?dc.footer1:dc.footer2).find("tr[node-id]");
}
}
}
}
}
}
}
}
}
},getRow:function(_10b,p){
var id=(typeof p=="object")?p.attr("node-id"):p;
return $(_10b).treegrid("find",id);
},getRows:function(_10c){
return $(_10c).treegrid("getChildren");
}},onBeforeLoad:function(row,_10d){
},onLoadSuccess:function(row,data){
},onLoadError:function(){
},onBeforeCollapse:function(row){
},onCollapse:function(row){
},onBeforeExpand:function(row){
},onExpand:function(row){
},onClickRow:function(row){
},onDblClickRow:function(row){
},onClickCell:function(_10e,row){
},onDblClickCell:function(_10f,row){
},onContextMenu:function(e,row){
},onBeforeEdit:function(row){
},onAfterEdit:function(row,_110){
},onCancelEdit:function(row){
}});
})(jQuery);

