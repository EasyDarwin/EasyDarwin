/**
 * 格式化文件大小
 * @param {number} size 文件大小（单位：字节）
 * @returns {string} 格式化后的大小
 */
export function formatFileSize(size) {
  if (size < 1024) {
    return size + ' B'
  } else if (size < 1024 * 1024) {
    return (size / 1024).toFixed(2) + ' KB'
  } else if (size < 1024 * 1024 * 1024) {
    return (size / (1024 * 1024)).toFixed(2) + ' MB'
  } else {
    return (size / (1024 * 1024 * 1024)).toFixed(2) + ' GB'
  }
}

/**
 * 格式化时间显示
 * @param {number} size 时间（单位：秒）
 * @returns {string} 格式化后的时间
 */
export function formatDuration(seconds) {
  if (seconds < 60) {
    return `${seconds} 秒`
  } else if (seconds < 3600) {
    const minutes = Math.floor(seconds / 60)
    const remainingSeconds = seconds % 60
    return remainingSeconds === 0
      ? `${minutes} 分钟`
      : `${minutes} 分钟 ${remainingSeconds} 秒`
  } else {
    const hours = Math.floor(seconds / 3600)
    const minutes = Math.floor((seconds % 3600) / 60)
    const remainingSeconds = seconds % 60
    let result = `${hours} 小时`
    if (minutes > 0) result += `${minutes} 分钟`
    if (remainingSeconds > 0) result += `${remainingSeconds} 秒`
    return result
  }
}
