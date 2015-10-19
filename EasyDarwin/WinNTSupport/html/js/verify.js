function isPort(str)  
{  
    var parten=/^(\d)+$/g;  
    if(parten.test(str)&&parseInt(str)<=65535&&parseInt(str)>=0){  
        return true;  
     }else{  
		alert("端口号格式不正确");
        return false;  
     }   
}  