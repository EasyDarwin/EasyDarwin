function isPort(str,warnstr)  
{  
    var parten=/^(\d)+$/g;  
    if(parten.test(str)&&parseInt(str)<=65535&&parseInt(str)>=0){  
        return true;  
     }else{  
		alert(warnstr);
        return false;  
     }   
}  
function istime(str,warnstr)  
{  
    var parten=/^(\d)+$/g;  
    if(parten.test(str)&&parseInt(str)>=0){  
        return true;  
     }else{  
		alert(warnstr);
        return false;  
     }   
}  