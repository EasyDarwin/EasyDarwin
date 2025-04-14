<script setup>
import { watch, onBeforeUnmount,onMounted, ref } from 'vue'
import {UndoOutlined} from '@ant-design/icons-vue'
const emit = defineEmits(['contextmenuClose', 'statsInfo'])
const playerPro = ref(null)
const isPlayer = ref(true)
const isError = ref(false)
const EasyplayerProRef = ref(null)
const props = defineProps({
    background: {
        type: String,
        default: "",
    },
    videoUrl: {
        type: String,
        default: "",
    },
    watermark: {
        type: Object,
        default: () => { },
    },
    isLogo: {
        type: Boolean,
        default: true,
    },
    muted: {
        type: Boolean,
        default: false,
    },
})
const playCreate = () => {
    var container = EasyplayerProRef.value;
    let config = {
        isLive:false,
        // loadTimeOut:10,
        // bufferTime:0.4,
        // loadTimeReplay:3,
        MSE: true,
        // hasAudio: false,
        // poster:"",
        stretch: false,
        // isMute: true,
        isLogo: props.isLogo
    }
    playerPro.value = new EasyPlayerPro(container, config);
    playerPro.value.on("error", (type, msg) => {
        console.log("error:", type, msg);
        onDestroy()
        isError.value = true
        // setTimeout(() => {
        //   onReplay();
        // }, 2000);
    })
    playerPro.value.on("timeout", () => {
        // onReplay();
    })
    playerPro.value.on("delayTimeout", () => {
    })
    playerPro.value.on("playbackEnd", () => {
    })
    playerPro.value.on("liveEnd", () => {
        emit("contextmenuClose")
    })
    playerPro.value.on("recordEnd", (t) => {
    })
    playerPro.value.on("play", () => {
    })
    playerPro.value.on("contextmenuClose", () => {
        emit("contextmenuClose")
    });

    playerPro.value.on("stats", (value) => {
        let obj = value
        obj.videoPlaybackQuality = playerPro.value.valueplayer.getVideoPlaybackQuality();
        obj.videoInfo = (playerPro.value.valueplayer.video && playerPro.value.valueplayer.video.videoInfo) || {};
        obj.audioInfo = (playerPro.value.valueplayer.audio && playerPro.value.valueplayer.audio.audioInfo) || {};
        obj.kbpsShow = playerPro.value.valueplayer.control ? playerPro.value.valueplayer.control.kbpsShow : '0 KB/s';
        emit('statsInfo', obj)
    });
}
const onPlayer = () => {
    if (props.videoUrl == "") return;
    isError.value = false
    playerPro.value && playerPro.value
        .play(props.videoUrl)
        .then(() => { })
        .catch((e) => {
            console.error("playerPro error:", e);
            setTimeout(() => {
                onReplay();
            }, 2000);
        });
}
const onReplay = () => {
    isPlayer.value = false
    onDestroy().then(() => {
        isPlayer.value = true
        setTimeout(() => {
            onCreate().then(() => {
                onPlayer();
            });
        }, 100);
    });
}
const onCreate = () => {
    return new Promise((resolve, reject) => {
        playCreate();
        resolve();
    });
}
const onDestroy = () => {
    return new Promise((resolve, reject) => {
        onClose();
        setTimeout(() => {
            resolve();
        }, 100);
    });
}
const onClose = () => {

    if (playerPro.value) {
        playerPro.value.destroy();
        playerPro.value = null;
    }
}
watch(() => props.videoUrl, (newValue) => {
    if (newValue == "") {
        onClose();
    } else {
        onReplay();
    }
}, { deep: true })
onMounted(() => {
    playCreate();
    if (props.videoUrl != "") {
        onPlayer();
    }
})

onBeforeUnmount(() => {
    onClose();
})
</script>
<template>
    <div class="player-pro-vod">
        <div ref="EasyplayerProRef" v-if="isPlayer"></div>
        <span>
            <UndoOutlined v-if="isError" @click="onReplay"/>
        </span>
    </div>
</template>

<style scoped lang="less">
.player-pro-vod {
    width: 100%;
    height: 100%;
    background: #000;
    background-size: 100% 100%;
    background-repeat: no-repeat;
    .easyplayer-poster {
        background-size: 100% calc(100%);
    }
    span {
        position: absolute;
        left: calc(50% - 15px);
        top: calc(50% - 15px);
        font-size: 30px;
        color: #fff;
        cursor: pointer;
        border-radius: 20px;
    }

}

.player-pro-vod:hover {
    .easyplayer-controls {
        opacity: 1 !important;
    }
}

.player-pro-vod {
    .easyplayer-controls {
        opacity: 0 !important;
    }
}
</style>