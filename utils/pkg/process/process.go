package process

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"log/slog"
	"os"
	"os/exec"
	"sync/atomic"
	"syscall"
	"time"
)

// Process 进程管理，启动进程/停止进程/重启恢复进程/进程重启/进行守护
type Process struct {
	Cmd    *exec.Cmd
	cancel context.CancelFunc
	path   string
	args   []string

	flag atomic.Int32 // 0:停止;1:运行; 2:守护运行;3:守护停止;
}

func NewProcess(path string, args ...string) *Process {
	p := Process{path: path, args: args}
	_ = p.Kill()
	return &p
}

type Config struct {
	PID  int
	Args []string
}

// Run 运行程序，run 和 Daemon 只能运行一个
func (p *Process) Run(ctx context.Context) error {
	if !p.flag.CompareAndSwap(0, 1) {
		return fmt.Errorf("进程已在运行")
	}
	err := p.run(ctx)
	p.flag.Store(0)
	return err
}

func (p *Process) run(ctx context.Context) error {
	if len(p.args) == 0 {
		return fmt.Errorf("no args")
	}
	ctx, p.cancel = context.WithCancel(ctx)
	p.Cmd = exec.CommandContext(ctx, p.args[0], p.args[1:]...) // nolint
	p.Cmd.Stdout = os.Stdout
	p.Cmd.Stdin = os.Stdin
	p.Cmd.Env = os.Environ()
	if err := p.Cmd.Start(); err != nil {
		return err
	}

	b, _ := json.Marshal(Config{PID: p.Cmd.Process.Pid, Args: p.args})
	_ = os.WriteFile(p.path, b, 0o600)
	return p.Cmd.Wait()
}

// Stop 停止运行
func (p *Process) Stop() error {
	p.flag.Store(3)
	return p.Kill()
}

// Kill 通知程序结束
func (p *Process) Kill() error {
	if cmd := p.Cmd; cmd != nil {
		if err := cmd.Process.Signal(syscall.SIGTERM); err != nil && errors.Is(err, os.ErrProcessDone) {
			slog.Error("failed to kill process", "err", err)
		}
	}
	if p.cancel != nil {
		p.cancel()
	}
	_ = p.killWithFile()
	return nil
}

func (p *Process) killWithFile() error {
	b, err := os.ReadFile(p.path)
	if err != nil {
		return err
	}
	var cfg Config
	if err := json.Unmarshal(b, &cfg); err != nil {
		return err
	}
	if len(cfg.Args) == 0 {
		p.args = cfg.Args
	}
	pro, err := os.FindProcess(cfg.PID)
	if err != nil {
		return err
	}
	return pro.Signal(syscall.SIGTERM)
}

// Reboot 进程重启
func (p *Process) Reboot(ctx context.Context) error {
	if err := p.Kill(); err != nil {
		slog.Error("kill process failed", "err", err)
	}
	if p.flag.Load() >= 2 {
		return nil
	}
	return p.run(ctx)
}

// Daemon 进程守护
func (p *Process) Daemon(ctx context.Context) error {
	if !p.flag.CompareAndSwap(0, 2) {
		return fmt.Errorf("进程已在运行中")
	}
	for {
		if p.flag.CompareAndSwap(3, 0) {
			return nil
		}
		if err := p.run(ctx); err != nil {
			slog.Error("run process failed", "err", err)
		}
		time.Sleep(10 * time.Second)
	}
}
