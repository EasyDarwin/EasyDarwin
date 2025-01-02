
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
