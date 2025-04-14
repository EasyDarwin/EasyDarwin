// Author: xiexu
// Date: 2022-09-20

package server

import (
	"fmt"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"testing"
	"time"
)

func TestServer(t *testing.T) {
	svr := New(http.NewServeMux(), Port("8081"), DefaultPrintln())

	interrupt := make(chan os.Signal, 1)
	signal.Notify(interrupt, syscall.SIGINT, syscall.SIGTERM)

	select {
	case s := <-interrupt:
		fmt.Printf("s(%s) := <-interrupt\n", s.String())
	case err := <-svr.Notify():
		fmt.Printf("err(%s) = <-server.Notify()\n", err)
	case <-time.After(2 * time.Second):
		fmt.Println("timeout")
	}
	if err := svr.Shutdown(); err != nil {
		fmt.Printf("err(%s) := server.Shutdown()\n", err)
	}
}
