//go:build !windows

package system

import (
	"context"
	"os"
	"os/exec"
	"strings"
)

func ExecCommand(ctx context.Context, args []string) ([]byte, error) {
	cmdStr := strings.Join(args, " ")
	cmd := exec.CommandContext(ctx, "sh", "-c", cmdStr)
	cmd.Env = append(cmd.Env, os.Environ()...)
	return cmd.CombinedOutput()
}
