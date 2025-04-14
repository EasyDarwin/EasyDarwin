package msgpush

import (
	"context"
	"encoding/json"
	"fmt"
	"log/slog"
	"strconv"
	"time"

	"easydarwin/lnton/pkg/orm"

	"easydarwin/lnton/pkg/conc"
	"easydarwin/lnton/pkg/web"
)

type AlarmMsg struct {
	DeviceID  string
	ChannelID string
	Msg       string
}

type Storer interface {
	MsgPush() MsgPushStorer
}

type Core struct {
	storer            Storer
	SSE               *web.SSE
	pushMsgWithSaveCh chan AlarmMsg // 消息队列，用于接收系统报警信息

	pushMsgCh chan *Message

	session conc.Map[*web.SSE, struct{}]
}

func NewCore(storer Storer) *Core {
	core := Core{
		storer:            storer,
		SSE:               web.NewSSE(200, time.Minute),
		pushMsgCh:         make(chan *Message, 24),
		pushMsgWithSaveCh: make(chan AlarmMsg, 200),
	}
	// 测试时请打开下列代码
	// go core.msgtest()
	go core.notifyWithSave()
	go core.notifyAll()
	return &core
}

func (c *Core) AddSession(sse *web.SSE) {
	c.session.Store(sse, struct{}{})
}

func (c *Core) DelSession(sse *web.SSE) {
	c.session.Delete(sse)
}

// PushMsgWithSave 带存储的消息推送
func (c *Core) PushMsgWithSave(msg AlarmMsg) {
	select {
	case c.pushMsgWithSaveCh <- msg:
	default:
		slog.Info("PushMsgWithSave 丢失", "len", len(c.pushMsgCh))
	}
}

// PushMsgWithSave 带存储的消息推送
// 主要处理消息的存储逻辑
func (c *Core) notifyWithSave() {
	for msg := range c.pushMsgWithSaveCh {
		// 检查报警信息
		// 暂时没想好要检查什么，根据实际项目来吧！

		// 保存到数据库
		if err := c.storer.MsgPush().Add(context.Background(), &MsgPush{
			ID:        strconv.FormatInt(time.Now().Unix(), 10) + orm.GenerateRandomString(5),
			DeviceID:  msg.DeviceID,
			ChannelID: msg.ChannelID,
			Msg:       msg.Msg,
		}); err != nil {
			// 处理错误
			slog.Error("MsgPush Add error", "err", err, "device", msg.DeviceID, "channel", msg.ChannelID, "msg", msg.Msg)
			// 发生错误就不提示给前端了
			continue
		}

		// 广播消息，分发给所有会话
		// 可以建立多个通道区分等级，或分组推送
		c.PushMsg(&Message{
			Msg: msg.Msg,
		})
	}
}

// PushMsg 推送消息
// 负责分发消息，如果写满了将消息丢弃
func (c *Core) PushMsg(msg *Message) {
	select {
	case c.pushMsgCh <- msg:
	default:
		slog.Info("PushMsg 丢失", "len", len(c.pushMsgCh))
	}
}

// notifyAll 遍历所有SSE链接，推送消息
// 消费消息队列
func (c *Core) notifyAll() {
	for msg := range c.pushMsgCh {
		b, _ := json.Marshal(msg)
		c.session.Range(func(key *web.SSE, _ struct{}) bool {
			key.Publish(web.Event{
				Event: "msg",
				Data:  b,
			})
			return true
		})
	}
}

func (c *Core) msgtest() {
	for {
		c.pushMsgWithSaveCh <- AlarmMsg{
			DeviceID:  "1",
			ChannelID: "1",
			Msg:       time.Now().Format(time.RFC3339),
		}
		fmt.Println(">>>>>>>>>>>>>>>>>>", c.session.Len())
		time.Sleep(3 * time.Second)
	}
}
