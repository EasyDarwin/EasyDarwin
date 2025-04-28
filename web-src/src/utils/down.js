/**
 * 下载资源到本地（纯 JS 版）
 *
 * @param {string} url       资源地址
 * @param {string} [fileName] 下载时的默认文件名，可选
 * @param {boolean} [useFetch=false] 是否先通过 fetch 获取 Blob 再下载
 */
export async function saveFile(url, fileName, useFetch = false) {
  // 从 URL 中提取文件名
  function inferNameFromUrl(u) {
    try {
      var pathname = new URL(u, window.location.href).pathname;
      return pathname.substring(pathname.lastIndexOf('/') + 1) || 'download';
    } catch (e) {
      return 'download';
    }
  }

  var name = fileName || inferNameFromUrl(url);
  var downloadUrl = url;
  var objectUrl = null;

  // 如果需要先 fetch 拿到 Blob
  if (useFetch) {
    var resp = await fetch(url);
    if (!resp.ok) {
      throw new Error('Fetch 资源失败：' + resp.status + ' ' + resp.statusText);
    }
    var blob = await resp.blob();
    objectUrl = URL.createObjectURL(blob);
    downloadUrl = objectUrl;
  }

  // 创建隐藏的 <a> 元素并触发下载
  var a = document.createElement('a');
  a.style.display = 'none';
  a.href = downloadUrl;
  a.download = name;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);

  // 释放临时的 ObjectURL
  if (objectUrl) {
    URL.revokeObjectURL(objectUrl);
  }
}
