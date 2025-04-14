
import clipboard3 from 'vue-clipboard3';
import { notification } from 'ant-design-vue'
const { toClipboard } = clipboard3();
export const copyText = async (text, key ='') => {
    try {
        await toClipboard(text)
        notification.success({
            description: key + " 复制成功!" 
        });
    } catch (e) {
        notification.error({
            description: key + " 复制失败!"
        });
    }
}

export function downLoad(url, name) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'arraybuffer'; // 返回类型blob
    xhr.onload = function() {
        if (xhr.readyState === 4 && xhr.status === 200) {
            let blob = this.response;
            // 转换一个blob链接
            // 注: URL.createObjectURL() 静态方法会创建一个 DOMString(DOMString 是一个UTF-16字符串)，
            // 其中包含一个表示参数中给出的对象的URL。这个URL的生命周期和创建它的窗口中的document绑定
            let downLoadUrl = window.URL.createObjectURL(new Blob([blob], {
                type: 'video/mp4'
            }));
            // 视频的type是video/mp4，图片是image/jpeg
            // 01.创建a标签
            let a = document.createElement('a');
            // 02.给a标签的属性download设定名称
            a.download = name;
            // 03.设置下载的文件名
            a.href = downLoadUrl;
            // 04.对a标签做一个隐藏处理
            a.style.display = 'none';
            // 05.向文档中添加a标签
            document.body.appendChild(a);
            // 06.启动点击事件
            a.click();
            // 07.下载完毕删除此标签
            a.remove();
        };
    };
    xhr.send()
}


export const isValidIP=(ip)=> {
    const ipRegex = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
    return ipRegex.test(ip);
}