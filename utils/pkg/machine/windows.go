/*
author: superl[N.S.T]
github: https://github.com/super-l/
desc: 获取windows操作系统的相关硬件基础编码信息
*/
package machine

import (
	"os/exec"
	"strings"
)

type WindowsMachine struct{}

func (i WindowsMachine) GetMachine() Information {
	platformUUID, _ := i.GetPlatformUUID()
	boardSerialNumber, _ := i.GetBoardSerialNumber()
	cpuSerialNumber, _ := i.GetCpuSerialNumber()

	machineData := Information{
		PlatformUUID:      platformUUID,
		BoardSerialNumber: boardSerialNumber,
		CpuSerialNumber:   cpuSerialNumber,
	}
	return machineData
}

func (WindowsMachine) GetBoardSerialNumber() (serialNumber string, err error) {
	// wmic baseboard get serialnumber
	cmd := exec.Command("wmic", "baseboard", "get", "serialnumber")
	b, e := cmd.CombinedOutput()
	if e == nil {
		serialNumber = string(b)
		serialNumber = serialNumber[12 : len(serialNumber)-2]
		serialNumber = strings.ReplaceAll(serialNumber, "\n", "")
		serialNumber = strings.ReplaceAll(serialNumber, " ", "")
		serialNumber = strings.ReplaceAll(serialNumber, "\r", "")
	} else {
		return "", nil
	}
	return serialNumber, nil
}

func (WindowsMachine) GetPlatformUUID() (uuid string, err error) {
	// wmic csproduct get uuid
	var cmd *exec.Cmd
	cmd = exec.Command("wmic", "csproduct", "get", "uuid")
	b, e := cmd.CombinedOutput()

	if e == nil {
		uuid = string(b)
		uuid = uuid[4 : len(uuid)-1]
		uuid = strings.ReplaceAll(uuid, "\n", "")
		uuid = strings.ReplaceAll(uuid, " ", "")
		uuid = strings.ReplaceAll(uuid, "\r", "")
	} else {
		return "", nil
	}
	return uuid, nil
}

func (WindowsMachine) GetCpuSerialNumber() (cpuId string, err error) {
	// wmic cpu get processorid
	var cpuid string
	cmd := exec.Command("wmic", "cpu", "get", "processorid")
	b, e := cmd.CombinedOutput()

	if e == nil {
		cpuid = string(b)
		cpuid = cpuid[12 : len(cpuid)-2]
		cpuid = strings.ReplaceAll(cpuid, "\n", "")
		cpuid = strings.ReplaceAll(cpuid, " ", "")
		cpuid = strings.ReplaceAll(cpuid, "\r", "")
	} else {
		return "", nil
	}
	return cpuid, nil
}
