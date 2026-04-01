# Captcha with Etcd as store example


## captcha/captcha_etcd.go
```go
package captcha

import (
	"context"
	"github.com/mojocn/base64Captcha"
	"go.etcd.io/etcd/clientv3"
	"image/color"
	"library/database/etcd"
	"time"
)

//CaptchaEtcd base64 captcha with etcd
type CaptchaEtcd struct {
	*base64Captcha.DriverString
	store *etcd.Client
}

//NewClientEtcd constructor
func NewClientEtcd(height, width int, store *etcd.Client) *CaptchaEtcd {
	d := base64Captcha.NewDriverString(height, width, 0, 0, 4, "%#=qwe23456789rtyupasdfghjkzxcvbnm", &color.RGBA{0, 0, 0, 0}, []string{"wqy-microhei.ttc"})
	cli := &CaptchaEtcd{store: store}
	cli.DriverString = d
	return cli
}

const (
	captchaPrefix  = "captcha:"
	requestTimeout = time.Second
)

//GenerateIdAndImage create image
func (c *CaptchaEtcd) GenerateIdAndImage() (id, b64s, ans string, err error) {
	id, content, answer := c.GenerateIdQuestionAnswer()
	item, err := c.DrawCaptcha(content)
	if err != nil {
		return "", "", "", err
	}
	//expire in 120s
	grantResp, err := c.store.Grant(context.TODO(), 120)
	if err != nil {
		return "", "", "", err
	}
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	_, err = c.store.Put(ctx, captchaPrefix+id, answer, clientv3.WithLease(grantResp.ID))
	cancel()
	if err != nil {
		return "", "", "", err
	}
	b64s = item.EncodeB64string()
	return id, b64s, answer, nil
}

//Verify check captcha answer
func (c *CaptchaEtcd) Verify(id, answer string) (match bool, err error) {
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	key := captchaPrefix + id
	resp, err := c.store.Get(ctx, key)
	cancel()
	if err != nil {
		return false, err
	}

	for _, ev := range resp.Kvs {
		if string(ev.Value) == answer {
			return true, err
		}
	}
	return false, err
}

```

## captcha\captcha_etcd_test.go

```go
package captcha

import (
	"library/database/etcd"
	"testing"
)

func TestCaptchaEtcd_Verify(t *testing.T) {
	store, err := etcd.New([]string{"10.217.56.146:2379"}, requestTimeout, "", "", 0, 0, "", "", "")
	if err != nil {
		t.Error("etcd new failed ", err)
		return
	}
	cap := NewClientEtcd(80, 240, store)
	id, _, ans, err := cap.GenerateIdAndImage()
	if err != nil {
		t.Error("captcha generate failed ", err)
		return
	}
	ok, err := cap.Verify(id, ans)
	if err != nil {
		t.Error("clear false ", err)
		return
	}
	if !ok {
		t.Error("verify failed")
		return
	}

}
```