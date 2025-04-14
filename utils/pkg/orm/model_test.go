package orm

import (
	"encoding/json"
	"fmt"
	"log/slog"
	"sync"
	"testing"
	"time"
)

type Date struct {
	Date Time `json:"date"`
}

func TestUnmarshalJSON(t *testing.T) {
	// {
	// 	b, _ := json.Marshal(map[string]any{"date": 0})
	// 	fmt.Println(string(b))
	// 	var date Date
	// 	if err := json.Unmarshal(b, &date); err != nil {
	// 		t.Fatal(err)
	// 	}
	// 	fmt.Println(date)
	// }
	// {
	// 	b, _ := json.Marshal(map[string]any{"date": nil})
	// 	fmt.Println(string(b))
	// 	var date Date
	// 	if err := json.Unmarshal(b, &date); err != nil {
	// 		t.Fatal(err)
	// 	}
	// 	fmt.Println(date)
	// }
	// {
	// 	b, _ := json.Marshal(map[string]any{"date": time.Now().Unix()})
	// 	fmt.Println(string(b))
	// 	var date Date
	// 	if err := json.Unmarshal(b, &date); err != nil {
	// 		t.Fatal(err)
	// 	}
	// 	fmt.Println(date)
	// }
	// {
	// 	b, _ := json.Marshal(map[string]any{"date": time.Now().UnixMilli()})
	// 	fmt.Println(string(b))

	// 	var date Date
	// 	if err := json.Unmarshal(b, &date); err != nil {
	// 		t.Fatal(err)
	// 	}
	// 	fmt.Println(date)
	// }
	// {
	// 	b, _ := json.Marshal(map[string]any{"date": time.Now().Format(time.DateTime)})
	// 	fmt.Println(string(b))

	// 	var date Date
	// 	if err := json.Unmarshal(b, &date); err != nil {
	// 		t.Fatal(err)
	// 	}
	// 	fmt.Println(date)
	// }

	{
		b, _ := json.Marshal(map[string]any{"date": `2024-11-21 07:37:44.811954573+05:00`})
		fmt.Println(string(b))

		var date Date
		if err := date.Date.Scan(`2024-11-21 07:37:44.811954573+05:00`); err != nil {
			t.Fatal(err.Error())
		}
		fmt.Println(date.Date)
		// if err := json.Unmarshal(b, &date); err != nil {
		// 	t.Fatal(err)
		// }
		// fmt.Println(date)

	}
	{
		b, _ := json.Marshal(map[string]any{"date": `2024-11-21 07:37:44.811954573+05:00`})
		fmt.Println(string(b))

		var date Date
		if err := date.Date.Scan(`2024-11-20 12:01:31`); err != nil {
			t.Fatal(err.Error())
		}
		fmt.Println(date.Date)
	}

	{
		b, _ := json.Marshal(map[string]any{"date": ""})
		fmt.Println(string(b))

		var date Date
		if err := json.Unmarshal(b, &date); err != nil {
			slog.Error("unmarshal", "err", err)
		}
		fmt.Println(date.Date)
	}
	{
		b, _ := json.Marshal(map[string]any{"date": 0})
		fmt.Println(string(b))

		var date Date
		if err := json.Unmarshal(b, &date); err != nil {
			slog.Error("unmarshal", "err", err)
		}
		fmt.Println(date.Date)
	}
}

func TestScan(t *testing.T) {
	for _, v := range []string{
		`2024-12-10 00:02:27.656272+08:00`,
		`2024-11-20 12:01:31`,
		`2024-11-21 07:37:44.811954573+05:00`,
		`2024-11-21 07:37:44.811954573-05:00`,
		`2024-11-21 07:37:44.811954573`,
		`2024-11-20 12:01:31+05:00`,
		`2024-11-20 12:01:31-05:00`,
		`2024/11/20 12:01:31`,
		`2024/11/20`,
		`2024-11-20`,
		"",
	} {

		var date Date
		if err := date.Date.Scan(v); err != nil {
			t.Fatal(err.Error())
		}
		fmt.Println(date.Date)
		fmt.Println(date.Date.Format(time.DateTime))

	}
}

func TestGenerateRandomString(t *testing.T) {
	var wg sync.WaitGroup

	var m sync.Map
	for range 10 {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for range 100 {
				s := GenerateRandomString(4)
				_, ok := m.LoadOrStore(s, struct{}{})
				if ok {
					fmt.Println("repeat", s)
				}
			}
		}()
	}
	wg.Wait()
}
