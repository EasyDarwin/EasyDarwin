package xcrypto

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"crypto/rsa"
	"crypto/sha1" // nolint
	"crypto/x509"
	"encoding/json"
	"encoding/pem"
	"fmt"
	"io"

	"easydarwin/lnton/pkg/orm"
)

// Client 客户端
type Client struct {
	pub *rsa.PublicKey
	key string
}

func NewClient(publicKey []byte) *Client {
	// 解码公钥
	pubBlock, _ := pem.Decode(publicKey)
	// 读取公钥
	pubKeyValue, err := x509.ParsePKIXPublicKey(pubBlock.Bytes)
	if err != nil {
		panic(err)
	}
	pub := pubKeyValue.(*rsa.PublicKey)
	return &Client{pub: pub, key: orm.GenerateRandomString(24)}
}

type Data struct {
	Key  string
	Body []byte
}

// Encrypt 加密数据
func (c *Client) Encrypt(body []byte) ([]byte, error) {
	data := Data{Key: c.key, Body: body}
	b, _ := json.Marshal(data)
	return EncryptRSA(c.pub, b)
}

// Decrypt 解密数据
func (c *Client) Decrypt(body []byte) ([]byte, error) {
	return AesDecryptCFB(body, []byte(c.key))
}

// Server 服务端
type Server struct {
	pri *rsa.PrivateKey
}

func NewServer(privateKey []byte) *Server {
	// 解析出私钥
	priBlock, _ := pem.Decode(privateKey)
	priKey, err := x509.ParsePKCS1PrivateKey(priBlock.Bytes)
	if err != nil {
		panic(err)
	}
	return &Server{pri: priKey}
}

// Encrypt 加密数据
func (s *Server) Encrypt(body []byte, aeskey string) ([]byte, error) {
	return AesEncryptCFB(body, []byte(aeskey))
}

// Decrypt 解密数据
func (s *Server) Decrypt(body []byte) ([]byte, string, error) {
	data, err := DecryptRSA(s.pri, body)
	if err != nil {
		return nil, "", err
	}
	var out Data
	err = json.Unmarshal(data, &out)
	return out.Body, out.Key, err
}

// EncryptRSA 使用对方的公钥的数据, 只有对方的私钥才能解开
func EncryptRSA(pub *rsa.PublicKey, msg []byte) (cipherByte []byte, err error) {
	return rsa.EncryptOAEP(sha1.New(), rand.Reader, pub, msg, nil)
}

// DecryptRSA 使用私钥解密公钥加密的数据
func DecryptRSA(pri *rsa.PrivateKey, cipherByte []byte) (plainText []byte, err error) {
	return rsa.DecryptOAEP(sha1.New(), rand.Reader, pri, cipherByte, nil)
}

func AesEncryptCFB(origData []byte, key []byte) ([]byte, error) {
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}
	encrypted := make([]byte, aes.BlockSize+len(origData))
	iv := encrypted[:aes.BlockSize]
	if _, err := io.ReadFull(rand.Reader, iv); err != nil {
		panic(err)
	}
	stream := cipher.NewCFBEncrypter(block, iv)
	stream.XORKeyStream(encrypted[aes.BlockSize:], origData)
	return encrypted, nil
}

func AesDecryptCFB(encrypted []byte, key []byte) ([]byte, error) {
	block, _ := aes.NewCipher(key)
	if len(encrypted) < aes.BlockSize {
		return nil, fmt.Errorf("ciphertext too short")
	}
	iv := encrypted[:aes.BlockSize]
	encrypted = encrypted[aes.BlockSize:]

	stream := cipher.NewCFBDecrypter(block, iv)
	stream.XORKeyStream(encrypted, encrypted)
	return encrypted, nil
}
