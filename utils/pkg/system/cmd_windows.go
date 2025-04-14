package system

import (
	"context"
	"os"
	"os/exec"
	"strings"
)

func ExecCommand(ctx context.Context, args []string) ([]byte, error) {
	newArgs := make([]string, 1, len(args)+1)
	newArgs[0] = "/c"
	newArgs = append(newArgs, args...)
	c := os.Getenv("ComSpec")
	cmd := exec.Command(c, newArgs...)
	cmd.Env = append(cmd.Env, os.Environ()...)
	s, err := cmd.CombinedOutput()
	if err != nil && strings.HasPrefix(err.Error(), "exit status") && len(s) > 0 {
		err = nil
	}
	return s, err
}
