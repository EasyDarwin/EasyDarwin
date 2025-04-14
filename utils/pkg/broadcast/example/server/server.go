package main

import (
	"flag"
	"time"
)

var aaa = flag.String("name", "test", "")

func main() {
	flag.Parse()
	// cfg := broadcast.Config{
	// 	Version: "v1",
	// 	WebAddr: "http://192.168.1.1",
	// 	Mac:     "as:as:as:as:as",
	// 	Name:    *aaa,
	// }
	// fmt.Println("strea")
	// s := broadcast.NewService(cfg, broadcast.NewServerHandler(cfg))
	// _ = s
	// defer s.Notify(false)
	// for range 3 {
	// 	if err := s.Notify(true); err != nil {
	// 		slog.Error("notfy", "err", err)
	// 	}
	// 	time.Sleep(2 * time.Second)
	// }

	time.Sleep(30 * time.Second)
}
