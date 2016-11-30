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
var _1;
$(document).unbind(".propertygrid").bind("mousedown.propertygrid",function(e){
var p=$(e.target).closest("div.datagrid-view,div.combo-panel");
if(p.length){
return;
}
_2(_1);
_1=undefined;
});
function _3(_4){
var _5=$.data(_4,"propertygrid");
var _6=$.data(_4,"propertygrid").options;
$(_4).datagrid($.extend({},_6,{cls:"propertygrid",view:(_6.showGroup?_6.groupView:_6.view),onBeforeEdit:function(_7,_8){
if(_6.onBeforeEdit.call(_4,_7,_8)==false){
return false;
}
var dg=$(this);
var _8=dg.datagrid("getRows")[_7];
var _9=dg.datagrid("getColumnOption","value");
_9.editor=_8.editor;
},onClickCell:function(_a,_b,_c){
if(_1!=this){
_2(_1);
_1=this;
}
if(_6.editIndex!=_a){
_2(_1);
$(this).datagrid("beginEdit",_a);
var ed=$(this).datagrid("getEditor",{index:_a,field:_b});
if(!ed){
ed=$(this).datagrid("getEditor",{index:_a,field:"value"});
}
if(ed){
var t=$(ed.target);
var _d=t.data("textbox")?t.textbox("textbox"):t;
_d.focus();
_6.editIndex=_a;
}
}
_6.onClickCell.call(_4,_a,_b,_c);
},loadFilter:function(_e){
_2(this);
return _6.loadFilter.call(this,_e);
}}));
};
function _2(_f){
var t=$(_f);
if(!t.length){
return;
}
var _10=$.data(_f,"propertygrid").options;
_10.finder.getTr(_f,null,"editing").each(function(){
var _11=parseInt($(this).attr("datagrid-row-index"));
if(t.datagrid("validateRow",_11)){
t.datagrid("endEdit",_11);
}else{
t.datagrid("cancelEdit",_11);
}
});
_10.editIndex=undefined;
};
$.fn.propertygrid=function(_12,_13){
if(typeof _12=="string"){
var _14=$.fn.propertygrid.methods[_12];
if(_14){
return _14(this,_13);
}else{
return this.datagrid(_12,_13);
}
}
_12=_12||{};
return this.each(function(){
var _15=$.data(this,"propertygrid");
if(_15){
$.extend(_15.options,_12);
}else{
var _16=$.extend({},$.fn.propertygrid.defaults,$.fn.propertygrid.parseOptions(this),_12);
_16.frozenColumns=$.extend(true,[],_16.frozenColumns);
_16.columns=$.extend(true,[],_16.columns);
$.data(this,"propertygrid",{options:_16});
}
_3(this);
});
};
$.fn.propertygrid.methods={options:function(jq){
return $.data(jq[0],"propertygrid").options;
}};
$.fn.propertygrid.parseOptions=function(_17){
return $.extend({},$.fn.datagrid.parseOptions(_17),$.parser.parseOptions(_17,[{showGroup:"boolean"}]));
};
var _18=$.extend({},$.fn.datagrid.defaults.view,{render:function(_19,_1a,_1b){
var _1c=[];
var _1d=this.groups;
for(var i=0;i<_1d.length;i++){
_1c.push(this.renderGroup.call(this,_19,i,_1d[i],_1b));
}
$(_1a).html(_1c.join(""));
},renderGroup:function(_1e,_1f,_20,_21){
var _22=$.data(_1e,"datagrid");
var _23=_22.options;
var _24=$(_1e).datagrid("getColumnFields",_21);
var _25=[];
_25.push("<div class=\"datagrid-group\" group-index="+_1f+">");
if((_21&&(_23.rownumbers||_23.frozenColumns.length))||(!_21&&!(_23.rownumbers||_23.frozenColumns.length))){
_25.push("<span class=\"datagrid-group-expander\">");
_25.push("<span class=\"datagrid-row-expander datagrid-row-collapse\">&nbsp;</span>");
_25.push("</span>");
}
if(!_21){
_25.push("<span class=\"datagrid-group-title\">");
_25.push(_23.groupFormatter.call(_1e,_20.value,_20.rows));
_25.push("</span>");
}
_25.push("</div>");
_25.push("<table class=\"datagrid-btable\" cellspacing=\"0\" cellpadding=\"0\" border=\"0\"><tbody>");
var _26=_20.startIndex;
for(var j=0;j<_20.rows.length;j++){
var css=_23.rowStyler?_23.rowStyler.call(_1e,_26,_20.rows[j]):"";
var _27="";
var _28="";
if(typeof css=="string"){
_28=css;
}else{
if(css){
_27=css["class"]||"";
_28=css["style"]||"";
}
}
var cls="class=\"datagrid-row "+(_26%2&&_23.striped?"datagrid-row-alt ":" ")+_27+"\"";
var _29=_28?"style=\""+_28+"\"":"";
var _2a=_22.rowIdPrefix+"-"+(_21?1:2)+"-"+_26;
_25.push("<tr id=\""+_2a+"\" datagrid-row-index=\""+_26+"\" "+cls+" "+_29+">");
_25.push(this.renderRow.call(this,_1e,_24,_21,_26,_20.rows[j]));
_25.push("</tr>");
_26++;
}
_25.push("</tbody></table>");
return _25.join("");
},bindEvents:function(_2b){
var _2c=$.data(_2b,"datagrid");
var dc=_2c.dc;
var _2d=dc.body1.add(dc.body2);
var _2e=($.data(_2d[0],"events")||$._data(_2d[0],"events")).click[0].handler;
_2d.unbind("click").bind("click",function(e){
var tt=$(e.target);
var _2f=tt.closest("span.datagrid-row-expander");
if(_2f.length){
var _30=_2f.closest("div.datagrid-group").attr("group-index");
if(_2f.hasClass("datagrid-row-collapse")){
$(_2b).datagrid("collapseGroup",_30);
}else{
$(_2b).datagrid("expandGroup",_30);
}
}else{
_2e(e);
}
e.stopPropagation();
});
},onBeforeRender:function(_31,_32){
var _33=$.data(_31,"datagrid");
var _34=_33.options;
_35();
var _36=[];
for(var i=0;i<_32.length;i++){
var row=_32[i];
var _37=_38(row[_34.groupField]);
if(!_37){
_37={value:row[_34.groupField],rows:[row]};
_36.push(_37);
}else{
_37.rows.push(row);
}
}
var _39=0;
var _3a=[];
for(var i=0;i<_36.length;i++){
var _37=_36[i];
_37.startIndex=_39;
_39+=_37.rows.length;
_3a=_3a.concat(_37.rows);
}
_33.data.rows=_3a;
this.groups=_36;
var _3b=this;
setTimeout(function(){
_3b.bindEvents(_31);
},0);
function _38(_3c){
for(var i=0;i<_36.length;i++){
var _3d=_36[i];
if(_3d.value==_3c){
return _3d;
}
}
return null;
};
function _35(){
if(!$("#datagrid-group-style").length){
$("head").append("<style id=\"datagrid-group-style\">"+".datagrid-group{height:"+_34.groupHeight+"px;overflow:hidden;font-weight:bold;border-bottom:1px solid #ccc;}"+".datagrid-group-title,.datagrid-group-expander{display:inline-block;vertical-align:bottom;height:100%;line-height:"+_34.groupHeight+"px;padding:0 4px;}"+".datagrid-group-expander{width:"+_34.expanderWidth+"px;text-align:center;padding:0}"+".datagrid-row-expander{margin:"+Math.floor((_34.groupHeight-16)/2)+"px 0;display:inline-block;width:16px;height:16px;cursor:pointer}"+"</style>");
}
};
}});
$.extend($.fn.datagrid.methods,{groups:function(jq){
return jq.datagrid("options").view.groups;
},expandGroup:function(jq,_3e){
return jq.each(function(){
var _3f=$.data(this,"datagrid").dc.view;
var _40=_3f.find(_3e!=undefined?"div.datagrid-group[group-index=\""+_3e+"\"]":"div.datagrid-group");
var _41=_40.find("span.datagrid-row-expander");
if(_41.hasClass("datagrid-row-expand")){
_41.removeClass("datagrid-row-expand").addClass("datagrid-row-collapse");
_40.next("table").show();
}
$(this).datagrid("fixRowHeight");
});
},collapseGroup:function(jq,_42){
return jq.each(function(){
var _43=$.data(this,"datagrid").dc.view;
var _44=_43.find(_42!=undefined?"div.datagrid-group[group-index=\""+_42+"\"]":"div.datagrid-group");
var _45=_44.find("span.datagrid-row-expander");
if(_45.hasClass("datagrid-row-collapse")){
_45.removeClass("datagrid-row-collapse").addClass("datagrid-row-expand");
_44.next("table").hide();
}
$(this).datagrid("fixRowHeight");
});
}});
$.extend(_18,{refreshGroupTitle:function(_46,_47){
var _48=$.data(_46,"datagrid");
var _49=_48.options;
var dc=_48.dc;
var _4a=this.groups[_47];
var _4b=dc.body2.children("div.datagrid-group[group-index="+_47+"]").find("span.datagrid-group-title");
_4b.html(_49.groupFormatter.call(_46,_4a.value,_4a.rows));
},insertRow:function(_4c,_4d,row){
var _4e=$.data(_4c,"datagrid");
var _4f=_4e.options;
var dc=_4e.dc;
var _50=null;
var _51;
if(!_4e.data.rows.length){
$(_4c).datagrid("loadData",[row]);
return;
}
for(var i=0;i<this.groups.length;i++){
if(this.groups[i].value==row[_4f.groupField]){
_50=this.groups[i];
_51=i;
break;
}
}
if(_50){
if(_4d==undefined||_4d==null){
_4d=_4e.data.rows.length;
}
if(_4d<_50.startIndex){
_4d=_50.startIndex;
}else{
if(_4d>_50.startIndex+_50.rows.length){
_4d=_50.startIndex+_50.rows.length;
}
}
$.fn.datagrid.defaults.view.insertRow.call(this,_4c,_4d,row);
if(_4d>=_50.startIndex+_50.rows.length){
_52(_4d,true);
_52(_4d,false);
}
_50.rows.splice(_4d-_50.startIndex,0,row);
}else{
_50={value:row[_4f.groupField],rows:[row],startIndex:_4e.data.rows.length};
_51=this.groups.length;
dc.body1.append(this.renderGroup.call(this,_4c,_51,_50,true));
dc.body2.append(this.renderGroup.call(this,_4c,_51,_50,false));
this.groups.push(_50);
_4e.data.rows.push(row);
}
this.refreshGroupTitle(_4c,_51);
function _52(_53,_54){
var _55=_54?1:2;
var _56=_4f.finder.getTr(_4c,_53-1,"body",_55);
var tr=_4f.finder.getTr(_4c,_53,"body",_55);
tr.insertAfter(_56);
};
},updateRow:function(_57,_58,row){
var _59=$.data(_57,"datagrid").options;
$.fn.datagrid.defaults.view.updateRow.call(this,_57,_58,row);
var tb=_59.finder.getTr(_57,_58,"body",2).closest("table.datagrid-btable");
var _5a=parseInt(tb.prev().attr("group-index"));
this.refreshGroupTitle(_57,_5a);
},deleteRow:function(_5b,_5c){
var _5d=$.data(_5b,"datagrid");
var _5e=_5d.options;
var dc=_5d.dc;
var _5f=dc.body1.add(dc.body2);
var tb=_5e.finder.getTr(_5b,_5c,"body",2).closest("table.datagrid-btable");
var _60=parseInt(tb.prev().attr("group-index"));
$.fn.datagrid.defaults.view.deleteRow.call(this,_5b,_5c);
var _61=this.groups[_60];
if(_61.rows.length>1){
_61.rows.splice(_5c-_61.startIndex,1);
this.refreshGroupTitle(_5b,_60);
}else{
_5f.children("div.datagrid-group[group-index="+_60+"]").remove();
for(var i=_60+1;i<this.groups.length;i++){
_5f.children("div.datagrid-group[group-index="+i+"]").attr("group-index",i-1);
}
this.groups.splice(_60,1);
}
var _5c=0;
for(var i=0;i<this.groups.length;i++){
var _61=this.groups[i];
_61.startIndex=_5c;
_5c+=_61.rows.length;
}
}});
$.fn.propertygrid.defaults=$.extend({},$.fn.datagrid.defaults,{groupHeight:21,expanderWidth:16,singleSelect:true,remoteSort:false,fitColumns:true,loadMsg:"",frozenColumns:[[{field:"f",width:16,resizable:false}]],columns:[[{field:"name",title:"Name",width:100,sortable:true},{field:"value",title:"Value",width:100,resizable:false}]],showGroup:false,groupView:_18,groupField:"group",groupFormatter:function(_62,_63){
return _62;
}});
})(jQuery);

