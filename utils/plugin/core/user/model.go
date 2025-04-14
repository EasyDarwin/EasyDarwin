package user

import (
	"database/sql/driver"
	"encoding/json"
	"fmt"
	"net"
	"regexp"
	"strings"
	"time"

	"easydarwin/utils/pkg/web"

	"easydarwin/utils/pkg/orm"
	"gorm.io/gorm"

	"github.com/lib/pq"
	"golang.org/x/crypto/bcrypt"
)

const DefaultSequenceName = "common_id_seq"

var defaultSystemGroup = UserGroup{
	ID:        1,
	PID:       0,
	Name:      "系统",
	Level:     1,
	Sort:      1,
	UID:       1,
	UserCount: 1,
	IsLeaf:    false,
	// Options: model.Options{
	// 	CanModify: false,
	// 	CanDelete: false,
	// },
	IsExpanded: true,
}

type Vcode struct {
	ID        int        `gorm:"primaryKey;" json:"id"`
	Question  string     `gorm:"notNull;default:''" json:"question"`                  //  问题
	Answer    string     `gorm:"notNull;default:''" json:"answer"`                    //  答案
	CreatedAt time.Time  `gorm:"notNull;default:CURRENT_TIMESTAMP" json:"created_at"` //  创建时间
	ExpiresAt time.Time  `gorm:"notNull;default:CURRENT_TIMESTAMP" json:"expires_at"` //  过期时间
	UsedAt    *time.Time `json:"column:used_at"`                                      //  使用时间
	Remark    string     `gorm:"notNull;default:''" json:"remark"`                    //  用途
	IP        string     `gorm:"notNull;default:''" json:"ip"`                        //  ip
	Key       string     `gorm:"notNull;default:''" json:"key"`                       //  关键对象
}

func (*Vcode) TableName() string {
	return "vcodes"
}

func (v *Vcode) BeforeCreate(tx *gorm.DB) error {
	v.CreatedAt = orm.Now().Time
	return nil
}

const (
	UserTypeDefault   = ""    // 空串表示默认用户类型
	UserTypeDeveloper = "dev" // 开发者用户，不允许登录
)

type User struct {
	ID               int            `gorm:"column:id" json:"id"`                                                   // id
	Name             string         `gorm:"column:name;notNull;default:''" json:"name"`                            // 昵称
	Username         string         `gorm:"column:username;notNull;default:'';unique" json:"username"`             // 登录用户名
	Password         []byte         `gorm:"column:password" json:"-"`                                              // 登录密码
	Role             string         `gorm:"column:role;notNull;default:''" json:"role"`                            // 角色名称
	CreatedAt        orm.Time       `gorm:"column:created_at;notNull;CURRENT_TIMESTAMP" json:"created_at"`         // 创建时间
	Phone            string         `gorm:"column:phone;notNull;default:''" json:"phone"`                          // 手机号码
	Email            string         `gorm:"column:email;notNull;default:''" json:"email"`                          // 邮箱
	Enabled          bool           `gorm:"column:enabled;notNull;default:true" json:"enabled"`                    // 是否可用
	Wechat           string         `gorm:"column:wechat;type:jsonb;default:'{}'" json:"-"`                        // 微信相关内容
	Type             string         `gorm:"column:type;notNull;default:''" json:"type"`                            // 区分用户的类型
	PID              int            `gorm:"column:pid;notNull;default:0" json:"pid"`                               // 父级ID
	Remark           string         `gorm:"column:remark;notNull;default:''" json:"remark"`                        // 描述
	LastLoginInfo    LastLoginInfo  `gorm:"column:last_login_info;type:jsonb;default:'{}'" json:"last_login_info"` // 最后登录信息
	FirstLoginInfo   FirstLoginInfo `gorm:"column:first_login_info;type:jsonb;default:'{}'" json:"-"`              // 首次登录信息
	Tree             pq.Int64Array  `gorm:"column:tree;type:int8[]" json:"-"`                                      // 用户树，注意: 用户组树是从顶级到当前，而用户树是从 1 到上级用户
	Level            int            `gorm:"column:level;notNull;default:0" json:"level"`                           // 级别
	DeletedAt        orm.DeletedAt  `gorm:"deleted_at" json:"-"`                                                   // 删除时间
	PasswordAttempts int            `gorm:"column:password_attempts;notNull;default:0" json:"-"`                   // 密码输入错误次数
	GroupID          int            `gorm:"column:group_id;notNull;default:0" json:"group_id"`                     // 用户组
	GroupTree        pq.Int64Array  `gorm:"column:group_tree;type:int8[]" json:"-"`                                // 用户组树
	Grade            int            `gorm:"column:grade;notNull;default:0" json:"grade"`                           // 用户等级

	GroupName string `gorm:"-" json:"group_name"`
	RoleName  string `gorm:"-" json:"role_name"`

	// RGroupID int `gorm:"-" json:"-"` // 顶级用户组
}

func (l Limiter) Value() (driver.Value, error) {
	return json.Marshal(l)
}

type UserItem struct {
	ID        int      `json:"id"`
	Name      string   `json:"name"`
	Username  string   `json:"username"`
	Role      string   `json:"role"` // 角色 admin：可以进行任何操作, user：普通用户，只能查看
	Remark    string   `json:"remark"`
	CreatedAt orm.Time `json:"created_at"`
	Enabled   bool     `json:"enabled"`
	Level     int      `json:"level"`
	GroupID   int      `json:"group_id"`
	PID       int      `json:"pid"`

	GroupName string `json:"group_name"`
	RoleName  string `json:"role_name"`
}

type IP struct {
	Start string `json:"start"`
	End   string `json:"end"`
}

type UserGroup struct {
	ID         int           `gorm:"primaryKey;column:id" json:"id"`
	PID        int           `gorm:"column:pid;notNull;default:0" json:"pid"` // 父级用户组
	Name       string        `gorm:"column:name;notNull;default:''" json:"name"`
	Level      int8          `gorm:"column:level;notNull;default:0" json:"level"` // 1:管理组;2:顶级用户组;3:子级用户组....
	Tree       pq.Int64Array `gorm:"column:tree;notNull;type:int8[]" json:"-"`
	Sort       int           `gorm:"column:sort;notNull;default:0" json:"-"`
	ChildCount int           `gorm:"column:child_count;notNull;default:0" json:"-"`
	UserCount  int           `gorm:"column:user_count;notNull;default:0" json:"-"`
	UID        int           `gorm:"column:uid;notNull;default:0" json:"-"`
	CreatedAt  orm.Time      `gorm:"column:created_at;notNull;default:CURRENT_TIMESTAMP" json:"created_at"` // 创建时间
	UpdatedAt  orm.Time      `gorm:"column:updated_at;notNull;default:CURRENT_TIMESTAMP" json:"updated_at"` // 更新时间

	IsExpanded bool `gorm:"-" json:"is_expanded"` // 提示前端页面展开用户组
	IsLeaf     bool `gorm:"-" json:"is_leaf"`     // true:没有下级节点。
	// Options    model.Options `gorm:"-" json:"options"`     // 权限相关
	Children []*UserGroup `gorm:"-" json:"children"` // 子级用户组
}

// TableName implements orm.Tabler.
func (*User) TableName() string {
	return "users"
}

func (u *User) BeforeCreate(tx *gorm.DB) error {
	u.CreatedAt = orm.Now()
	return nil
}

func (u *User) CheckPasswordAttempts() time.Duration {
	switch true {
	case u.PasswordAttempts < 2:
		return 0
	case u.PasswordAttempts == 2:
		return 1 * time.Minute
	case u.PasswordAttempts == 5:
		return 10 * time.Minute
	case u.PasswordAttempts >= 8:
		return time.Hour
	}
	return 0
}

var (
	_ orm.Scaner = (*LastLoginInfo)(nil)
	_ orm.Scaner = (*FirstLoginInfo)(nil)
	_ orm.Scaner = (*Wechat)(nil)
)

type LastLoginInfo struct {
	IP        string `json:"ip"`
	Mac       string `json:"mac"`
	LimitedAt int64  `json:"limited_at"`
	Time      string `json:"time"`
}

// Value implements driver.Valuer.
func (l LastLoginInfo) Value() (driver.Value, error) {
	return json.Marshal(l)
}

// Scan implements orm.Scaner.
func (l *LastLoginInfo) Scan(input any) error {
	return orm.JsonUnmarshal(input, l)
}

var _ driver.Valuer = LastLoginInfo{}

type FirstLoginInfo struct {
	IP      string    `json:"ip"`       // 登录 ip
	Mac     string    `json:"mac"`      // 登录 mac address
	Time    time.Time `json:"time"`     // 登录时间
	ResetPW bool      `json:"reset_pw"` // 是否需要重置密码?
}

func (l FirstLoginInfo) Value() (driver.Value, error) {
	return json.Marshal(l)
}

type Wechat struct{}

// Scan implements orm.Scaner.
func (w *Wechat) Scan(input interface{}) error {
	return json.Unmarshal(input.([]uint8), w)
}

// Scan implements orm.Scaner.
func (f *FirstLoginInfo) Scan(input any) error {
	return orm.JsonUnmarshal(input, f)
}

type Limiter struct {
	Macs []string `json:"macs"`
	IPs  []IP     `json:"ips"`
}

func (l *Limiter) IPStartSlice() []string {
	out := make([]string, 0, len(l.IPs))
	for _, v := range l.IPs {
		out = append(out, v.Start)
	}
	return out
}

// VaildIPs 函数用于验证IP地址的有效性
func VaildIPs(v []IP) error {
	// 如果IP地址列表为空，则返回nil
	if len(v) == 0 {
		return nil
	}
	// 遍历IP地址列表
	for i, ip := range v {
		// 将IP地址转换为无符号整数
		start := ipToUint(ip.Start)
		// 如果转换失败，则返回错误信息
		if start == nil {
			return web.ErrBadRequest.Msg("ip 格式有误").With(fmt.Sprintf("start:%v", ip.Start))
		}
		// 将IP地址转换为无符号整数
		end := ipToUint(ip.End)
		// 如果转换失败，则返回错误信息
		if end == nil {
			return web.ErrBadRequest.Msg("ip 格式有误").With(fmt.Sprintf("end:%v", ip.End))
		}
		// 如果IP地址不在相同ip段，则返回错误信息
		if start[0] != end[0] {
			return web.ErrBadRequest.Msg("ip 格式有误").With(fmt.Sprintf("start:%v,end:%v，不在相同 ip 段", start, end))
		}
		// 如果IP地址的起始地址大于结束地址，则交换起始地址和结束地址
		if compareUint(start, end) == 1 {
			v[i].Start, v[i].End = ip.End, ip.Start
		}
	}
	// 返回nil
	return nil
}

// VaildMacs 函数用于验证mac地址的格式是否正确
func VaildMacs(macs []string) error {
	// 如果mac地址列表为空，则返回nil
	if len(macs) == 0 {
		return nil
	}
	// 使用正则表达式匹配mac地址的格式
	reg := regexp.MustCompile(`^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$`)
	// 遍历mac地址列表
	for i, mac := range macs {
		// 如果mac地址格式不正确，则返回错误信息
		if !reg.MatchString(mac) {
			return web.ErrBadRequest.Msg("mac 格式有误").With(fmt.Sprintf("mac:%v", mac))
		}
		// 将mac地址转换为大写
		macs[i] = strings.ToUpper(mac)
	}
	// 返回nil表示验证通过
	return nil
}

// CheckMac函数用于检查给定的MAC地址是否在限流器中
func (l Limiter) CheckMac(mac string) error {
	// 如果限流器中没有MAC地址，则返回nil
	if len(l.Macs) <= 0 {
		return nil
	}
	// 将给定的MAC地址转换为大写
	mac = strings.ToUpper(mac)
	// 遍历限流器中的MAC地址
	for _, v := range l.Macs {
		// 如果给定的MAC地址在限流器中，则返回nil
		if mac == v {
			return nil
		}
	}
	// 如果给定的MAC地址不在限流器中，则返回web.ErrLoginLimiter错误
	return web.ErrLoginLimiter
}

// CheckIP 函数用于检查IP地址是否在限流器中
func (l Limiter) CheckIP(ip string) error {
	// 如果限流器中没有IP地址，则直接返回nil
	if len(l.IPs) <= 0 {
		return nil
	}
	// 将IP地址转换为uint类型
	target := ipToUint(ip)
	// 如果转换失败，则返回错误
	if target == nil {
		return web.ErrBadRequest.With(ip, "ip 格式有误").With(fmt.Sprintf("ip:%v", ip))
	}
	// 遍历限流器中的IP地址
	for _, v := range l.IPs {
		// 将IP地址的起始和结束地址转换为uint类型
		start := ipToUint(v.Start)
		end := ipToUint(v.End)
		// 如果目标IP地址在起始和结束地址之间，则返回nil
		if compareUint(target, start) >= 0 && compareUint(target, end) <= 0 {
			return nil
		}
	}
	// 如果目标IP地址不在限流器中的IP地址范围内，则返回错误
	return web.ErrLoginLimiter
}

// 将 IP 地址转换为无符号整数切片
func ipToUint(ip string) []uint8 {
	parsedIP := net.ParseIP(ip)
	if parsedIP == nil || parsedIP.To4() == nil {
		return nil
	}
	return []uint8(parsedIP.To4())
}

// 比较两个无符号整数切片的大小
func compareUint(a, b []uint8) int {
	for i := 0; i < len(a); i++ {
		if a[i] < b[i] {
			return -1
		}
		if a[i] > b[i] {
			return 1
		}
	}
	return 0
}

// Scan implements orm.Scaner.
func (l *Limiter) Scan(input any) error {
	return orm.JsonUnmarshal(input, l)
}

var _ orm.Scaner = (*Limiter)(nil)

// MarshalJSON implements json.Marshaler.
func (u UserGroup) MarshalJSON() ([]byte, error) {
	type alias UserGroup
	a := alias(u)
	a.IsLeaf = a.ChildCount <= 0
	return json.Marshal(a)
}

func (*UserGroup) TableName() string {
	return "user_groups"
}

func (ug *UserGroup) BeforeCreate(tx *gorm.DB) error {
	ug.CreatedAt = orm.Now()
	ug.UpdatedAt = orm.Now()
	return nil
}

func (ug *UserGroup) BeforeUpdate(tx *gorm.DB) error {
	ug.UpdatedAt = orm.Now()
	return nil
}

var _ json.Marshaler = (*UserGroup)(nil)

// GenerateFromPassword 生成密码 hash
func GenerateFromPassword(passwd string) ([]byte, error) {
	return bcrypt.GenerateFromPassword([]byte(passwd), 6)
}

// CompareHashAndPasswordsd 比较 hash 与密码是否匹配
func CompareHashAndPasswordsd(hash []byte, passwd string) error {
	return bcrypt.CompareHashAndPassword(hash, []byte(passwd))
}

type DelUserInput struct {
	UserName string `json:"-"`
	GroupID  int    `json:"-"`
}
