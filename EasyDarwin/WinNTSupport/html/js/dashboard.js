var Dashboard = function ()
{
	var chartColors = ['#F90','#333', '#555', '#111','#002646','#999','#bbb','#ccc','#eee'];

	return { init: init };
	
	function init ()
	{		
		$('*[rel=facebox]').facebox ();
		$('.datatable').dataTable ();
		$('.uniform').find ('input, select').uniform ();
		$('input, textarea').placeholder ();
		
		$('table.stats').each(function() 
		{		
			var chartType = '';
			
			if ( $(this).attr('data-chart') ) 
			{ 
				chartType = $(this).attr('data-chart'); 				
			}
			else 
			{ 
				chartType = 'area'; 
			}
			
			var chart_width = $(this).parent ().width () * .92;
					
			$(this).hide ().visualize({		
				type: chartType,	// 'bar', 'area', 'pie', 'line'
				width: chart_width,
				height: '240px',
				colors: chartColors
			});				
		});
	}	
}();