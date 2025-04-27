export const toggleClass = (el, className, flag) => {
  if (flag) {
    el.classList.add(className)
  } else {
    el.classList.remove(className)
  }
}
