# Keyboard
Simple library to listen for keystrokes from the keyboard

The code is inspired by [termbox-go](https://github.com/nsf/termbox-go) library.

### Installation
Install and update this go package with `go get -u github.com/eiannone/keyboard`

### Usage
Example of getting a single keystroke:

```go
char, _, err := keyboard.GetSingleKey()
if (err != nil) {
    panic(err)
}
fmt.Printf("You pressed: %q\r\n", char)
```

Example of getting a series of keystrokes:
```go
package main

import (
	"fmt"
	"github.com/eiannone/keyboard"
)

func main() {	
	err := keyboard.Open()
	if err != nil {
		panic(err)
	}
	defer keyboard.Close()

	fmt.Println("Press ESC to quit")
	for {
		char, key, err := keyboard.GetKey()
		if (err != nil) {
			panic(err)
		} else if (key == keyboard.KeyEsc) {
			break
		}
		fmt.Printf("You pressed: %q\r\n", char)
	}	
}
```
