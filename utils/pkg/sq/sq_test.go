package sq

import (
	"context"
	"fmt"
	"math/rand/v2"
	"strconv"
	"testing"
	"time"
)

var msg = make(chan string, 100000)

func mockReponse(key string) {
	sleep := rand.IntN(1000) + 100
	time.Sleep(time.Duration(sleep) * time.Millisecond)
	if key == "71" || key == "51" {
		return
	}

	msg <- key
	return
}

func TestSQ(t *testing.T) {
	sq := NewSimpleQueue[string]()
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// 模拟接收
	go func() {
		for v := range msg {
			stream := sq.GetStream(v)
			if stream != nil {
				stream.SetReponse(v)
			}
		}
	}()

	// 模拟发送
	for i := range 50 {
		go func(i int) {
			s := &Stream[string]{ID: strconv.Itoa(i), active: make(chan struct{}, 1), Ctx: ctx}
			resp, err := sq.Request(strconv.Itoa(i), s, func() {
				mockReponse(strconv.Itoa(i))
			})
			if err != nil {
				fmt.Println(i, err)
			}
			if s.ID != resp {
				fmt.Printf("s.id[%s] != resp[%s]  s[resp[%s]] \n ", s.ID, resp, s.response)
			}
		}(i)
	}
	for i := range 50 {
		go func(i int) {
			i += 50
			s := &Stream[string]{ID: strconv.Itoa(i), active: make(chan struct{}, 1), Ctx: ctx}

			resp, err := sq.Request(strconv.Itoa(i), s, func() {
				mockReponse(strconv.Itoa(i))
			})
			if err != nil {
				fmt.Println(i, err)
			}
			if s.ID != resp {
				fmt.Printf("s.id[%s] != resp[%s]  s[resp[%s]] \n ", s.ID, resp, s.response)
			}
		}(i)
	}
	time.Sleep(10 * time.Second)
	close(msg)
	fmt.Println("end")

	// sq.streams.Range(func(key string, value *Stream[string]) bool {
	// 	fmt.Println(key, ":", value.response)
	// 	return true
	// })
}
