var language = getcookie('language0');
function getcookie(name) {
    var strCookie = document.cookie;
    var arrCookie = strCookie.split('; ');
    for (var i = 0; i < arrCookie.length; i++) {
        var arr = arrCookie[i].split('=');
        if (arr[0] == name)
            return unescape(arr[1]);
    }
    return '1';
}
function setcookie(name, value, expirehours) {
    var cookieString = name + '=' + escape(value);
    if (expirehours > 0) {
        var date = new Date();
        date.setTime(date.getTime() + expirehours * 3600 * 1000);
        cookieString = cookieString + '; expires=' + date.toGMTString();
    }
    document.cookie = cookieString;
}
if (language == '1') {
    document.write('<script src="english/language.js"><\/script>');
}
else {
    
	document.write('<script src="chinese/language.js"><\/script>');
}
/// <summary>
/// 建立XMLHttpRequest请求(AJAX)
/// </summary>
function createXMLHttpRequest() {
    if (window.XMLHttpRequest) return new XMLHttpRequest();
    try { return new ActiveXObject('MSXML2.XMLHTTP.4.0'); }
    catch (e) {
        try { return new ActiveXObject('MSXML2.XMLHTTP.3.0'); }
        catch (e) {
            try { return new ActiveXObject('MSXML2.XMLHTTP.2.6'); }
            catch (e) {
                try { return new ActiveXObject('MSXML2.XMLHTTP'); }
                catch (e) {
                    try { return new ActiveXObject('Microsoft.XMLHTTP'); }
                    catch (e) { return null; }
                }
            }
        }
    }
}