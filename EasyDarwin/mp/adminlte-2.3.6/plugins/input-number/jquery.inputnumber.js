/** number only text input */
(function($) {
	var methods = {
		"init" : function(cfg) {
			var $this = this;
			var default_settings = {
				type : 'integer'
			};
			var settings = $.extend(default_settings, cfg);
			$this.attr("placeholder", settings.placeholder);
			this.on("keyup", function(e) {
				switch (settings.type) {
				case 'integer':
				case 'int':
					this.value = this.value.replace(/[^0-9]/g, '');
					break;
				case 'float':
				case 'double':
					this.value = this.value.replace(/[^0-9\.]/g, '');
					break;
				default:
					this.value = this.value.replace(/[^0-9]/g, '');
					break;
				}
			});
			return this;
		}

	};

	$.fn.inputNumber = function(method) {
		if (methods[method]) {
			return methods[method].apply(this, Array.prototype.slice.call(arguments, 1));
		} else if (typeof method === 'object' || !method) {
			return methods.init.apply(this, arguments);
		} else {
			$.error('Method ' + method + ' does not exist on jQuery.inputNumber');
		}
	};
})(jQuery);