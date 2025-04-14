package user

import (
	"context"
	"crypto/sha256"
	"encoding/hex"
	"errors"
	"fmt"
	"log/slog"
	"math/rand/v2"
	"slices"
	"strings"
	"time"

	"easydarwin/utils/pkg/web"

	"easydarwin/utils/pkg/orm"

	"easydarwin/utils/pkg/fn"
	"easydarwin/utils/pkg/vcode"
	"github.com/lib/pq"
)

const (
	userMinID      = 1000000 //
	DefaultGroupID = 111
	DefaultUserID  = 222
)

type UserStorer interface {
	GetUserByUsername(v *User, username string) error    // 查询用户
	GetUserByID(v *User, id int) error                   // 查询用户
	UpdatePasswordAttempts(id int, limitdAt int64) error // 更新密码尝试次数
	UpdateUser(v *User, id int, fn func(group *User)) error
	UpdateUserByUserName(v *User, un string, fn func(user *User)) error           // 更新用户
	DeleteUser(id, groupID int) error                                             // 删除用户
	DelUserByUserName(un string, groupID int) error                               // 通过用户名删除用户
	CreateUser(*User) error                                                       // 创建用户
	FindUsers(v *[]*User, pUID int, in *FindUsersInput, Level int) (int64, error) // 查询用户列表
	FindApps(v *[]*User, limit, offset int) (int64, error)
	DeleteApp(int) error // 删除开发者账户
	FirstOrCreate(any) (bool, error)

	UpdateUser2(ctx context.Context, model *User, username int, fn func(*User) error) error
}

type Storer interface {
	GroupStorer
	UserStorer
	InsertOne(orm.Tabler) error                                    // 插入数据
	UpdateOne(model orm.Tabler, id int, data map[string]any) error // 更新
	GetCaptchaByID(v *Vcode, id int) error                         // 查询验证码
	UpdateVcodeUsed(id int, username string) error                 // 更新验证码为已使用
}

// Core 结构体定义了一个核心结构体，包含以下字段：
type Core struct {
	// Ctx：上下文，用于控制协程的执行
	Ctx context.Context
	// Store：存储器，用于存储数据
	Store Storer
	// vc：验证码，用于验证用户输入
	vc *vcode.SlideCaptcha
	// log：日志，用于记录程序运行过程中的信息
	log *slog.Logger
	// Message chan string
	ue UserEventer

	disabledCaptcha *bool
}

/*
	用户模块的设计:
		权限控制:
			1.用户角色采用用户等级划分,等级越小权限越大，Level=1为超级管理员，Level=2为管理员，Level=3为普通用户
				匹配出错时，会返回默认值0，所以取消了Level=0的情况
		关联查询:
			1.该模块用户名为UserName是唯一表示，不能重复;name则为昵称，用户随便填写
			2.在表的关联查询中，舍去了ID，使用UserName作为唯一标识，进行关联查询
			3.前端传递用户资源表示时，同样使用UserName,不使用ID
			4.特殊情况:
         关系表示:
			1.为了防止用户嵌套多次创建，需要限制层级，Level用来表示身份，所以使用Grade表示层级
*/
// UserEventer 接口定义了对用户记录操作的相关事件
type UserEventer interface {
	// 函数名前面加On通常用于表示该函数是一个事件处理函数（Event Handler）

	// CreateUser 在创建用户时触发

	// OnDelUser 在删除用户时触发
	OnDelUser(username string)
}

type Option func(*Core)

// WithDisabledCaptcha true:禁用验证码
func WithDisabledCaptcha(disabled *bool) Option {
	return func(c *Core) {
		c.disabledCaptcha = disabled
	}
}

// NewCore 函数用于创建一个新的 Core 实例
// 验证码，默认启用
func NewCore(store Storer, log *slog.Logger, ue UserEventer, opts ...Option) Core {
	// 返回一个 Core 实例，包含传入的 store 和 log 参数，以及一个新的 vcode 实例
	vc, err := vcode.NewSlideCaptcha()
	if err != nil {
		slog.Error("NewSlideCaptcha", "err", err)
	}
	c := Core{
		Store: store,
		vc:    vc,
		log:   log,
		ue:    ue,
	}

	for _, opt := range opts {
		opt(&c)
	}

	return c
}

// 初始化函数，用于创建用户组和用户
func (c Core) Init(groupName, username, password string) error {
	// 创建用户组
	g := UserGroup{
		ID:         DefaultGroupID,                // 默认用户组ID
		Sort:       1,                             // 用户组排序
		PID:        0,                             // 父用户组ID
		Name:       groupName,                     // 用户组名称
		Level:      1,                             // 用户组级别
		Tree:       pq.Int64Array{DefaultGroupID}, // 用户组树
		ChildCount: 0,                             // 子用户组数量
		UserCount:  1,                             // 用户数量
		UID:        1,                             // 用户ID
		CreatedAt:  orm.Now(),                     // 创建时间
	}

	// 将用户组保存到数据库中
	_, err := c.Store.FirstOrCreate(&g)
	if err != nil {
		return err
	}

	// 2024/09/23 目前没有使用用户组，所以直接创建用户，角色分为admin，user两种

	// 创建系统管理员用户
	u := User{
		ID:        222,      // 用户ID
		Name:      "系统管理员",  // 用户名称
		Remark:    "系统创建",   // 用户备注
		Username:  username, // 用户名
		Level:     1,
		Role:      "admin",          // 用户角色
		Enabled:   true,             // 用户是否启用
		GroupID:   g.ID,             // 用户所属用户组ID
		GroupTree: pq.Int64Array{1}, // 用户所属用户组树
		Tree:      pq.Int64Array{},  // 用户树
		Grade:     1,
		FirstLoginInfo: FirstLoginInfo{
			ResetPW: true,
		},
	}
	// 对密码进行加密
	s := sha256.Sum256([]byte(password))
	b, _ := GenerateFromPassword(hex.EncodeToString(s[:]))
	u.Password = b
	// 将用户保存到数据库中
	if _, err := c.Store.FirstOrCreate(&u); err != nil {
		return err
	}

	// 创建开发者用户
	{
		dev := User{
			ID:        223,             // 用户ID
			Name:      "gbs",           // 用户名称
			Username:  "administrator", // 用户名
			Remark:    "系统创建",          // 用户备注
			Level:     1,
			Role:      "admin",           // 用户角色
			Type:      UserTypeDeveloper, // 用户类型
			Enabled:   true,              // 用户是否启用
			GroupID:   g.ID,              // 用户所属用户组ID
			GroupTree: pq.Int64Array{1},  // 用户所属用户组树
			Tree:      pq.Int64Array{},   // 用户树
			Grade:     1,
		}
		// 对密码进行加密
		s := sha256.Sum256([]byte("B139DE0A-0268-4FD9-8E55-E1DE308748D5"))
		b, _ := GenerateFromPassword(hex.EncodeToString(s[:]))
		dev.Password = b
		// 将用户保存到数据库中
		if _, err := c.Store.FirstOrCreate(&dev); err != nil {
			return err
		}
	}
	// 创建第三方调用账户
	{
		u := User{
			ID:        224,         // 用户ID
			Name:      "接口调用",      // 用户名称
			Remark:    "系统创建",      // 用户备注
			Username:  "api_admin", // 用户名
			Level:     1,
			Role:      "admin",          // 用户角色
			Enabled:   true,             // 用户是否启用
			GroupID:   g.ID,             // 用户所属用户组ID
			GroupTree: pq.Int64Array{1}, // 用户所属用户组树
			Tree:      pq.Int64Array{},  // 用户树
			Grade:     1,
		}
		// 对密码进行加密
		s := sha256.Sum256([]byte(orm.GenerateRandomString(20)))
		b, _ := GenerateFromPassword(hex.EncodeToString(s[:]))
		u.Password = b
		// 将用户保存到数据库中
		if _, err := c.Store.FirstOrCreate(&u); err != nil {
			return err
		}
	}

	return err
}

// 根据pid获取用户组级别和树
func (c Core) getGroupLevelAndTree(pid int) (int8, pq.Int64Array, error) {
	// 如果pid为0，则返回1和空的pq.Int64Array
	if pid == 0 {
		return 1, pq.Int64Array{}, nil
	}

	// 定义一个UserGroup类型的变量pNode
	var pNode UserGroup
	// 根据pid获取用户组信息
	if err := c.Store.GetGroupByID(&pNode, pid); errors.Is(err, orm.ErrRevordNotFound) {
		// 如果用户组不存在，则返回0和错误信息
		return 0, nil, web.ErrDB.Msg("用户组不存在").Withf("pid[%d] 不存在", pid)
	} else if err != nil {
		// 如果发生其他错误，则返回0和错误信息
		return 0, nil, web.ErrDB.Withf(`err[%s] := c.Store.GetGroupByID(&pNode, pid[%d])`, err, pid)
	}
	// 返回用户组级别和树
	return pNode.Level + 1, append(pNode.Tree, int64(pNode.ID)), nil
}

type CreateCaptchaOutput struct {
	ID      int    `json:"captcha_id"`
	Master  string `json:"master"`
	Tile    string `json:"tile"`
	W       int    `json:"w"`
	H       int    `json:"h"`
	X       int    `json:"x"`
	Y       int    `json:"y"`
	Expired int    `json:"expired"` // 过期时间，单位秒
}

// 每个用户 1 分钟只能请求 10 次验证码
// 每个 ip 1 分钟只能请求 100 次验证码
func (c Core) CreateCaptcha(username, ip string) (*CreateCaptchaOutput, error) {
	if *c.disabledCaptcha {
		return &CreateCaptchaOutput{}, nil
	}
	const duration = 3 * 60
	// 生成问题与答案
	data, m, t, err := c.vc.GenerateIdQuestionAnswer()
	if err != nil {
		return nil, web.ErrServer.Msg(err.Error())
	}
	// 存储到数据库
	v := Vcode{
		Question:  "",
		Answer:    fmt.Sprintf("%d,%d", data.X, data.Y),
		CreatedAt: time.Now(),
		ExpiresAt: time.Now().Add(duration * time.Second),
		Remark:    "登录",
		IP:        ip,
		Key:       username,
	}

	if v.Answer == "" {
		return nil, web.ErrServer.With("Q&A == empty", "生成验证码错误")
	}
	if err := c.Store.InsertOne(&v); err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.Store.InsertOne(&v)`, err)
	}
	// 绘制成图片返回
	// img, err := c.vc.DrawCaptcha(q)
	// if err != nil {
	// return 0, "", web.ErrServer.Withf(`img, err[%s] := c.vc.DrawCaptcha(q[%s])`, err, q)
	// }
	// return v.ID, img, nil
	return &CreateCaptchaOutput{
		ID:      v.ID,
		Master:  m,
		Tile:    t,
		W:       data.Width,
		H:       data.Height,
		X:       0, //   data.TileX,
		Y:       data.Y,
		Expired: duration,
	}, nil
}

// 定义UserInput结构体，用于存储用户输入的信息
type UserInput struct {
	Remark string   `json:"remark"` // 用户备注
	IPs    []IP     `json:"ips"`    // 用户IP地址
	Macs   []string `json:"macs"`   // 用户MAC地址
}

// 定义EditUserInput结构体，用于存储编辑用户的信息
type EditUserInput struct {
	UserInput // 继承UserInput结构体
}

// 定义CreateUserInput结构体，用于存储创建用户的信息
type CreateUserInput struct {
	Name     string `json:"name"`     // 昵称
	Username string `json:"username"` // 用户名
	Password string `json:"password"` // 用户密码
	// Role      string `json:"role"`     // 用户角色
	IsAdmin   bool   `json:"is_admin"` // 是否是管理员
	GroupID   int    `json:"group_id"` // 用户组ID
	Phone     string `json:"phone"`    // 用户手机号
	Email     string `json:""`         // 用户邮箱
	Enabled   bool   `json:"enabled"`  // 用户是否启用
	Remark    string `json:"remark"`   // 用户备注
	UserInput        // 继承UserInput结构体
}

// 将CreateUserInput结构体转换为User结构体
func (v CreateUserInput) ToUser() User {
	// 生成密码
	pw, _ := GenerateFromPassword(v.Password)
	// 如果IPs为空，则初始化为空数组
	if v.IPs == nil {
		v.IPs = make([]IP, 0)
	}
	// 如果Macs为空，则初始化为空数组
	if v.Macs == nil {
		v.Macs = make([]string, 0)
	}
	// 返回User结构体
	return User{
		Name:     v.Name,
		Username: v.Username, // 用户名
		Password: pw,         // 密码
		Phone:    v.Phone,
		Email:    v.Email,  // 邮箱
		Remark:   v.Remark, // 备注
		// Role:      v.Role,    // 角色
		Enabled:   v.Enabled, // 是否启用
		CreatedAt: orm.Now(), // 创建时间
		FirstLoginInfo: FirstLoginInfo{ // 首次登录信息
			ResetPW: true, // 是否重置密码
		},
		GroupID: v.GroupID, // 用户组ID
	}
}

// FindApps 查询开发者列表
// FindApps 函数用于查找应用，返回应用列表、总数量和错误信息
func (c Core) FindApps(limit, offset int) ([]*FindAppsOutput, int64, error) {
	// 创建一个用户切片，用于存储查询到的用户信息
	users := make([]*User, 0, 10)
	// 调用 Store 的 FindApps 方法，查询用户信息，返回总数量和错误信息
	total, err := c.Store.FindApps(&users, limit, offset)
	// 如果查询出错，返回错误信息
	if err != nil {
		return nil, 0, web.ErrDB.Withf("total, err[%s] := c.Store.FindApps(&users, limit, offset)", err)
	}
	// 创建一个 FindAppsOutput 切片，用于存储查询到的应用信息
	out := make([]*FindAppsOutput, 0, len(users))
	// 遍历查询到的用户信息，将用户信息转换为应用信息
	for _, user := range users {
		out = append(out, &FindAppsOutput{
			ID:        user.ID,
			AppID:     user.Username,
			CreatedAt: user.CreatedAt,
		})
	}
	// 返回应用信息、总数量和错误信息
	return out, total, err
}

// DeleteApp 删除开发者账户
// DeleteApp函数用于删除指定用户的应用
func (c Core) DeleteApp(targetUID, uid int) error {
	// 定义一个User类型的变量user
	var user User
	// 调用Store的GetUserByID函数，根据uid获取用户信息
	if err := c.Store.GetUserByID(&user, uid); errors.Is(err, orm.ErrRevordNotFound) {
		// 如果用户不存在，返回错误信息
		return web.ErrNotFound.With("用户不存在")
	} else if err != nil {
		// 如果发生其他错误，返回错误信息
		return web.ErrDB.With("ResetSecret->GetUserByID", err.Error())
	}

	// 如果用户类型为开发者，返回错误信息
	if user.Type == UserTypeDeveloper {
		return web.ErrPermissionDenied.Withf("开发者用户不允许操作")
	}

	// 调用Store的DeleteApp函数，删除指定用户的应用
	if err := c.Store.DeleteApp(targetUID); err != nil {
		// 如果发生错误，返回错误信息
		return web.ErrDB.Withf("err[%s] := c.Store.DeleteApp(targetUID)", err)
	}
	// 返回nil，表示删除成功
	return nil
}

// ResetSecret 重置密码
// ResetSecret函数用于重置指定用户的密码
func (c Core) ResetSecret(targetUID, uid int) (string, error) {
	// 生成一个24位的随机字符串作为密码
	secret := generateRandomString(24)

	// 定义一个User类型的变量
	var user User
	// 根据uid获取用户信息
	if err := c.Store.GetUserByID(&user, uid); errors.Is(err, orm.ErrRevordNotFound) {
		// 如果用户不存在，返回错误信息
		return "", web.ErrNotFound.With("用户不存在")
	} else if err != nil {
		// 如果获取用户信息失败，返回错误信息
		return "", web.ErrDB.With("ResetSecret->GetUserByID", err.Error())
	}

	// 如果用户类型为开发者，返回错误信息
	if user.Type == UserTypeDeveloper {
		return "", web.ErrPermissionDenied.Withf("开发者用户不允许操作")
	}

	// 更新用户密码
	var u User
	if err := c.Store.UpdateUser(&u, targetUID, func(u *User) {
		// 生成密码
		u.Password, _ = GenerateFromPassword(secret)
		// 重置密码尝试次数
		u.PasswordAttempts = 0
		// 重置最后登录时间
		u.LastLoginInfo.LimitedAt = 0
	}); err != nil {
		return "", web.ErrDB.With(err.Error())
	}

	// 返回生成的密码
	return secret, nil
}

// EditApp 函数用于编辑应用
func (c Core) EditApp(uid, targetUID int, ips []string) error {
	// 定义一个用户变量
	var user User
	// 从数据库中获取用户信息
	if err := c.Store.GetUserByID(&user, uid); errors.Is(err, orm.ErrRevordNotFound) {
		// 如果用户不存在，返回错误
		return web.ErrNotFound.With("用户不存在")
	} else if err != nil {
		// 如果获取用户信息失败，返回错误
		return web.ErrDB.With("ResetSecret->GetUserByID", err.Error())
	}

	// 如果用户类型为开发者，返回错误
	if user.Type == UserTypeDeveloper {
		return web.ErrPermissionDenied.Withf("开发者用户不允许操作")
	}

	// 定义一个IP数组
	limitIPs := make([]IP, 0, len(ips))
	// 去重
	ips = fn.Deduplication(ips...)
	// 遍历IP数组
	for _, v := range ips {
		// 如果IP为空，跳过
		if v == "" {
			continue
		}
		// 将IP添加到IP数组中
		limitIPs = append(limitIPs, IP{Start: v, End: v})
	}

	// 验证IP数组
	if err := VaildIPs(limitIPs); err != nil {
		return err
	}
	return nil
}

// CreateApp 创建开发者账户
// CreateApp 创建应用
func (c Core) CreateApp(appID string, ips []string) (string, error) {
	// 检查应用ID是否合法
	name, err := CheckName(appID)
	if err != nil {
		return "", err
	}

	// 去重
	limitIPs := make([]IP, 0, len(ips))
	ips = fn.Deduplication(ips...)
	for _, v := range ips {
		if v == "" {
			continue
		}
		limitIPs = append(limitIPs, IP{Start: v, End: v})
	}

	// 检查IP是否合法
	if err := VaildIPs(limitIPs); err != nil {
		return "", err
	}
	// 生成随机字符串作为密钥
	secret := generateRandomString(24)
	passwd, _ := GenerateFromPassword(secret)
	// 创建用户
	if err := c.Store.CreateUser(&User{
		Name:     "",
		Username: name,
		Enabled:  true,
		Remark:   "开发者账户",
		Level:    1,
		Type:     UserTypeDeveloper,
		Password: passwd,
	}); orm.IsDuplicatedKey(err) {
		return "", web.ErrBadRequest.Msg("AppID 重复")
	} else if err != nil {
		return "", web.ErrDB.Withf("%s", err)
	}

	return secret, nil
}

// CreateUser 创建用户
func (c Core) CreateUser(v *CreateUserInput, pUID int) (*User, error) {
	// 检查用户名是否合法
	name, err := CheckName(v.Username)
	if len(v.Username) < 7 {
		return nil, web.ErrBadRequest.Msg("账号长度不能小于 7 位").Withf("user[%s]", v.Username)
	}
	if err != nil {
		return nil, err
	}
	v.Username = name
	// 检查密码长度是否合法
	if length := len(v.Password); length > 64 && length < 8 {
		return nil, web.ErrBadRequest.Msg("密码长度错误").With("应采用 sha256 加密")
	}
	// 检查MAC地址是否合法
	if err := VaildMacs(v.Macs); err != nil {
		return nil, err
	}
	// 检查IP地址是否合法
	if err := VaildIPs(v.IPs); err != nil {
		return nil, err
	}
	// 将输入转换为用户对象
	user := v.ToUser()
	// 查询用户组信息
	var g UserGroup
	if err := c.Store.GetGroupByID(&g, v.GroupID); errors.Is(err, orm.ErrRevordNotFound) {
		return nil, web.ErrBadRequest.Msg("用户组不存在").With(fmt.Sprintf("groupID:%d 不存在", v.GroupID), err.Error())
	} else if err != nil {
		return nil, web.ErrDB.With(err.Error())
	}
	// user.RGroupID = g.RID
	user.GroupTree = g.Tree

	// 查询操作者信息
	var pUser User
	if err := c.Store.GetUserByID(&pUser, pUID); err != nil {
		return nil, web.ErrUsedLogic.With(err.Error(), fmt.Sprintf(`c.store.GetUserByID(&pUser, pUID[%d])`, pUID))
	}
	// pUser.Role是空字符串，外层增加了用户等级判断的中间件，所以把下列代码注释了
	//if pUser.Role != "admin" {
	//	return nil, web.ErrPermissionDenied
	//}
	// if err := vaildCreateUserPower(g.RID); err != nil {
	// return nil, err
	// }

	// 检查操作者是否被禁用
	if !pUser.Enabled {
		return nil, web.ErrUnauthorizedToken.Msg("用户已被禁用")
	}
	// 检查操作者是否超出最大层级
	{
		if pUser.Grade >= MaxLevel {
			return nil, web.ErrUsedLogic.Msg("不允许创建，超出最大层级")
		}
		user.Grade = pUser.Grade + 1
	}

	// 设置用户信息
	user.PID = pUser.ID
	if v.IsAdmin {
		user.Level = 2
	} else {
		user.Level = 3
	}
	// user.Level = pUser.Level + 1
	user.Tree = append(pUser.Tree, int64(pUser.ID))
	// 创建用户
	if err := c.Store.CreateUser(&user); errors.Is(err, orm.ErrDuplicatedKey) {
		return nil, web.ErrBadRequest.Msg("用户名重复").Withf("username[%s]", user.Username)
	} else if err != nil {
		return nil, web.ErrDB.Withf("err[%s] := c.Store.CreateUser(&user)", err)
	}
	return &user, nil
}

type UpdateUserInput struct {
	UserName string `json:"-"`
	Name     string `json:"name"`     // 昵称
	Role     string `json:"role"`     // 角色
	Phone    string `json:"phone"`    // 手机号
	Password string `json:"password"` // 密码
	Enabled  bool   `json:"enabled"`  // 是否启用
	Email    string `json:"email"`    // 邮箱
	Remark   string `json:"remark"`   // 备注
}

func (c Core) UpdateUser(pUID int, in *UpdateUserInput) error {
	// 查询操作者信息
	var pUser User
	if err := c.Store.GetUserByID(&pUser, pUID); err != nil {
		return web.ErrUsedLogic.With(err.Error(), fmt.Sprintf(`c.store.GetUserByID(&pUser, pUID[%d])`, pUID))
	}
	if pUser.Role != "admin" {
		return web.ErrPermissionDenied
	}
	// 更新用户密码
	var u User
	if err := c.Store.UpdateUserByUserName(&u, in.UserName, func(u *User) {
		u.Name = in.Name
		u.Role = in.Role
		u.Phone = in.Phone
		u.Email = in.Email
		u.Enabled = in.Enabled
		if in.Password != "" {
			// todo:校验密码
			u.Password, _ = GenerateFromPassword(in.Password)
		}
		u.Remark = in.Remark
	}); err != nil {
		return web.ErrDB.With(err.Error())
	}
	return nil
}

func (c Core) DelUser(pUID int, in *DelUserInput) error {
	// 查询操作者信息
	var pUser User
	if err := c.Store.GetUserByID(&pUser, pUID); err != nil {
		return web.ErrUsedLogic.With(err.Error(), fmt.Sprintf(`c.store.GetUserByID(&pUser, pUID[%d])`, pUID))
	}
	// 查询被删除者的信息
	var u User
	if err := c.Store.GetUserByUsername(&u, in.UserName); err != nil {
		return web.ErrUsedLogic.Msg("用户不存在").With(err.Error(), fmt.Sprintf(`c.store.GetUserByUsername(&u,in.UserName)`, in.UserName))
	}
	// 校验用户权限
	if pUser.Role != "admin" {
		return web.ErrPermissionDenied
	}
	// 用户不能同级别的用户
	if pUser.Level >= u.Level {
		return web.ErrPermissionDenied
	}
	if err := c.Store.DelUserByUserName(in.UserName, in.GroupID); err != nil {
		return err
	}
	if c.ue != nil {
		c.ue.OnDelUser(in.UserName) // 删除用户关联关系
	}
	return nil
}

// VaildDeleteUserPermissions 验证删除用户权限
// hUID：要删除的用户ID
// tree：用户权限树
// isAdmin：是否为管理员
// 返回值：错误信息
func VaildDeleteUserPermissions(hUID int64, tree pq.Int64Array, isAdmin bool) error {
	// 上级用户可以删除
	if isExist := fn.Any(tree, func(v int64) bool {
		return v == hUID
	}); isExist {
		return nil
	}
	return web.ErrPermissionDenied
}

func (c Core) GetUserByUsername(username string) (*User, error) {
	var user User
	if err := c.Store.GetUserByUsername(&user, username); err != nil {
		return nil, web.ErrDB.Msg("用户不存在").Withf("err[%s] := c.Store.GetUserByID(&hUser, handleUID)", err)
	}
	return &user, nil
}

// ResetPassword 函数用于重置密码
func (c Core) ResetPassword(username string, password, newPassword string) error {
	// 判断新密码长度是否小于8
	if len(newPassword) < 8 {
		return web.ErrBadRequest.Msg("密码不合法").With(newPassword)
	}

	// 获取操作用户信息
	{
		var hUser User
		// 根据操作用户ID获取用户信息
		if err := c.Store.GetUserByUsername(&hUser, username); err != nil {
			return web.ErrDB.Msg("用户不存在").Withf("err[%s] := c.Store.GetUserByID(&hUser, handleUID)", err)
		}
		// 判断操作用户是否被禁用
		if !hUser.Enabled {
			return web.ErrUnauthorizedToken
		}
		// 判断操作用户密码是否正确
		if err := CompareHashAndPasswordsd(hUser.Password, password); err != nil {
			return web.ErrNameOrPasswd.With(err.Error())
		}

	}

	// 获取目标用户信息
	var targetUser User
	// 根据目标用户ID获取用户信息
	if err := c.Store.GetUserByUsername(&targetUser, username); errors.Is(err, orm.ErrRevordNotFound) {
		return web.ErrNotFound.With("用户不存在")
	} else if err != nil {
		return web.ErrDB.With("ResetPassword->GetUserByID", err.Error())
	}

	// 更新用户密码
	var u User
	if err := c.Store.UpdateUserByUserName(&u, username, func(u *User) {
		// 生成新密码的哈希值
		u.Password, _ = GenerateFromPassword(newPassword)
		// 重置密码尝试次数
		u.PasswordAttempts = 0
		// 重置最后登录信息
		u.LastLoginInfo.LimitedAt = 0
	}); err != nil {
		return web.ErrDB.With(err.Error())
	}
	return nil
}

// ResetUsernameInput 重置用户名
type ResetUsernameInput struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

// ResetUsername 重置用户名
func (c Core) ResetUsername(ctx context.Context, targetUID int, in *ResetUsernameInput) error {
	if len(in.Username) < 7 {
		return web.ErrBadRequest.Msg("账号长度不能小于 7 位").Withf("user[%s]", in.Username)
	}
	if len(in.Password) < 12 {
		return web.ErrBadRequest.Msg("密码过于简单，不能小于 12 位").With(in.Password)
	}

	var targetUser User

	// 重置密码
	if err := c.Store.UpdateUser2(ctx, &targetUser, targetUID, func(u *User) error {
		u.Password, _ = GenerateFromPassword(in.Password)
		u.Username = in.Username
		u.PasswordAttempts = 0
		u.LastLoginInfo.LimitedAt = 0
		u.FirstLoginInfo.ResetPW = false
		return nil
	}); err != nil {
		if errors.Is(err, orm.ErrRevordNotFound) {
			return web.ErrNotFound.With("用户不存在")
		}
		return web.ErrDB.Withf("UpdateUser2 err[%s]", err.Error())
	}
	return nil
}

// 定义FindUsersInput结构体，用于查找用户
type FindUsersInput struct {
	// 用户名
	Username string `form:"username"`
	// 角色ID
	RoleID int `form:"role_id"`
	// 是否启用
	Enabled string `form:"enabled"`
	// 组ID
	GroupID int `form:"group_id"`
	// 用户权限等级
	Level int `form:"level"`
	// 分页过滤器
	web.PagerFilter
}

func (c Core) FindUsers(hUID int, in *FindUsersInput) (*[]*User, int64, error) {
	// 没有携带参数时，默认是查询自己当前顶级用户组的账号
	var group UserGroup
	// 根据传入的GroupID获取用户组信息
	if err := c.Store.GetGroupByID(&group, in.GroupID); errors.Is(err, orm.ErrRevordNotFound) {
		// 如果用户组不存在，返回错误信息
		return nil, 0, web.ErrNotFound.Msg("用户组不存在").Withf("err[%s] := c.Store.GetGroupByID(&group, in.GroupID[%d])", err, in.GroupID)
	} else if err != nil {
		// 如果获取用户组信息失败，返回错误信息
		return nil, 0, web.ErrDB.Withf("err[%s]", err)
	}
	// 参数权限检测

	var u User
	// 根据传入的hUID获取用户信息
	if err := c.Store.GetUserByID(&u, hUID); err != nil {
		// 如果获取用户信息失败，返回错误信息
		return nil, 0, web.ErrDB.Withf(`err[%s] := c.Store.GetUserByID(&u, hUID[%d])`, err, hUID)
	}
	var Level int = u.Level
	// 管理组全查
	// 顶级用户组管理员，顶级用户组内全查
	// 普通用户组，查询自己添加的角色
	users := make([]*User, 0, 8)
	// 提示:用户列表里面没有用户自己 ,否则用户可以修改自己权限
	if Level >= in.Level && in.Level != 0 { // 不能查看比自己权限大的用户
		return nil, 0, web.ErrPermissionDenied
	}
	total, err := c.Store.FindUsers(&users, hUID, in, Level)
	if err != nil {
		// 如果查询用户失败，返回错误信息
		return nil, total, web.ErrDB.With(err.Error())
	}

	return &users, total, nil
}

// SetUsersGroupName 函数用于设置用户组名
func (c Core) SetUsersGroupName(users []*User) {
	// 创建一个缓存，用于存储用户组
	cached := make(map[int64]UserGroup, 10)
	// 遍历用户列表
	for i := range users {
		user := users[i]
		// 创建一个字符串切片，用于存储用户组名
		gName := make([]string, 0, len(user.GroupTree))
		// 遍历用户组树
		for _, v := range user.GroupTree {
			// 从缓存中获取用户组
			group, ok := cached[v]
			// 如果缓存中没有该用户组，则从数据库中获取
			if !ok {
				if err := c.Store.GetGroupByID(&group, int(v)); err != nil {
					continue
				}
				// 将用户组存入缓存
				cached[v] = group
			}
			// 将用户组名存入字符串切片
			gName = append(gName, group.Name)
		}
		// 反转字符串切片
		slices.Reverse(gName)
		// 将字符串切片连接成一个字符串，并赋值给用户的GroupName字段
		user.GroupName = strings.Join(gName, "/")
	}
}

// generateRandomString 函数用于生成指定长度的随机字符串
func generateRandomString(length int) string {
	// 定义一个包含所有可能字符的字符串
	const letterBytes = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"
	// 创建一个字节数组，用于存储生成的随机字符串
	b := make([]byte, length)
	// 遍历字节数组
	for i := range b {
		// 从letterBytes中随机选择一个字符，并赋值给字节数组的对应位置
		b[i] = letterBytes[rand.N(len(letterBytes))]
	}
	// 将字节数组转换为字符串，并返回
	return string(b)
}
