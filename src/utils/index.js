
import clipboard3 from 'vue-clipboard3';
import { notification } from 'ant-design-vue'
const { toClipboard } = clipboard3();
export const copyText = async (text) => {
    try {
        await toClipboard(text)
        notification.success({
            description: "复制成功!"
        });
    } catch (e) {
        notification.error({
            description: "复制失败!"
        });
    }
}


export const isValidIP=(ip)=> {
    const ipRegex = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
    return ipRegex.test(ip);
}