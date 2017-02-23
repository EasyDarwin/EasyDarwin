$(function () {
    var haojuMallInit = {
        backToTop: function () {

            /* 返回顶部按钮 */
            var screenWith = $(window).innerWidth();
            var tureMarR = (screenWith - 1000) / 2 - 62;
            tureMarR = parseInt(tureMarR);
            $('.scroll-top').css('right', tureMarR);
            $(document).on('click', '.scroll-top', function () {
                $('html, body').animate({
                    scrollTop: 0
                }, 450);
            })
        },
        closebtn: function () {

            /* 关闭按钮 */
            var screenWith = $(window).innerWidth();
            var tureMarR = (screenWith - 1000) / 2 - 37;
            tureMarR = parseInt(tureMarR);
            $('.close-btn').css('right', tureMarR);
            $(document).on('click', '.close-btn', function () {
                $(this).parents('.mall-pop').fadeOut();
            });
            $('.close-btn').hover(function () {
                    $(this).removeClass('blur').addClass('hover');
                },
                function () {
                    $(this).addClass('blur').removeClass('hover');
                })
        }
    };


    haojuMallInit.backToTop();

    haojuMallInit.closebtn();

    $(window).scroll(function () {
        var scrollLength = $(document).scrollTop();
        if (scrollLength > 200) {
            $('.scroll-top').fadeIn();
        } else {
            $('.scroll-top').fadeOut();
        }
    });

    /*底部悬浮框模拟placeholder*/
    $('.pop-form input').focus(function () {
        $(this).siblings('.tool-placeholder').hide();
    });
    $('.pop-form input').blur(function () {
        if ($(this).val() == '') {
            $(this).siblings('.tool-placeholder').show();
        }
    });
    $('.tool-placeholder').click(function () {
        $(this).siblings('input').focus();
    });

    /*表单模拟placeholder*/
    $('.input-group').on('focus', 'input , textarea', function () {
        $(this).siblings('label').hide();
    });
    $('.input-group').on('blur', 'input , textarea', function () {
        if ($(this).val() == '') {
            $(this).siblings('label').show();
        }
    });

    /*底部表单验证*/
    $('.pop-form form').submit(function () {
        var phoneNumber = $(this).find('input').val();
        if (!/^1\d{10}$/.test(phoneNumber)) {
            $(this).find('input').val('');
            alert('请正确填写手机号码。');
            return false;
        }
    });

    /*团购表单表单验证*/
    $('.join-group-buy').submit(function () {
        var _name = $('#group-username').val();
        var _tel = $('#group-phone').val();
        if (_name == '' | _tel == '') {
            alert('姓名和手机号码为必填字段');
            return false;
        } else if (!/^1\d{10}$/.test(_tel)) {
            alert('请正确填写手机号码');
            return false;

        }
    });

    /*去除户型列表/3边距*/
    $('.house-type-list li').each(function () {
        if (($(this).index() + 1) % 3 == 0) {
            $(this).css('margin-right', '0');
        }
    });

    /*去除楼盘相册/3边距*/
    $('.album-list li').each(function () {
        if (($(this).index() + 1) % 3 == 0) {
            $(this).css('margin-right', '0');
        }
    });
})
