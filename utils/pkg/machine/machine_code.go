/*
author: superl[N.S.T]
github: https://github.com/super-l/
desc: 获取操作系统的相关硬件基础编码信息
*/
package machine

import (
	"errors"
	"net"
	"runtime"
	"strings"
)

func GetMachineData() (data Information) {
	var osMachine OsMachineInterface
	if runtime.GOOS == "darwin" {
		osMachine = MacMachine{}
	} else if runtime.GOOS == "linux" {
		osMachine = LinuxMachine{}
	} else if runtime.GOOS == "windows" {
		osMachine = WindowsMachine{}
	}
	machineData := osMachine.GetMachine()
	machineData.LocalMacInfo, _ = GetMACAddress()
	return machineData
}

func GetBoardSerialNumber() (data string, err error) {
	var osMachine OsMachineInterface
	if runtime.GOOS == "darwin" {
		osMachine = MacMachine{}
	} else if runtime.GOOS == "linux" {
		osMachine = LinuxMachine{}
	} else if runtime.GOOS == "windows" {
		osMachine = WindowsMachine{}
	}
	return osMachine.GetBoardSerialNumber()
}

func GetPlatformUUID() (data string, err error) {
	var osMachine OsMachineInterface
	if runtime.GOOS == "darwin" {
		osMachine = MacMachine{}
	} else if runtime.GOOS == "linux" {
		osMachine = LinuxMachine{}
	} else if runtime.GOOS == "windows" {
		osMachine = WindowsMachine{}
	}
	return osMachine.GetPlatformUUID()
}

func GetCpuSerialNumber() (data string, err error) {
	var osMachine OsMachineInterface
	if runtime.GOOS == "darwin" {
		osMachine = MacMachine{}
	} else if runtime.GOOS == "linux" {
		osMachine = LinuxMachine{}
	} else if runtime.GOOS == "windows" {
		osMachine = WindowsMachine{}
	}
	return osMachine.GetCpuSerialNumber()
}

func GetMACAddress() (string, error) {
	netInterfaces, err := net.Interfaces()
	if err != nil {
		return "", err
	}
	var mac string
	var bakMac1 string
	var bakMac2 string

	for i := 0; i < len(netInterfaces); i++ {
		flags := netInterfaces[i].Flags.String()

		if strings.Contains(flags, "up") &&
			strings.Contains(flags, "broadcast") &&
			strings.Contains(flags, "running") &&
			!strings.Contains(flags, "loopback") {

			// fmt.Println(fmt.Sprintf("i:%d name:%s %v", i, netInterfaces[i].Name, flags))
			if strings.Contains(netInterfaces[i].Name, "WLAN") {
				mac = netInterfaces[i].HardwareAddr.String()
				return mac, nil
			}
			if !strings.Contains(netInterfaces[i].Name, "VMware") {
				bakMac1 = netInterfaces[i].HardwareAddr.String()
			} else {
				bakMac2 = netInterfaces[i].HardwareAddr.String()
			}
		}
	}
	if mac == "" {
		if bakMac1 != "" {
			return bakMac1, nil
		}
		return bakMac2, nil
	}
	return mac, errors.New("unable to get the correct MAC address")
}

func GetLocalIpAddr() (string, error) {
	conn, err := net.Dial("udp", "8.8.8.8:53")
	if err != nil {
		return "", err
	}
	localAddr := conn.LocalAddr().(*net.UDPAddr)
	ip := strings.Split(localAddr.String(), ":")[0]
	return ip, nil
}

func GetIpAddrAll() ([]string, error) {
	var ipList []string
	addrList, err := net.InterfaceAddrs()
	if err != nil {
		return ipList, err
	}
	for _, address := range addrList {
		if ipNet, ok := address.(*net.IPNet); ok && !ipNet.IP.IsLoopback() && !ipNet.IP.IsLinkLocalUnicast() {
			if ipNet.IP.To4() != nil {
				ipList = append(ipList, ipNet.IP.To4().String())
			}
		}
	}
	return ipList, nil
}
