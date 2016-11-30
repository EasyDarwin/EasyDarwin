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
var _3=$.data(_2,"datalist").options;
$(_2).datagrid($.extend({},_3,{cls:"datalist"+(_3.lines?" datalist-lines":""),frozenColumns:(_3.frozenColumns&&_3.frozenColumns.length)?_3.frozenColumns:(_3.checkbox?[[{field:"_ck",checkbox:true}]]:undefined),columns:(_3.columns&&_3.columns.length)?_3.columns:[[{field:_3.textField,width:"100%",formatter:function(_4,_5,_6){
return _3.textFormatter?_3.textFormatter(_4,_5,_6):_4;
}}]]}));
};
var _7=$.extend({},$.fn.datagrid.defaults.view,{render:function(_8,_9,_a){
var _b=$.data(_8,"datagrid");
var _c=_b.options;
if(_c.groupField){
var g=this.groupRows(_8,_b.data.rows);
this.groups=g.groups;
_b.data.rows=g.rows;
var _d=[];
for(var i=0;i<g.groups.length;i++){
_d.push(this.renderGroup.call(this,_8,i,g.groups[i],_a));
}
$(_9).html(_d.join(""));
}else{
$(_9).html(this.renderTable(_8,0,_b.data.rows,_a));
}
},renderGroup:function(_e,_f,_10,_11){
var _12=$.data(_e,"datagrid");
var _13=_12.options;
var _14=$(_e).datagrid("getColumnFields",_11);
var _15=[];
_15.push("<div class=\"datagrid-group\" group-index="+_f+">");
if(!_11){
_15.push("<span class=\"datagrid-group-title\">");
_15.push(_13.groupFormatter.call(_e,_10.value,_10.rows));
_15.push("</span>");
}
_15.push("</div>");
_15.push(this.renderTable(_e,_10.startIndex,_10.rows,_11));
return _15.join("");
},groupRows:function(_16,_17){
var _18=$.data(_16,"datagrid");
var _19=_18.options;
var _1a=[];
for(var i=0;i<_17.length;i++){
var row=_17[i];
var _1b=_1c(row[_19.groupField]);
if(!_1b){
_1b={value:row[_19.groupField],rows:[row]};
_1a.push(_1b);
}else{
_1b.rows.push(row);
}
}
var _1d=0;
var _17=[];
for(var i=0;i<_1a.length;i++){
var _1b=_1a[i];
_1b.startIndex=_1d;
_1d+=_1b.rows.length;
_17=_17.concat(_1b.rows);
}
return {groups:_1a,rows:_17};
function _1c(_1e){
for(var i=0;i<_1a.length;i++){
var _1f=_1a[i];
if(_1f.value==_1e){
return _1f;
}
}
return null;
};
}});
$.fn.datalist=function(_20,_21){
if(typeof _20=="string"){
var _22=$.fn.datalist.methods[_20];
if(_22){
return _22(this,_21);
}else{
return this.datagrid(_20,_21);
}
}
_20=_20||{};
return this.each(function(){
var _23=$.data(this,"datalist");
if(_23){
$.extend(_23.options,_20);
}else{
var _24=$.extend({},$.fn.datalist.defaults,$.fn.datalist.parseOptions(this),_20);
_24.columns=$.extend(true,[],_24.columns);
_23=$.data(this,"datalist",{options:_24});
}
_1(this);
if(!_23.options.data){
var _25=$.fn.datalist.parseData(this);
if(_25.total){
$(this).datalist("loadData",_25);
}
}
});
};
$.fn.datalist.methods={options:function(jq){
return $.data(jq[0],"datalist").options;
}};
$.fn.datalist.parseOptions=function(_26){
return $.extend({},$.fn.datagrid.parseOptions(_26),$.parser.parseOptions(_26,["valueField","textField","groupField",{checkbox:"boolean",lines:"boolean"}]));
};
$.fn.datalist.parseData=function(_27){
var _28=$.data(_27,"datalist").options;
var _29={total:0,rows:[]};
$(_27).children().each(function(){
var _2a=$.parser.parseOptions(this,["value","group"]);
var row={};
var _2b=$(this).html();
row[_28.valueField]=_2a.value!=undefined?_2a.value:_2b;
row[_28.textField]=_2b;
if(_28.groupField){
row[_28.groupField]=_2a.group;
}
_29.total++;
_29.rows.push(row);
});
return _29;
};
$.fn.datalist.defaults=$.extend({},$.fn.datagrid.defaults,{fitColumns:true,singleSelect:true,showHeader:false,checkbox:false,lines:false,valueField:"value",textField:"text",groupField:"",view:_7,textFormatter:function(_2c,row){
return _2c;
},groupFormatter:function(_2d,_2e){
return _2d;
}});
})(jQuery);

