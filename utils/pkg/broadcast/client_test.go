package broadcast

import (
	"fmt"
	"runtime"
	"testing"
	"time"
)

func TestBroadcast(t *testing.T) {
	{
		cfg := Config{
			Version: "v2",
			WebAddr: "http://192.168.1.1",
			Mac:     "as:as:as:as:as",
			Name:    "11111111111",
		}
		s, _ := NewServer(cfg, NewServerHandler(cfg))
		// s.name = "11111111111"
		_ = s
	}

	cfg := Config{
		Version: "v1",
		WebAddr: "http://192.168.1.1",
		Mac:     "as:as:as:as:as",
		Name:    "22222222222",
	}
	s, _ := NewServer(cfg, NewServerHandler(cfg))
	// s.name = "22222222222"
	_ = s

	fmt.Println("start")

	// time.Sleep(10 * time.Second)

	{

		cfg := Config{
			Version: "v2",
			WebAddr: "http://192.168.1.2",
			Mac:     "as:as:as:as:as",
		}
		client123 := NewClient(cfg, NewServerHandler(cfg))
		if err := client123.Discover(""); err != nil {
			fmt.Println(err)
		}
	}

	fmt.Println("end")
	time.Sleep(5 * time.Second)

	// var wg sync.WaitGroup
	// for range 30000 {
	// 	wg.Add(1)
	// 	go func() {
	// 		defer wg.Done()
	// 		if err := client.Discover(""); err != nil {
	// 			fmt.Println(err)
	// 		}
	// 	}()
	// }
	// wg.Wait()

	// fmt.Println("Number of goroutines:", runtime.NumGoroutine())
	// printMemUsage()
	// runtime.GC()
	// runtime.GC()
	// runtime.GC()
	// fmt.Println(">>>>>>>>>>>>>>.")
	// fmt.Println("Number of goroutines:", runtime.NumGoroutine())
	// printMemUsage()
}

func printMemUsage() {
	var m runtime.MemStats
	runtime.ReadMemStats(&m)
	fmt.Printf("Alloc = %v MiB", m.Alloc/1024/1024)
	fmt.Printf("\tTotalAlloc = %v MiB", m.TotalAlloc/1024/1024)
	fmt.Printf("\tSys = %v MiB", m.Sys/1024/1024)
	fmt.Printf("\tNumGC = %v\n", m.NumGC)
}

func TestMulticast(t *testing.T) {
	go func() {
		g := NewServerHandler(Config{
			Name: "1111",
		})
		c, _ := NewServer(Config{
			Name: "1111",
		}, g)
		_ = c
	}()

	go func() {
		g := NewServerHandler(Config{
			Name: "2222",
		})
		c, _ := NewServer(Config{Name: "2222"}, g)
		_ = c
	}()

	gg := NewClient(Config{
		Name: "3333",
	}, NewServerHandler(Config{Name: "3333"}))
	_ = gg

	gg.Discover("")

	// c.conn.WriteTo([]byte("hello123"), &net.UDPAddr{IP: broadcastIP, Port: broadcastPort})
	// go func() {
	// 	for {
	// 		buf := make([]byte, 1024)
	// 		n, raddr, err := c.conn.ReadFrom(buf)
	// 		if err != nil {
	// 			panic(err)
	// 		}
	// 		fmt.Println(raddr.String(), string(buf[:n]))
	// 		// conn.WriteTo([]byte("ok"), raddr)
	// 	}
	// }()
	time.Sleep(10 * time.Second)
}
