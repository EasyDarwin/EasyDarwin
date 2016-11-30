//格式化时间
Date.prototype.format = function (f) {
    var o = {
        "M+": this.getMonth() + 1, // month
        "d+": this.getDate(), // day
        "H+": this.getHours(), // hour
        "h+": this.getHours() % 12, // 12hour
        "m+": this.getMinutes(), // minute
        "s+": this.getSeconds(), // second
        "q+": Math.floor((this.getMonth() + 3) / 3), // quarter
        "S": this.getMilliseconds()
        // millisecond
    }
    if (/(y+)/.test(f)) {
        f = f.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
    }
    for (var k in o) {
        if (new RegExp("(" + k + ")").test(f)) {
            f = f.replace(RegExp.$1, RegExp.$1.length == 1 ? o[k] : ("00" + o[k]).substr(("" + o[k]).length));
        }
    }
    return f;
}

// String 格式化
String.prototype.format = function (args) {
    var result = this;
    if (arguments.length > 0) {
        if (arguments.length == 1 && typeof (args) == "object") {
            for (var key in args) {
                if (args[key] != undefined) {
                    var reg = new RegExp("({" + key + "})", "g");
                    result = result.replace(reg, args[key]);
                }
            }
        } else {
            for (var i = 0; i < arguments.length; i++) {
                if (arguments[i] != undefined) {
                    var reg = new RegExp("({)" + i + "(})", "g");
                    result = result.replace(reg, arguments[i]);
                }
            }
        }
    }
    return result;
}

/** time formatter */
function formatDateTime(value, row, index) {
    if (!value) {
        return "";
    }
    return new Date(value).format('yyyy-MM-dd HH:mm:ss');
}
function formatDate(value, row, index) {
    if (!value) {
        return "";
    }
    return new Date(value).format('yyyy-MM-dd');
}
/** link formatter */
function formatLink(value, row, index) {
    return "<a href='{0}'>{1}</a>".format(value, value);
}
/** size formatter */
function formatSize(value, row, index) {
    function roundFun(numberRound, roundDigit) {
        if (numberRound >= 0) {
            var tempNumber = parseInt((numberRound * Math.pow(10, roundDigit) + 0.5)) / Math.pow(10, roundDigit);
            return tempNumber;
        } else {
            numberRound1 = -numberRound
            var tempNumber = parseInt((numberRound1 * Math.pow(10, roundDigit) + 0.5)) / Math.pow(10, roundDigit);
            return -tempNumber;
        }
    }

    if (!value || value < 0) {
        return "0 Bytes";
    }
    var unitArr = new Array("Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB");
    var index = 0;
    var srcsize = parseFloat(value);
    var size = roundFun(srcsize / Math.pow(1024, (index = Math.floor(Math.log(srcsize) / Math.log(1024)))), 2);
    return size + unitArr[index];
}

function isPC() {
    var ua = navigator.userAgent.toLowerCase();
    var agents = ["android", "iphone",
        "symbianos", "windows phone",
        "ipad", "ipod"];
    var flag = true;
    for (var v in agents) {
        if (ua.indexOf(agents[v]) > 0) {
            flag = false;
            break;
        }
    }
    return flag;
}

var _url = "http://121.40.50.44:10008/api/v1";
try {
    $.ajax({
        type: "GET",
        url: _url + "/getserverinfo",
        global: false,
        async: false,
        success: function (data) {
            var ret = JSON.parse(data);
            _url = "http://121.40.50.44:10008/api/" + ret.EasyDarwin.Body.InterfaceVersion;
        }
    });
} catch (e) { }

$(function () {
    $.extend($.gritter.options, {
        class_name: 'gritter-error',
        position: 'bottom-right',
        fade_in_speed: 100,
        fade_out_speed: 100,
        time: 3000
    });

    $(".content-wrapper").on("transitionend", function () {
        $("table.easyui-datagrid").each(function (i) {
            $(this).datagrid("resize");
        });
    });
    $(".main-header").on("transitionend", function () {
        $.AdminLTE.layout.fix();
        $(".main-sidebar").css('padding-top', $(".main-header").outerHeight());
    });
    $.AdminLTE.layout.fix();
    $(document).on('click', '.sidebar-toggle', function () {
        $(".main-sidebar").css('padding-top', $(".main-header").outerHeight());
    })

    if (typeof errorMsg != 'undefined' && errorMsg) {
        $.gritter.add({
            text: errorMsg
        });
    }
    $(document).ajaxSuccess(function () {
        $("body").removeClass("hide");
    })
    $(document).ajaxError(function (evt, xhr, opts, ex) {
        if (xhr.status == 401) {
            $.cookie("token", "", { expires: -1 });
            $.cookie("username", "", { expires: -1 });
            top.location.href = '/login.html';
            return false;
        }
        if (xhr.status == 404) {
            xhr.responseText = "请求服务不存在或已停止";
        }
        var msg = xhr.responseText;
        try {
            msg = JSON.parse(msg);
        } catch (e) {
        }
        if (typeof msg != 'undefined' && msg) {
            $.gritter.add({
                text: msg
            });
        }
    });

    $(document).on("shown.bs.modal", ".modal", function () {
        //$(this).find("input:visible:first").focus();
    }).on("hidden.bs.modal", ".modal", function (e) {
        $(this).find("form").each(function () {
            $(this)[0].reset();
        });
        $(this).find("input:hidden").val('');
    }).on("show.bs.modal", ".modal", function (e) {
        $(this).find(".form-group").removeClass("has-error").removeClass("has-success");
        $(this).find(".with-errors").empty();
    });

    $(document).ajaxStart(function () {
        $(".modal:visible .btn-primary").prop("disabled", true).attr("data-ajaxing", "true");
    }).ajaxComplete(function () {
        $(".btn-primary:disabled[data-ajaxing=true]").prop("disabled", false).removeAttr("data-ajaxing");
    });


    easyloader.theme = 'bootstrap';
    easyloader.locale = 'zh_CN';
    easyloader.onProgress = function (name) {
        if (name == 'parser') {
            $.parser.onComplete = function () {
                $("section.content").removeClass("easyui-wrapper");
                easyloader.load(['form']);
            }
        }
    }

    var $menu = $("ul.sidebar-menu");
    //渲染菜单状态
    $menu.find("li").removeClass('active');
    $link = $menu.find("li a[href='{0}']".format(location.pathname)).first();
    if ($link.size() == 1) {
        $link.parents("ul.sidebar-menu li").addClass("active");
    }

    $("form[data-toggle=validator]").attr("data-disable", "false").attr("autocomplete", "off");
    $('form[data-toggle=validator]').validator().on('submit', function (e) {
        var $form = $(this);
        if (e.isDefaultPrevented()) {
            $form.find(".form-group.has-error:first").find("input:visible").focus();
        }
    });
    $(document).on("keydown", "form[data-toggle=validator]", function (event) {
        if (event.keyCode == 13) { return false; }
    })
    $("input[data-toggle=integer]").inputNumber();
    $(".modal.fade").attr("data-backdrop", "static");
});