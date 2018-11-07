<template>
<div class="container-fluid no-padding">
	<div class="col-lg-offset-2 col-lg-8 no-padding server-info">
		<div class="box box-widget">
			<div class="box-header">
				<h3> <i class="fa fa-support"></i> 版本信息</h3>
			</div>
			<div class="box-body table-responsive no-padding">
				<table class="table table-striped">
          <tbody>
            <tr>
                <td style="width:20%;">硬件信息</td>
                <td><span id="hardware-info">{{serverInfo.Hardware}}</span></td>
            </tr>
            <tr>
                <td>接口版本</td>
                <td><span id="interface-info">{{serverInfo.InterfaceVersion}}</span></td>
            </tr>
            <tr>
                <td>运行时间</td>
                <td><span id="running-time-info">{{runningTime || serverInfo.RunningTime}}</span></td>
            </tr>					
            <tr>
                <td>软件信息</td>
                <td><span id="software-info">{{serverInfo.Server}}</span></td>
            </tr>
          </tbody>
				</table>
			</div>
		</div>
	</div>
</div>
</template>

<script>
import { mapState, mapActions } from "vuex";
import moment from 'moment'
export default {
  data() {
    return {
      timer: 0,
      runningTime: ""
    };
  },
  computed: {
    ...mapState(["serverInfo"])
  },
  mounted() {
    this.timer = setInterval(() => {
        if(this.serverInfo && this.serverInfo.StartUpTime) {
          var start = moment(this.serverInfo.StartUpTime, "YYYY-MM-DD HH:mm:ss");
          var now = moment();
          var d = moment.duration(now.diff(start));
          this.runningTime = `${parseInt(d.asDays())} Days ${d.hours()} Hours ${d.minutes()} Mins ${d.seconds()} Secs`;          
        }
    }, 1000) 
  },
  beforeDestroy() {
      if(this.timer) {
        clearInterval(this.timer);
        this.timer = 0;
      }
  },
  methods: {}
}
</script>


