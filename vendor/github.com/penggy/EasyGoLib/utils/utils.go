package utils

import (
	"bytes"
	"crypto/md5"
	"encoding/gob"
	"encoding/hex"
	"fmt"
	"log"
	"net"
	"os"
	"os/exec"
	"os/user"
	"path/filepath"
	"runtime"
	"strings"
	"sync"
	"time"

	"github.com/eiannone/keyboard"
	"github.com/teris-io/shortid"

	"github.com/go-ini/ini"
)

func LocalIP() string {
	ip := ""
	if addrs, err := net.InterfaceAddrs(); err == nil {
		for _, addr := range addrs {
			if ipnet, ok := addr.(*net.IPNet); ok && !ipnet.IP.IsLoopback() && !ipnet.IP.IsMulticast() && !ipnet.IP.IsLinkLocalUnicast() && !ipnet.IP.IsLinkLocalMulticast() && ipnet.IP.To4() != nil {
				ip = ipnet.IP.String()
			}
		}
	}
	return ip
}

func MD5(str string) string {
	encoder := md5.New()
	encoder.Write([]byte(str))
	return hex.EncodeToString(encoder.Sum(nil))
}

func CWD() string {
	path, err := os.Executable()
	if err != nil {
		return ""
	}
	return filepath.Dir(path)
}

var workInDirLock sync.Mutex

func WorkInDir(f func(), dir string) {
	wd, _ := os.Getwd()
	workInDirLock.Lock()
	defer workInDirLock.Unlock()
	os.Chdir(dir)
	defer os.Chdir(wd)
	f()
}

func EXEName() string {
	path, err := os.Executable()
	if err != nil {
		return ""
	}
	return strings.TrimSuffix(filepath.Base(path), filepath.Ext(path))
}

func HomeDir() string {
	u, err := user.Current()
	if err != nil {
		return ""
	}
	return u.HomeDir
}

func LogDir() string {
	dir := filepath.Join(CWD(), "logs")
	EnsureDir(dir)
	return dir
}

func ErrorLogFilename() string {
	return filepath.Join(LogDir(), fmt.Sprintf("%s-error.log", strings.ToLower(EXEName())))
}

func DataDir() string {
	dir := CWD()
	_dir := Conf().Section("").Key("data_dir").Value()
	if _dir != "" {
		dir = _dir
	}
	dir = ExpandHomeDir(dir)
	EnsureDir(dir)
	return dir
}

func ConfFile() string {
	if Exist(ConfFileDev()) {
		return ConfFileDev()
	}
	return filepath.Join(CWD(), strings.ToLower(EXEName())+".ini")
}

func ConfFileDev() string {
	return filepath.Join(CWD(), strings.ToLower(EXEName())+".dev.ini")
}

func DBFile() string {
	if Exist(DBFileDev()) {
		return DBFileDev()
	}
	return filepath.Join(CWD(), strings.ToLower(EXEName()+".db"))
}

func DBFileDev() string {
	return filepath.Join(CWD(), strings.ToLower(EXEName())+".dev.db")
}

var conf *ini.File

func Conf() *ini.File {
	if conf != nil {
		return conf
	}
	if _conf, err := ini.InsensitiveLoad(ConfFile()); err != nil {
		_conf, _ = ini.LoadSources(ini.LoadOptions{Insensitive: true}, []byte(""))
		conf = _conf
	} else {
		conf = _conf
	}
	return conf
}

func ReloadConf() *ini.File {
	if _conf, err := ini.InsensitiveLoad(ConfFile()); err != nil {
		_conf, _ = ini.LoadSources(ini.LoadOptions{Insensitive: true}, []byte(""))
		conf = _conf
	} else {
		conf = _conf
	}
	return conf
}

func SaveToConf(section string, kvmap map[string]string) error {
	var _conf *ini.File
	var err error
	if _conf, err = ini.InsensitiveLoad(ConfFile()); err != nil {
		if _conf, err = ini.LoadSources(ini.LoadOptions{Insensitive: true}, []byte("")); err != nil {
			return err
		}
	}
	sec := _conf.Section(section)
	for k, v := range kvmap {
		sec.Key(k).SetValue(v)
	}
	_conf.SaveTo(ConfFile())
	conf = _conf
	return nil
}

func ExpandHomeDir(path string) string {
	if len(path) == 0 {
		return path
	}
	if path[0] != '~' {
		return path
	}
	if len(path) > 1 && path[1] != '/' && path[1] != '\\' {
		return path
	}
	return filepath.Join(HomeDir(), path[1:])
}

func EnsureDir(dir string) (err error) {
	if _, err = os.Stat(dir); os.IsNotExist(err) {
		err = os.MkdirAll(dir, 0755)
		if err != nil {
			return
		}
	}
	return
}

func Exist(path string) bool {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		return false
	}
	return true
}

func DeepCopy(dst, src interface{}) error {
	var buf bytes.Buffer
	if err := gob.NewEncoder(&buf).Encode(src); err != nil {
		return err
	}
	return gob.NewDecoder(bytes.NewBuffer(buf.Bytes())).Decode(dst)
}

func Open(url string) error {
	var cmd string
	var args []string

	switch runtime.GOOS {
	case "windows":
		cmd = "cmd"
		args = []string{"/c", "start"}
	case "darwin":
		cmd = "open"
	default: // "linux", "freebsd", "openbsd", "netbsd"
		cmd = "xdg-open"
	}
	args = append(args, url)
	return exec.Command(cmd, args...).Start()
}

func ShortID() string {
	return shortid.MustGenerate()
}

func PauseExit() {
	log.Println("Press any to exit")
	keyboard.GetSingleKey()
	os.Exit(0)
}

func PauseGo(msg ...interface{}) {
	log.Println(msg...)
	keyboard.GetSingleKey()
}

func IsPortInUse(port int) bool {
	if conn, err := net.DialTimeout("tcp", net.JoinHostPort("", fmt.Sprintf("%d", port)), 3*time.Second); err == nil {
		conn.Close()
		return true
	}
	return false
}

func init() {
	gob.Register(map[string]interface{}{})
	gob.Register(StringArray(""))
	ini.PrettyFormat = false
}
