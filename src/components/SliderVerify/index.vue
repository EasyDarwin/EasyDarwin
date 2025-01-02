<template>
  <div id="plugin-slider-verify_containe">
    <div id="slider-verify" :class="{ 'is-border': isBorder }">
      <div id="verify_containe" :class="{ 'is-opt': isCloseBtn || isReloadBtn }">
        <div id="canvas_containe">
          <div class="loading" :style="{ width: `${width}px`, height: `${height}px` }" v-if="loading">
            <div type="circular" vertical>
              <span>加载中...</span>
            </div>
          </div>
          <canvas id="bg_canvas" v-show="!loading"></canvas>
          <canvas v-show="!loading" id="block_canvas" 
            @mousedown.prevent="(e) => drag(e, 'block_canvas', 'circle')"
            @touchstart.prevent="(e) => {
              terminal = 'mobile'
              drag(e, 'block_canvas', 'circle')
            }" 
            @click="e => {
              terminal = setTerminal()
              drag(e, 'block_canvas', 'circle', true)
            }">
          </canvas>
        </div>
        <div class="slide-box">
          <div id="circle" 
            @mousedown.prevent="(e) => drag(e, 'circle', 'block_canvas')" 
            @touchstart.prevent="(e) => {
              terminal = 'mobile'
              drag(e, 'circle', 'block_canvas')
            }" 
            @click="e => {
              // terminal = setTerminal()
              drag(e, 'circle', 'block_canvas', true)
            }">
            <div class="verticals" v-show="!isTouch">
              <img src="./images/vertical_line.png" alt="png" />
              <img src="./images/vertical_line.png" alt="png" />
              <img src="./images/vertical_line.png" alt="png" />
            </div>
            <div class="arrow" v-show="isTouch">
              <img src="./images/arrow_left.png" alt="png" />
              <img src="./images/circle.png" class="circle" alt="png" />
              <img src="./images/arrow_right.png" alt="png" />
            </div>
          </div>
          <span id="placehold">拖动滑块完成拼图</span>
        </div>

        <div class="operational" v-if="isCloseBtn || isReloadBtn">
          <img src="./images/close.png" alt="" @click="close" v-if="isCloseBtn" />
          <img src="./images/reload.png" alt="" @click="reload" v-if="isReloadBtn" />
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import {defineComponent,reactive,watch,computed,onMounted,toRefs} from 'vue'

const l = 42 // 滑块边长
const r = 9 // 滑块圆半径
const L = l + r * 2 + 3 // 滑块实际边长
const Y = 70 // 滑块Y轴距离

export default defineComponent({
  name: 'SliderVerify',
  props: {
    width: {
      type: Number,
      default: 300,
    },
    height: {
      type: Number,
      default: 220,
    },
    w: {
      type: Number,
      default: 60,
    },
    h: {
      type: Number,
      default: 60,
    },
    top: {
      type: Number,
      default: 60,
    },
    isBorder: {
      type: Boolean,
      default: true,
    },
    imgX: {
      type: String,
      default: '',
    },
    imgL: {
      type: String,
      default: '',
    },
    isCloseBtn: {
      type: Boolean,
      default: true,
    },
    isReloadBtn: {
      type: Boolean,
      default: true,
    },
    isParentNode: {
      type: Boolean,
      default: false,
    }
  },
  emits: ['ok','close','reload'],
  setup(props, context) {
    const state = reactive({popupShow: false,verifyResult: false,terminal: 'pc',blkTilesW: 0,circleLeft:0,bgWidth: 0,isTouch: false,bgRandom: 0,loading: false,isLoad: false})

    const isMobile = () => {
      return navigator.userAgent.match(
        /(phone|pad|pod|iPhone|iPod|ios|iPad|Android|Mobile|BlackBerry|IEMobile|MQQBrowser|JUC|Fennec|wOSBrowser|BrowserNG|WebOS|Symbian|Windows Phone)/i
      );
    }

    const setTerminal = () => {
      return isMobile() ? 'mobile' : 'pc'
    }

    const reload = () => {
      context.emit('reload')
      initCanvas()
    }

    const close = () => {
      context.emit('close')

    }
    const getContainer = () => {
      return document.getElementById('canvas_containe')
    }

    const drag = (event,targetId,linkageId,isClick = false) => {
      state.isTouch = true
      const targetDom = document.querySelector(`#${targetId}`)
      const linkageDom = document.querySelector(`#${linkageId}`)
      const placehold = document.querySelector('#placehold')
      const terminal = state.terminal

      let x = 0
      const move = (moveEV) => {
        if (terminal === 'pc') {
          x = moveEV.x - event.x
        } else {
          // click事件触发changedTouches可能丢失
          x = moveEV.changedTouches[0].clientX - (event.changedTouches ? event.changedTouches[0].clientX : event.clientX)
        }
        //滑块拖动限定
        if (x < 8) {
          placehold.style.opacity = '1'
        }
        if (x >= state.bgWidth - L || x <= -2) return false
        targetDom.style.left = x + 'px'
        linkageDom.style.left = x + 'px'
        // console.log("move  x:",  x);
        state.circleLeft = x
        placehold.style.opacity = '0'
      }

      const up = (isVerify = true) => {
        state.isTouch = false
        document.removeEventListener('mousemove', move)
        document.removeEventListener('mouseup', up)

        document.removeEventListener('touchmove', move)
        document.removeEventListener('touchend', up)
        if (isVerify)context.emit('ok',state.circleLeft)
        targetDom.style.left = '0'
        linkageDom.style.left = '0'
        initCanvas()
      }

      if (terminal === 'pc') {
        document.addEventListener('mousemove', move)
        document.addEventListener('mouseup', up)
      } else {
        document.addEventListener('touchmove', move)
        document.addEventListener('touchend', up)
      }

      if (isClick) up(false)
    }
    const initCanvas = () => {
      if (state.isLoad) return

      state.isLoad = true
      state.loading = true

      const bg_canvas = document.getElementById('bg_canvas')
      const bg_ctx = bg_canvas.getContext('2d')

      const block_canvas = document.getElementById('block_canvas')
      const block_ctx = block_canvas.getContext('2d')

      const placehold = document.getElementById('placehold')
      placehold.style.opacity = '1'

      const random = (max, min) => {
        return Math.floor(Math.random() * (min - max) + max)
      }
      const img = new Image()
      img.crossOrigin = 'Anonymous'
      img.src = props.imgL
      img.onerror = () => {}

      let width = props.width

      if (props.isParentNode) {
        const sliderVerify = document.getElementById('plugin-slider-verify_containe')
        const s_verify_width =
          sliderVerify.parentElement?.getBoundingClientRect().width

        s_verify_width ? (width = s_verify_width - 20) : null
        if (props.isBorder) {
          width = width - 2
        }
      }

      const height = props.height
      state.bgWidth = width
      const blkTilesW = random(L + 10, width - (L + 10))
      state.blkTilesW = blkTilesW

      bg_canvas.width = width
      bg_canvas.height = height
      block_canvas.width = props.w
      block_canvas.height = props.h

      img.onload = () => {
        state.loading = false
        bg_ctx.drawImage(img, 0, 0, width, height)
        state.isLoad = false
      }

      const imgBlock = new Image()
      imgBlock.crossOrigin = 'Anonymous'
      imgBlock.src = props.imgX
      imgBlock.onerror = () => {}
      imgBlock.onload = () => {
        block_ctx.drawImage(imgBlock, 0, 0, props.w, props.h)
        block_canvas.style.top = `${props.top}px`
      }
    }


    watch(() => props.imgL,  (newval) => {
      initCanvas()
    },{ deep: true }) 
    onMounted(() => {
      initCanvas()
    })

    return {
      context,
      ...toRefs(state),
      drag,
      close,
      reload,
      getContainer,
      setTerminal
    }
  },
})
</script>

<style lang="less" scoped>
#slider-verify {
  width: auto;
  height: auto;
  display: inline-block;
  padding: 10px;
  border-radius: 5px;
  overflow: hidden;

  .is-border {
    border: 1px solid rgb(199, 198, 198);
  }

  .is-opt {
    padding-bottom: 45px;
  }

  #verify_containe {
    position: relative;

    #canvas_containe {
      position: relative;
      line-height: 0;
      .is-border {
        border: 1px solid rgb(199, 198, 198);
      }

      #block_canvas {
        position: absolute;
        left: 0;
        cursor: pointer;
      }

      .loading {
        display: flex;
        justify-content: center;
        align-items: center;
      }
    }

    .slide-box {
      width: 100%;
      height: 40px;
      margin-top: 15px;
      border-radius: 20px;
      background: #DFE0E1;
      position: relative;
      color: #A3ABB3;
      display: flex;
      align-items: center;
      justify-content: center;

      #circle {
        width: 50px;
        height: 50px;
        top: -8px;
        left: 0;
        border-radius: 50px;
        position: absolute;
        background: white;
        border: 1px solid #D0D0D0;
        cursor: pointer;
        display: flex;
        justify-content: center;
        align-items: center;

        .verticals {
          display: flex;
          align-items: center;

          img {
            width: 8px;
            height: 16px;
          }
        }

        .arrow {
          display: flex;
          align-items: center;

          img {
            width: 13px;
            height: 16px;
          }

          img.circle {
            width: 13px;
            height: 13px;
          }
        }
      }

      #placehold {
        transition: opacity 0.3s;
        user-select: none;
      }
    }

    .operational {
      position: absolute;
      width: 100%;
      height: 32px;
      left: -10px;
      bottom: 0;
      border-top: 1px solid #EAE8E8;
      padding: 0 10px;
      display: flex;
      align-items: flex-end;

      img:first-child {
        margin-left: 0;
      }

      img {
        width: 22px;
        height: 22px;
        margin-left: 10px;
        cursor: pointer;
      }
    }
  }
}
</style>
