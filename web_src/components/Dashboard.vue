<template>
  <div class="container-fluid no-padding">
    <div class="row">
      <div class="col-md-6">
          <div class="panel">
              <div class="panel-body">
                  <ve-line :data="cpuData" :settings="memSettings" :legend-visible="false" :title="{text: 'CPU使用', left: 'center'}"></ve-line>
              </div>
          </div>
      </div>      
      <div class="col-md-6">
          <div class="panel">
              <div class="panel-body">
                  <ve-line :data="memData" :settings="memSettings" :legend-visible="false" :title="{text: '内存使用', left: 'center'}"></ve-line>
              </div>
          </div>
      </div>      
      <div class="col-md-6">
          <div class="panel">
              <div class="panel-body">
                  <ve-line :data="pusherData" :settings="cntSettings" :legend-visible="false" :title="{text: '推流统计', left: 'center'}"></ve-line>
              </div>
          </div>
      </div>      
      <div class="col-md-6">
          <div class="panel">
              <div class="panel-body">
                  <ve-line :data="playerData" :settings="cntSettings" :legend-visible="false" :title="{text: '拉流统计', left: 'center'}"></ve-line>
              </div>
          </div>
      </div>      
    </div>
  </div>
</template>

<script>
import moment from "moment";
import { mapState, mapActions } from "vuex"

export default {
  data() {
    return {
      timer: 0,
      memSettings: {
        area: true,
        xAxisType: "time",
        yAxisType: ["percent"],
        min: [0],
        max: [1]
      },
      cntSettings: {
        area: true,
        xAxisType: "time",
        yAxisType: ["normal"],
        min: [0],
        max: [100]
      }
    };
  },
  mounted() {
    this.timer = setInterval(() => {
      this.getServerInfo();
    }, 2000);
  },
  beforeDestroy() {
    if (this.timer) {
      clearInterval(this.timer);
      this.timer = 0;
    }
  },
  computed: {
    ...mapState([
      "serverInfo"
    ]),
    cpuData() {
      return {
        columns: ["time", "使用"],
        rows: this.serverInfo ? this.serverInfo.cpuData : []
      }
    },
    memData() {
      return {
        columns: ["time", "使用"],
        rows: this.serverInfo ? this.serverInfo.memData : []
      }
    },
    pusherData() {
      return {
        columns: ["time", "总数"],
        rows: this.serverInfo ? this.serverInfo.pusherData : []
      }
    },
    playerData() {
      return { 
        columns: ["time", "总数"],
        rows: this.serverInfo ? this.serverInfo.playerData : []
      }
    }    
  },
  methods: {
    ...mapActions([
      "getServerInfo"
    ])
  }
};
</script>

