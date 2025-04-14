### 一、程序简介

machine-code，是GO语言实现的跨平台机器码(硬件信息)获取程序，包括PlatformUUID、主板序列号、CPU序列号、MAC网卡信息、精确IP地址等。同时支持windows、Linux、mac等系统！


#### 支持的系统：
    windows
    linux
    mac

### 二、安装说明

```
$ go get github.com/super-l/machine-code/machine
```


### 三、获取机器信息

实例代码：
```
machineData := machine.GetMachineData()
result, err := json.Marshal(machineData)
if err != nil {
    fmt.Println(err.Error())
}
fmt.Println(string(result))
```

运行结果：

```
{
    "platformUUID": "748869E5-06B6-5855-A0A4-7E19497XXXXX",
    "serialNumber": "C02P767XXXXX",
    "cpuId": "0x00040000",
    "mac": "34:36:3b:XX:XX:XX"
}

```

### 四、获取精准出口流量IP信息
```
func TestIp(t *testing.T) {
    macInfo2, err := machine.GetLocalIpAddr()
    if err != nil {
    fmt.Println(err.Error())
    }
    fmt.Println("Local Ip = ", macInfo2)
    return
}

结果：

=== RUN   TestIp
Local Ip =  192.168.2.224
--- PASS: TestIp (0.00s)
PASS
```