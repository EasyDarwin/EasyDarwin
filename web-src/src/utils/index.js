
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
    xhr.responseType = 'arraybuffer'; // Return type blob
    xhr.onload = function() {
        if (xhr.readyState === 4 && xhr.status === 200) {
            let blob = this.response;
            // Convert a blob link
            // Note: The URL.createObjectURL() static method creates a DOMString (DOMString is a UTF-16 string).
            // which contains a URL representing the object given in the parameter, and whose lifecycle is bound to the document in the window in which it was created.
            let downLoadUrl = window.URL.createObjectURL(new Blob([blob], {
                type: 'video/mp4'
            }));
            // The type of the video is video/mp4 and the image is image/jpeg
            // 01.Create a tag
            let a = document.createElement('a');
            // 02.Name the attribute download of the a tag.
            a.download = name;
            // 03.Setting the name of the downloaded file
            a.href = downLoadUrl;
            // 04.Do a hide on the a tag
            a.style.display = 'none';
            // 05.Adding a tag to a document
            document.body.appendChild(a);
            // 06.Launching a click event
            a.click();
            // 07.Delete this tab after downloading
            a.remove();
        };
    };
    xhr.send()
}


export const isValidIP=(ip)=> {
    const ipRegex = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
    return ipRegex.test(ip);
}
