// Author: xiexu
// Date: 2022-09-20

package system

import (
	"fmt"
	"testing"
)

func TestPortUsed(t *testing.T) {
	ok := PortUsed("tcp", 8080)
	t.Log(ok)
	ok = PortUsed("tcp", 8001)
	t.Log(ok)
	ok = PortUsed("tcp", 8000)
	t.Log(ok)
	ok = PortUsed("udp", 8000)
	t.Log(ok)
}

func TestPortCheck(t *testing.T) {
	//ok := PortUsed("tcp", 30001)
	//if !ok {
	//	t.Fatal("ok")
	//}

}
func TestTCPPortCheck(t *testing.T) {
	if err := TCPPortUsed(30001); err != nil {
		t.Fatal(err)
	} //TCPPortUsed(30001)
}

func TestUDPPortCheck(t *testing.T) {
	if err := UDPPortUsed(30001); err != nil {
		t.Fatal(err)
	} //TCPPortUsed(30001)
}

func TestIP2Info(t *testing.T) {
	info, err := IP2Info("120.231.246.103")
	if err != nil {
		t.Fatal(err)
	}
	fmt.Println(info.Err)
	fmt.Println(info.Addr)

	fmt.Printf("%+v", info)
}
