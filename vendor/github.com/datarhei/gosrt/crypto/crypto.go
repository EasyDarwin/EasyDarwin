// Package crypto provides SRT cryptography
package crypto

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/sha1"
	"encoding/binary"
	"errors"
	"fmt"

	"github.com/datarhei/gosrt/packet"
	"github.com/datarhei/gosrt/rand"

	"github.com/benburkert/openpgp/aes/keywrap"
	"golang.org/x/crypto/pbkdf2"
)

// Crypto implements the SRT data encryption and decryption.
type Crypto interface {
	// Generate generates an even or odd SEK.
	GenerateSEK(key packet.PacketEncryption) error

	// UnmarshalMK unwraps the key with the passphrase in a Key Material Extension Message. If the passphrase
	// is wrong an error is returned.
	UnmarshalKM(km *packet.CIFKeyMaterialExtension, passphrase string) error

	// MarshalKM wraps the key with the passphrase and the odd/even SEK for a Key Material Extension Message.
	MarshalKM(km *packet.CIFKeyMaterialExtension, passphrase string, key packet.PacketEncryption) error

	// EncryptOrDecryptPayload encrypts or decrypts the data of a packet with an even or odd SEK and
	// the sequence number.
	EncryptOrDecryptPayload(data []byte, key packet.PacketEncryption, packetSequenceNumber uint32) error
}

// crypto implements the Crypto interface
type crypto struct {
	salt      []byte
	keyLength int

	evenSEK []byte
	oddSEK  []byte
}

// New returns a new SRT data encryption and decryption for the keyLength. On failure
// error is non-nil.
func New(keyLength int) (Crypto, error) {
	// 3.2.2.  Key Material
	switch keyLength {
	case 16:
	case 24:
	case 32:
	default:
		return nil, fmt.Errorf("crypto: invalid key size, must be either 16, 24, or 32")
	}

	c := &crypto{
		keyLength: keyLength,
	}

	// 3.2.2.  Key Material: "The only valid length of salt defined is 128 bits."
	c.salt = make([]byte, 16)
	if err := c.prng(c.salt); err != nil {
		return nil, fmt.Errorf("crypto: can't generate salt: %w", err)
	}

	sek, err := c.generateSEK(c.keyLength)
	if err != nil {
		return nil, err
	}
	c.evenSEK = sek

	sek, err = c.generateSEK(c.keyLength)
	if err != nil {
		return nil, err
	}
	c.oddSEK = sek

	return c, nil
}

func (c *crypto) GenerateSEK(key packet.PacketEncryption) error {
	if !key.IsValid() {
		return fmt.Errorf("crypto: unknown key type")
	}

	sek, err := c.generateSEK(c.keyLength)
	if err != nil {
		return err
	}

	if key == packet.EvenKeyEncrypted {
		c.evenSEK = sek
	} else if key == packet.OddKeyEncrypted {
		c.oddSEK = sek
	}

	return nil
}

func (c *crypto) generateSEK(keyLength int) ([]byte, error) {
	sek := make([]byte, keyLength)

	err := c.prng(sek)
	if err != nil {
		return nil, fmt.Errorf("crypto: can't generate SEK: %w", err)
	}

	return sek, nil
}

// ErrInvalidKey is returned when the packet encryption is invalid
var ErrInvalidKey = errors.New("crypto: invalid key for encryption. Must be even, odd, or both")

// ErrInvalidWrap is returned when the packet encryption indicates a different length of the wrapped key
var ErrInvalidWrap = errors.New("crypto: the un/wrapped key has the wrong length")

func (c *crypto) UnmarshalKM(km *packet.CIFKeyMaterialExtension, passphrase string) error {
	if km.KeyBasedEncryption == packet.UnencryptedPacket || !km.KeyBasedEncryption.IsValid() {
		return ErrInvalidKey
	}

	n := 1
	if km.KeyBasedEncryption == packet.EvenAndOddKey {
		n = 2
	}

	wrapLength := n * c.keyLength

	if len(km.Wrap)-8 != wrapLength {
		return ErrInvalidWrap
	}

	if len(km.Salt) != 0 {
		copy(c.salt, km.Salt)
	}

	kek := c.calculateKEK(passphrase, c.salt, c.keyLength)

	unwrap, err := keywrap.Unwrap(kek, km.Wrap)
	if err != nil {
		return err
	}

	if len(unwrap) != wrapLength {
		return ErrInvalidWrap
	}

	if km.KeyBasedEncryption == packet.EvenKeyEncrypted {
		copy(c.evenSEK, unwrap)
	} else if km.KeyBasedEncryption == packet.OddKeyEncrypted {
		copy(c.oddSEK, unwrap)
	} else {
		copy(c.evenSEK, unwrap[:c.keyLength])
		copy(c.oddSEK, unwrap[c.keyLength:])
	}

	return nil
}

func (c *crypto) MarshalKM(km *packet.CIFKeyMaterialExtension, passphrase string, key packet.PacketEncryption) error {
	if key == packet.UnencryptedPacket || !key.IsValid() {
		return ErrInvalidKey
	}

	km.S = 0
	km.Version = 1
	km.PacketType = 2
	km.Sign = 0x2029
	km.KeyBasedEncryption = key // even or odd key
	km.KeyEncryptionKeyIndex = 0
	km.Cipher = 2
	km.Authentication = 0
	km.StreamEncapsulation = 2
	km.SLen = 16
	km.KLen = uint16(c.keyLength)

	if len(km.Salt) != 16 {
		km.Salt = make([]byte, 16)
	}
	copy(km.Salt, c.salt)

	n := 1
	if key == packet.EvenAndOddKey {
		n = 2
	}

	w := make([]byte, n*c.keyLength)

	if key == packet.EvenKeyEncrypted {
		copy(w, c.evenSEK)
	} else if key == packet.OddKeyEncrypted {
		copy(w, c.oddSEK)
	} else {
		copy(w[:c.keyLength], c.evenSEK)
		copy(w[c.keyLength:], c.oddSEK)
	}

	kek := c.calculateKEK(passphrase, c.salt, c.keyLength)

	wrap, err := keywrap.Wrap(kek, w)
	if err != nil {
		return err
	}

	if len(km.Wrap) != len(wrap) {
		km.Wrap = make([]byte, len(wrap))
	}

	copy(km.Wrap, wrap)

	return nil
}

func (c *crypto) EncryptOrDecryptPayload(data []byte, key packet.PacketEncryption, packetSequenceNumber uint32) error {
	// 6.1.2.  AES Counter
	//    0   1   2   3   4   5  6   7   8   9   10  11  12  13  14  15
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |                   0s                  |      psn      |  0   0|
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	//                            XOR
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |                    MSB(112, Salt)                     |
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	//
	// psn    (32 bit): packet sequence number
	// ctr    (16 bit): block counter, all zeros
	// nonce (112 bit): 14 most significant bytes of the salt
	//
	// CTR = (MSB(112, Salt) XOR psn) << 16

	if len(c.salt) != 16 {
		return fmt.Errorf("crypto: invalid salt. Must be of length 16 bytes")
	}

	ctr := make([]byte, 16)

	binary.BigEndian.PutUint32(ctr[10:], packetSequenceNumber)

	for i := range ctr[:14] {
		ctr[i] ^= c.salt[i]
	}

	var sek []byte
	if key == packet.EvenKeyEncrypted {
		sek = c.evenSEK
	} else if key == packet.OddKeyEncrypted {
		sek = c.oddSEK
	} else {
		return fmt.Errorf("crypto: invalid SEK selected. Must be either even or odd")
	}

	// 6.2.2.  Encrypting the Payload
	// 6.3.2.  Decrypting the Payload
	block, err := aes.NewCipher(sek)
	if err != nil {
		return err
	}

	stream := cipher.NewCTR(block, ctr)
	stream.XORKeyStream(data, data)

	return nil
}

// calculateKEK calculates a KEK based on the passphrase.
func (c *crypto) calculateKEK(passphrase string, salt []byte, keyLength int) []byte {
	// 6.1.4.  Key Encrypting Key (KEK)
	return pbkdf2.Key([]byte(passphrase), salt[8:], 2048, keyLength, sha1.New)
}

// prng generates a random sequence of byte into the given slice p.
func (c *crypto) prng(p []byte) error {
	_, err := rand.Read(p)
	return err
}
