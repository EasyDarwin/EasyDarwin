package process

import (
	"context"
	"fmt"
	"testing"
	"time"
)

func TestProcess(t *testing.T) {
	p := NewProcess("./.p.dat", "./ntd")
	_ = p
	go func() {
		if err := p.run(context.TODO()); err != nil {
			fmt.Println(err)
		}
		fmt.Println(">>>>>>>. end")
	}()
	time.Sleep(3 * time.Second)
	// p.Kill()
	fmt.Println(">>>>>>>. start", p.Cmd.Process.Pid)
	time.Sleep(3 * time.Second)
}

func TestRace(t *testing.T) {
	p := NewProcess("./.p.dat", "./ntd")
	go func() {
		fmt.Println(p.Run(context.TODO()))
	}()
	go func() {
		fmt.Println(p.Daemon(context.TODO()))
	}()
	time.Sleep(5 * time.Second)
	p.Stop()
}

func TestReboot(t *testing.T) {
	p := NewProcess("./.p.dat", "./ntd")
	_ = p
	go func() {
		fmt.Println(p.Daemon(context.TODO()))
	}()
	time.Sleep(time.Second)
	p.Reboot(context.TODO())
	time.Sleep(3 * time.Second)
	p.Stop()
}

func TestStop(t *testing.T) {
	p := NewProcess("./.p.dat", `./ntd`, `-F`, `ip=192.168.3.1`)
	_ = p
	go func() {
		fmt.Println(p.Daemon(context.TODO()))
	}()
	time.Sleep(time.Second)
	p.Stop()
	time.Sleep(3 * time.Second)
}
