/*
author: superl[N.S.T]
github: https://github.com/super-l/
*/
package machine

import (
	"bytes"
	"errors"
	"os/exec"
	"strings"
)

type LinuxMachine struct{}

func (i LinuxMachine) GetMachine() Information {
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

func (LinuxMachine) GetBoardSerialNumber() (serialNumber string, err error) {
	// dmidecode -s system-serial-number  序列号
	var cmd *exec.Cmd
	cmd = exec.Command("dmidecode", "-s", "system-serial-number")

	var out bytes.Buffer
	cmd.Stdout = &out
	// cmd.Stderr = os.Stderr

	err = cmd.Start()
	if err != nil {
		return "", err
	}
	err = cmd.Wait()
	if err == nil {
		serial_number := out.String()
		serial_number = strings.Replace(serial_number, "\n", "", -1)
		return serial_number, nil
	} else {
		return "", err
	}
}

func (LinuxMachine) GetPlatformUUID() (UUID string, err error) {
	// dmidecode -s system-uuid           UUID
	var cmd *exec.Cmd
	cmd = exec.Command("dmidecode", "-s", "system-uuid")

	var out bytes.Buffer
	cmd.Stdout = &out
	// cmd.Stderr = os.Stderr

	err = cmd.Start()
	if err != nil {
		return "", err
	}
	err = cmd.Wait()
	if err == nil {
		uuid := out.String()
		uuid = strings.Replace(uuid, "\n", "", -1)
		return uuid, nil
	} else {
		return "", err
	}
}

func (LinuxMachine) GetCpuSerialNumber2() (cpuId string, err error) {
	// dmidecode -t processor |grep ID |head -1
	var cmd *exec.Cmd
	cmd = exec.Command("dmidecode", "-t", "processor", "|grep ID |head -1")

	var out bytes.Buffer
	cmd.Stdout = &out
	// cmd.Stderr = os.Stderr

	err = cmd.Start()
	if err != nil {
		return "", err
	}
	err = cmd.Wait()
	if err == nil {
		uuid := out.String()
		// uuid = strings.Replace(uuid, "\n", "", -1)
		return uuid, nil
	} else {
		return "", err
	}
}

func (i LinuxMachine) GetCpuSerialNumber() (cpuId string, err error) {
	// dmidecode -t processor |grep ID |head -1
	cmds := []*exec.Cmd{
		exec.Command("dmidecode", "-t", "processor"),
		exec.Command("grep", "ID"),
		exec.Command("head", "-1"),
	}
	cpuId, err = i.execPipeLine(cmds...)
	cpuId = strings.TrimSpace(cpuId)
	cpuId = strings.Replace(cpuId, "ID: ", "", -1)
	cpuId = strings.Replace(cpuId, "\t", "", -1)
	cpuId = strings.Replace(cpuId, "\n", "", -1)
	cpuId = strings.Replace(cpuId, " ", "-", -1)
	return
}

func (LinuxMachine) pipeline(cmds ...*exec.Cmd) (pipeLineOutput, collectedStandardError []byte, pipeLineError error) {
	if len(cmds) < 1 {
		return nil, nil, nil
	}

	var output bytes.Buffer
	var stderr bytes.Buffer

	last := len(cmds) - 1
	for i, cmd := range cmds[:last] {
		var err error
		if cmds[i+1].Stdin, err = cmd.StdoutPipe(); err != nil {
			return nil, nil, err
		}
		cmd.Stderr = &stderr
	}

	cmds[last].Stdout, cmds[last].Stderr = &output, &stderr

	for _, cmd := range cmds {
		if err := cmd.Start(); err != nil {
			return output.Bytes(), stderr.Bytes(), err
		}
	}

	for _, cmd := range cmds {
		if err := cmd.Wait(); err != nil {
			return output.Bytes(), stderr.Bytes(), err
		}
	}

	return output.Bytes(), stderr.Bytes(), nil
}

func (i LinuxMachine) execPipeLine(cmds ...*exec.Cmd) (string, error) {
	output, stderr, err := i.pipeline(cmds...)
	if err != nil {
		return "", err
	}

	if len(output) > 0 {
		return string(output), nil
	}

	if len(stderr) > 0 {
		return string(stderr), nil
	}
	return "", errors.New("no returns")
}
