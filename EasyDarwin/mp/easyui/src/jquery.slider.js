/**
 * jQuery EasyUI 1.4.4
 * 
 * Copyright (c) 2009-2015 www.jeasyui.com. All rights reserved.
 *
 * Licensed under the freeware license: http://www.jeasyui.com/license_freeware.php
 * To use it on other terms please contact us: info@jeasyui.com
 *
 */
/**
 * slider - jQuery EasyUI
 * 
 * Dependencies:
 *  draggable
 * 
 */
(function($){
	function init(target){
		var slider = $('<div class="slider">' +
				'<div class="slider-inner">' +
				'<a href="javascript:void(0)" class="slider-handle"></a>' +
				'<span class="slider-tip"></span>' +
				'</div>' +
				'<div class="slider-rule"></div>' +
				'<div class="slider-rulelabel"></div>' +
				'<div style="clear:both"></div>' +
				'<input type="hidden" class="slider-value">' +
				'</div>').insertAfter(target);
		var t = $(target);
		t.addClass('slider-f').hide();
		var name = t.attr('name');
		if (name){
			slider.find('input.slider-value').attr('name', name);
			t.removeAttr('name').attr('sliderName', name);
		}
		slider.bind('_resize', function(e,force){
			if ($(this).hasClass('easyui-fluid') || force){
				setSize(target);
			}
			return false;
		});
		return slider;
	}
	
	/**
	 * set the slider size, for vertical slider, the height property is required
	 */
	function setSize(target, param){
		var state = $.data(target, 'slider');
		var opts = state.options;
		var slider = state.slider;
		
		if (param){
			if (param.width) opts.width = param.width;
			if (param.height) opts.height = param.height;
		}
		slider._size(opts);
		if (opts.mode == 'h'){
			slider.css('height', '');
			slider.children('div').css('height', '');
		} else {
			slider.css('width', '');
			slider.children('div').css('width', '');
			slider.children('div.slider-rule,div.slider-rulelabel,div.slider-inner')._outerHeight(slider._outerHeight());
		}
		initValue(target);
	}
	
	/**
	 * show slider rule if needed
	 */
	function showRule(target){
		var state = $.data(target, 'slider');
		var opts = state.options;
		var slider = state.slider;
		
		var aa = opts.mode == 'h' ? opts.rule : opts.rule.slice(0).reverse();
		if (opts.reversed){
			aa = aa.slice(0).reverse();
		}
		_build(aa);
		
		function _build(aa){
			var rule = slider.find('div.slider-rule');
			var label = slider.find('div.slider-rulelabel');
			rule.empty();
			label.empty();
			for(var i=0; i<aa.length; i++){
				var distance = i*100/(aa.length-1)+'%';
				var span = $('<span></span>').appendTo(rule);
				span.css((opts.mode=='h'?'left':'top'), distance);
				
				// show the labels
				if (aa[i] != '|'){
					span = $('<span></span>').appendTo(label);
					span.html(aa[i]);
					if (opts.mode == 'h'){
						span.css({
							left: distance,
							marginLeft: -Math.round(span.outerWidth()/2)
						});
					} else {
						span.css({
							top: distance,
							marginTop: -Math.round(span.outerHeight()/2)
						});
					}
				}
			}
		}
	}
	
	/**
	 * build the slider and set some properties
	 */
	function buildSlider(target){
		var state = $.data(target, 'slider');
		var opts = state.options;
		var slider = state.slider;
		
		slider.removeClass('slider-h slider-v slider-disabled');
		slider.addClass(opts.mode == 'h' ? 'slider-h' : 'slider-v');
		slider.addClass(opts.disabled ? 'slider-disabled' : '');
		
		var inner = slider.find('.slider-inner');
		inner.html(
			'<a href="javascript:void(0)" class="slider-handle"></a>' +
			'<span class="slider-tip"></span>'
		);
		if (opts.range){
			inner.append(
				'<a href="javascript:void(0)" class="slider-handle"></a>' +
				'<span class="slider-tip"></span>'
			);
		}
		
		slider.find('a.slider-handle').draggable({
			axis:opts.mode,
			cursor:'pointer',
			disabled: opts.disabled,
			onDrag:function(e){
				var left = e.data.left;
				var width = slider.width();
				if (opts.mode!='h'){
					left = e.data.top;
					width = slider.height();
				}
				if (left < 0 || left > width) {
					return false;
				} else {
					setPos(left, this);
					return false;
				}
			},
			onStartDrag:function(){
				state.isDragging = true;
				opts.onSlideStart.call(target, opts.value);
			},
			onStopDrag:function(e){
				setPos(opts.mode=='h'?e.data.left:e.data.top, this);
				opts.onSlideEnd.call(target, opts.value);
				opts.onComplete.call(target, opts.value);
				state.isDragging = false;
			}
		});
		slider.find('div.slider-inner').unbind('.slider').bind('mousedown.slider', function(e){
			if (state.isDragging || opts.disabled){return}
			var pos = $(this).offset();
			setPos(opts.mode=='h'?(e.pageX-pos.left):(e.pageY-pos.top));
			opts.onComplete.call(target, opts.value);
		});
		
		function setPos(pos, handle){
			var value = pos2value(target, pos);
			var s = Math.abs(value % opts.step);
			if (s < opts.step/2){
				value -= s;
			} else {
				value = value - s + opts.step;
			}
			if (opts.range){
				var v1 = opts.value[0];
				var v2 = opts.value[1];
				var m = parseFloat((v1+v2)/2);
				if (handle){
					var isLeft = $(handle).nextAll('.slider-handle').length > 0;
					if (value <= v2 && isLeft){
						v1 = value;
					} else if (value >= v1 && (!isLeft)){
						v2 = value;
					}
				} else {
					if (value < v1){
						v1 = value;
					} else if (value > v2){
						v2 = value;
					} else {
						value < m ? v1 = value : v2 = value;
					}					
				}
				$(target).slider('setValues', [v1,v2]);
			} else {
				$(target).slider('setValue', value);
			}
		}
	}
	
	/**
	 * set a specified value to slider
	 */
	function setValues(target, values){
		var state = $.data(target, 'slider');
		var opts = state.options;
		var slider = state.slider;
		var oldValues = $.isArray(opts.value) ? opts.value : [opts.value];
		var newValues = [];
		
		if (!$.isArray(values)){
			values = $.map(String(values).split(opts.separator), function(v){
				return parseFloat(v);
			});
		}
		
		slider.find('.slider-value').remove();
		var name = $(target).attr('sliderName') || '';
		for(var i=0; i<values.length; i++){
			var value = values[i];
			if (value < opts.min) value = opts.min;
			if (value > opts.max) value = opts.max;
			
			var input = $('<input type="hidden" class="slider-value">').appendTo(slider);
			input.attr('name', name);
			input.val(value);
			newValues.push(value);
			
			var handle = slider.find('.slider-handle:eq('+i+')');
			var tip = handle.next();
			var pos = value2pos(target, value);
			if (opts.showTip){
				tip.show();
				tip.html(opts.tipFormatter.call(target, value));
			} else {
				tip.hide();
			}
			
			if (opts.mode == 'h'){
				var style = 'left:'+pos+'px;';
				handle.attr('style', style);
				tip.attr('style', style +  'margin-left:' + (-Math.round(tip.outerWidth()/2)) + 'px');
			} else {
				var style = 'top:' + pos + 'px;';
				handle.attr('style', style);
				tip.attr('style', style + 'margin-left:' + (-Math.round(tip.outerWidth())) + 'px');
			}
		}
		opts.value = opts.range ? newValues : newValues[0];
		$(target).val(opts.range ? newValues.join(opts.separator) : newValues[0]);
		
		if (oldValues.join(',') != newValues.join(',')){
			opts.onChange.call(target, opts.value, (opts.range?oldValues:oldValues[0]));
		}
	}
	
	function initValue(target){
		var opts = $.data(target, 'slider').options;
		var fn = opts.onChange;
		opts.onChange = function(){};
		setValues(target, opts.value);
		opts.onChange = fn;
	}
	
	/**
	 * translate value to slider position
	 */
	function value2pos(target, value){
		var state = $.data(target, 'slider');
		var opts = state.options;
		var slider = state.slider;
		var size = opts.mode == 'h' ? slider.width() : slider.height();
		var pos = opts.converter.toPosition.call(target, value, size);
		if (opts.mode == 'v'){
			pos = slider.height() - pos;
		}
		if (opts.reversed){
			pos = size - pos;
		}
		return pos.toFixed(0);
	}
	
	/**
	 * translate slider position to value
	 */
	function pos2value(target, pos){
		var state = $.data(target, 'slider');
		var opts = state.options;
		var slider = state.slider;
		var size = opts.mode == 'h' ? slider.width() : slider.height();
		var pos = opts.mode=='h' ? (opts.reversed?(size-pos):pos) : (opts.reversed?pos:(size-pos));
		var value = opts.converter.toValue.call(target, pos, size);
		return value.toFixed(0);
	}
	
	$.fn.slider = function(options, param){
		if (typeof options == 'string'){
			return $.fn.slider.methods[options](this, param);
		}
		
		options = options || {};
		return this.each(function(){
			var state = $.data(this, 'slider');
			if (state){
				$.extend(state.options, options);
			} else {
				state = $.data(this, 'slider', {
					options: $.extend({}, $.fn.slider.defaults, $.fn.slider.parseOptions(this), options),
					slider: init(this)
				});
				$(this).removeAttr('disabled');
			}
			
			var opts = state.options;
			opts.min = parseFloat(opts.min);
			opts.max = parseFloat(opts.max);
			if (opts.range){
				if (!$.isArray(opts.value)){
					opts.value = $.map(String(opts.value).split(opts.separator), function(v){
						return parseFloat(v);
					});
				}
				if (opts.value.length < 2){
					opts.value.push(opts.max);
				}
			} else {
				opts.value = parseFloat(opts.value);
			}
			opts.step = parseFloat(opts.step);
			opts.originalValue = opts.value;
			
			buildSlider(this);
			showRule(this);
			setSize(this);
		});
	};
	
	$.fn.slider.methods = {
		options: function(jq){
			return $.data(jq[0], 'slider').options;
		},
		destroy: function(jq){
			return jq.each(function(){
				$.data(this, 'slider').slider.remove();
				$(this).remove();
			});
		},
		resize: function(jq, param){
			return jq.each(function(){
				setSize(this, param);
			});
		},
		getValue: function(jq){
			return jq.slider('options').value;
		},
		getValues: function(jq){
			return jq.slider('options').value;
		},
		setValue: function(jq, value){
			return jq.each(function(){
				setValues(this, [value]);
			});
		},
		setValues: function(jq, values){
			return jq.each(function(){
				setValues(this, values);
			});
		},
		clear: function(jq){
			return jq.each(function(){
				var opts = $(this).slider('options');
				setValues(this, opts.range?[opts.min,opts.max]:[opts.min]);
			});
		},
		reset: function(jq){
			return jq.each(function(){
				var opts = $(this).slider('options');
				$(this).slider(opts.range?'setValues':'setValue', opts.originalValue);
			});
		},
		enable: function(jq){
			return jq.each(function(){
				$.data(this, 'slider').options.disabled = false;
				buildSlider(this);
			});
		},
		disable: function(jq){
			return jq.each(function(){
				$.data(this, 'slider').options.disabled = true;
				buildSlider(this);
			});
		}
	};
	
	$.fn.slider.parseOptions = function(target){
		var t = $(target);
		return $.extend({}, $.parser.parseOptions(target, [
			'width','height','mode',{reversed:'boolean',showTip:'boolean',range:'boolean',min:'number',max:'number',step:'number'}
		]), {
			value: (t.val() || undefined),
			disabled: (t.attr('disabled') ? true : undefined),
			rule: (t.attr('rule') ? eval(t.attr('rule')) : undefined)
		});
	};
	
	$.fn.slider.defaults = {
		width: 'auto',
		height: 'auto',
		mode: 'h',	// 'h'(horizontal) or 'v'(vertical)
		reversed: false,
		showTip: false,
		disabled: false,
		range: false,
		value: 0,
		separator: ',',
		min: 0,
		max: 100,
		step: 1,
		rule: [],	// [0,'|',100]
		tipFormatter: function(value){return value},
		converter:{
			toPosition:function(value, size){
				var opts = $(this).slider('options');
				return (value-opts.min)/(opts.max-opts.min)*size;
			},
			toValue:function(pos, size){
				var opts = $(this).slider('options');
				return opts.min + (opts.max-opts.min)*(pos/size);
			}
		},
		onChange: function(value, oldValue){},
		onSlideStart: function(value){},
		onSlideEnd: function(value){},
		onComplete: function(value){}
	};
})(jQuery);
