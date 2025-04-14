package xcrypto

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"os"
	"slices"
	"testing"
)

func TestExample(t *testing.T) {
	t.Run("key", TestGenerateKey)
	private, _ := os.ReadFile("private.pem")
	public, _ := os.ReadFile("public.pem")
	svr := NewServer(private)
	cli := NewClient(public)

	msg1 := slices.Repeat([]byte("Hello world"), 10)
	fmt.Println(">>>>>>>msg:", len(msg1))
	cliMsg1, err := cli.Encrypt(msg1)
	if err != nil {
		t.Fatal(err)
	}
	fmt.Println(">>>>>>>cliMsg1:", len(cliMsg1))
	svrMsg1, key, err := svr.Decrypt(cliMsg1)
	if err != nil {
		t.Fatal(err)
	}
	fmt.Println(svrMsg1)
	if !slices.Equal(msg1, svrMsg1) {
		t.Fatal("服务端解密后的数据不正确")
	}
	svrMsg2, err := svr.Encrypt(msg1, key)
	if err != nil {
		t.Fatal(err)
	}
	fmt.Println(">>>>>>>svrMsg2:", len(svrMsg2))
	cliMsg2, err := cli.Decrypt(svrMsg2)
	if err != nil {
		t.Fatal(err)
	}
	fmt.Println(cliMsg2)
	if !slices.Equal(msg1, svrMsg1) {
		t.Fatal("客户端解密后的数据不正确")
	}
}

func TestGenerateKey(t *testing.T) {
	// 生成 RSA 密钥对
	privateKey, err := rsa.GenerateKey(rand.Reader, 4096)
	if err != nil {
		fmt.Println("Error generating key:", err)
		return
	}

	// 保存私钥到文件
	privateKeyFile, err := os.Create("private.pem")
	if err != nil {
		fmt.Println("Error creating private key file:", err)
		return
	}
	defer privateKeyFile.Close()

	privateKeyBytes := x509.MarshalPKCS1PrivateKey(privateKey)
	privateKeyPEM := &pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: privateKeyBytes,
	}
	err = pem.Encode(privateKeyFile, privateKeyPEM)
	if err != nil {
		fmt.Println("Error writing private key to file:", err)
		return
	}

	// 生成公钥
	publicKey := &privateKey.PublicKey

	// 保存公钥到文件
	publicKeyFile, err := os.Create("public.pem")
	if err != nil {
		fmt.Println("Error creating public key file:", err)
		return
	}
	defer publicKeyFile.Close()

	publicKeyBytes, err := x509.MarshalPKIXPublicKey(publicKey)
	if err != nil {
		fmt.Println("Error marshalling public key:", err)
		return
	}
	publicKeyPEM := &pem.Block{
		Type:  "PUBLIC KEY",
		Bytes: publicKeyBytes,
	}
	err = pem.Encode(publicKeyFile, publicKeyPEM)
	if err != nil {
		fmt.Println("Error writing public key to file:", err)
		return
	}

	fmt.Println("Keys generated and saved to files.")
}
