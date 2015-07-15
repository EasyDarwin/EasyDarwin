(function($){
	$.fn.accordion = function(options) {
	
		var defaults = 
		{
			
		};
	
		var options = $.extend(defaults, options);
	
		return this.each( function() 
		{		
			var $container = $(this);
			var $triggers = $container.find ('.accordion_panel');
			var $panels = $container.find ('.accordion_content');
			
			
			$panels.hide ();			
			$triggers.eq (0).addClass ('active').next ().show ();			
			
			// Set min-height to prevent content from jumping as much
			$container.css ('min-height' , $container.height ()  + 10 + 'px');
			
			$triggers.live ('click' , function () 
			{
				if ( $(this).next ().is (':hidden') )
				{
					$triggers.removeClass ('active').next ().slideUp ();
					$(this).toggleClass ('active').next ().slideDown ();
				}					
				return false;
			});
		});		
	};

})(jQuery);


(function($){
	$.fn.tabs = function(options) {
	
		var defaults = 
		{
			
		};
	
		var options = $.extend(defaults, options);
	
		return this.each( function() {
	   		
			var $tabContainer = $(this);
			var $tabLi = $tabContainer.find ('.tabs li');
			var $tabContent = $tabContainer.find ('.tab_content');
			
			$tabContent.hide ();
			$tabLi.eq (0).addClass ('active').show ();
			$tabContent.eq (0).show ();
			
			$tabLi.live ('click' , function () 
			{
				var activeTab = $(this).find ('a').attr ('href');
				
				$tabLi.removeClass("active");
				$(this).addClass("active");
				$tabContent.hide ();
				
				$tabContainer.find (activeTab).fadeIn ('slow');
				return false;
			});	
		});		
	};

})(jQuery);
