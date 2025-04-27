package efile

import (
	"bufio"
	"bytes"
	"easydarwin/internal/gutils/estring"
	"encoding/gob"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"math"
	"mime/multipart"
	"os"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strconv"
	"strings"
)

// GetDirListPath 获取目录下的所有文件夹路径
func GetDirListPath(dirpath string) []string {
	var dirList []string
	if Exisit(dirpath) {
		err := filepath.Walk(dirpath,
			func(path string, f os.FileInfo, err error) error {
				if f == nil {
					return err
				}
				if f.IsDir() {
					dirList = append(dirList, path)
					return nil
				}
				return nil
			})
		if err != nil {
			return []string{}
		}
	}
	return dirList
}

// 判断文件夹是否为空，如果读取错误默认为非空
func IsEmptyDir(dirPath string) bool {
	dir, err := ioutil.ReadDir(dirPath)
	if err != nil {
		return false
	}
	if len(dir) == 0 {
		return true
	} else {
		return false
	}
}

// GetDirFileListPath 获取目录下的所有文件路径
func GetDirFileListPath(dirpath string) []string {
	var fileList []string
	if Exisit(dirpath) {
		err := filepath.Walk(dirpath,
			func(path string, f os.FileInfo, err error) error {
				if f == nil {
					return err
				}
				if !f.IsDir() {
					fileList = append(fileList, path)
					return nil
				}
				return nil
			})
		if err != nil {
			return []string{}
		}
	}
	return fileList
}

func GetNotEmptyDirNames(dirpath string) []string {
	return getDirNames(dirpath, false, false)
}

// GetDirNames 获取目录所有的文件夹名称，降序排列
func GetDirNames(dirpath string) []string {
	return getDirNames(dirpath, false, true)
}

// GetDirNamesAsc 获取目录所有的文件夹名称,升序
func GetDirNamesAsc(dirpath string) []string {
	return getDirNames(dirpath, true, true)
}

func getDirNames(dirpath string, isAsc bool, canEmpty bool) []string {
	var dirList []string
	if Exisit(dirpath) {
		list, err := ioutil.ReadDir(dirpath)
		if err != nil {
			return []string{}
		}
		for i := 0; i < len(list); i++ {
			//如果是文件夹或是软连接
			if list[i].IsDir() || IsLink(list[i]) {
				subDirName := list[i].Name()
				if canEmpty {
					dirList = append(dirList, subDirName)
				} else {
					if !IsEmptyDir(filepath.Join(dirpath, list[i].Name())) {
						dirList = append(dirList, subDirName)
					}
				}
			}
		}
		if isAsc {
			sort.Sort(sort.StringSlice(dirList))
		} else {
			sort.Sort(sort.Reverse(sort.StringSlice(dirList)))
		}
	}
	return dirList
}

// GetFileNamesBySuffix 获取所有文件名称根据后缀
func GetFileNamesBySuffix(dirpath, suffix string, isAsc bool) []string {
	var fileList []string

	if Exisit(dirpath) {
		list, err := ioutil.ReadDir(dirpath)
		if err != nil {
			return []string{}
		}
		for i := 0; i < len(list); i++ {
			if !list[i].IsDir() && strings.HasSuffix(list[i].Name(), suffix) {
				fileList = append(fileList, list[i].Name())
			}
		}
		if isAsc {
			// 升序排列
			sort.Sort(sort.StringSlice(fileList))
		} else {
			//降序排列
			sort.Sort(sort.Reverse(sort.StringSlice(fileList)))
		}

	}
	return fileList
}

// GetFileNameByPrefix 获取文件名根据前缀
func GetFileNameByPrefix(dirpath string, prefix string) string {
	if Exisit(dirpath) {
		list, err := ioutil.ReadDir(dirpath)
		if err != nil {
			return ""
		}
		for i := 0; i < len(list); i++ {
			if !list[i].IsDir() && strings.HasPrefix(list[i].Name(), prefix) {
				return list[i].Name()
			}
		}
	}
	return ""
}

// GetDirNamesWithoutImportant 获取当前目录下所有文件夹，排除重要标记的
// 返回 true，代表有重要标记的文件夹
func GetDirNamesWithoutImportant(id, dirpath string) ([]string, bool) {
	return getDirNamesWithoutImportant(id, dirpath, false)
}

// GetDirNamesWithoutImportantAsc 获取当前目录下所有文件夹，排除重要标记的,升序排列
// 返回 true，代表有重要标记的文件夹
func GetDirNamesWithoutImportantAsc(id, dirpath string) ([]string, bool) {
	return getDirNamesWithoutImportant(id, dirpath, true)
}

func getDirNamesWithoutImportant(id, dirpath string, isAsc bool) ([]string, bool) {
	var dirList []string
	existImport := false

	if Exisit(dirpath) {
		list, err := ioutil.ReadDir(dirpath)
		if err != nil {
			return []string{}, existImport
		}
		for i := 0; i < len(list); i++ {
			dir := list[i]
			if dir.IsDir() {
				timePath := filepath.Join(dirpath, dir.Name())
				m3u8Path := filepath.Join(timePath, fmt.Sprintf(`%s_record.m3u8`, id))
				importantPath := filepath.Join(timePath, "important")
				if (Exisit(m3u8Path) && !Exisit(importantPath)) || !Exisit(m3u8Path) {
					dirList = append(dirList, dir.Name())
				}

				// 如果存在 import 文件夹
				if Exisit(importantPath) {
					existImport = true
				}
			}
		}
		if isAsc {
			sort.Sort(sort.StringSlice(dirList))
		} else {
			sort.Sort(sort.Reverse(sort.StringSlice(dirList)))
		}
	}
	return dirList, existImport
}

// GetDirNamesWithM3u8 获取当前目录下所有文件夹，必须包含m3u8
func GetDirNamesWithM3u8(id, dirpath string, m3u8Suffix string, isDesc bool) []string {
	var dirList []string

	if Exisit(dirpath) {
		list, err := ioutil.ReadDir(dirpath)
		if err != nil {
			return []string{}
		}
		for i := 0; i < len(list); i++ {
			dir := list[i]
			if dir.IsDir() {
				timePath := filepath.Join(dirpath, dir.Name())
				m3u8Path := filepath.Join(timePath, fmt.Sprintf(`%s%s`, id, m3u8Suffix))
				if Exisit(m3u8Path) {
					dirList = append(dirList, dir.Name())
				}
			}
		}
		if isDesc {
			//降序排列
			sort.Sort(sort.Reverse(sort.StringSlice(dirList)))
		}
	}
	return dirList
}

// GetDirCurrentLayerNames 获取当前目录第一层名称
func GetDirCurrentLayerNames(dirpath string) []string {
	var dirList []string
	if Exisit(dirpath) {
		list, err := ioutil.ReadDir(dirpath)
		if err != nil {
			return []string{}
		}
		for i := 0; i < len(list); i++ {
			dirList = append(dirList, list[i].Name())
		}
		sort.Sort(sort.Reverse(sort.StringSlice(dirList)))
	}
	return dirList
}

func EnsureDir(dir string) {
	defer func() {
		if p := recover(); p != nil {
			log.Println(p)
		}
	}()
	if _, err := os.Stat(dir); os.IsNotExist(err) {
		err = os.MkdirAll(dir, 0755)
		if err != nil {
			panic(err)
		}
	}
}

// EnsureSubPath 创建子文件夹
func EnsureSubPath(subPath string, realPath string) error {
	if realPath == "" {
		if err := EnsureDirWithError(subPath); err != nil {
			log.Println(err)
			return fmt.Errorf("子目录创建失败")
		}
	} else {
		//判断子文件夹是否存在，如果存在则返回失败
		if Exisit(subPath) {
			return fmt.Errorf("子目录已经存在")
		}
		if err := EnsureDirWithError(realPath); err != nil {
			log.Println(err)
			return fmt.Errorf("目标路径不正确")
		}
		if err := os.Symlink(realPath, subPath); err != nil {
			log.Println(err)
			return fmt.Errorf("创建连接失败,请以管理员权限启动")
		}
	}
	return nil
}

func EnsureDirWithError(dir string) error {
	if _, err := os.Stat(dir); os.IsNotExist(err) {
		err = os.MkdirAll(dir, 0755)
		if err != nil {
			return err
		}
	}
	return nil
}
func RemoveFile(file string) {
	defer func() {
		if p := recover(); p != nil {
			log.Println(p)
		}
	}()
	if Exisit(file) {
		log.Println("remove file use util : ", file)
		err := os.Remove(file)
		if err != nil {
			panic(err)
		}
	}
}

func RemoveAll(path string) {
	defer func() {
		if p := recover(); p != nil {
			log.Println(p)
		}
	}()
	if Exisit(path) {
		log.Println("remove path use util : ", path)
		err := os.RemoveAll(path)
		if err != nil {
			panic(err)
		}
	}
}

func EnsureFile(file string) {
	defer func() {
		if p := recover(); p != nil {
			log.Println(p)
		}
	}()
	if _, err := os.Stat(file); os.IsNotExist(err) {
		f, err := os.Create(file)
		if err != nil {
			panic(err)
		}
		defer f.Close()
	}
}

func Exisit(path string) bool {
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

func SaveUploadedFile(file *multipart.FileHeader, dst string) error {
	src, err := file.Open()
	if err != nil {
		return err
	}
	defer src.Close()

	out, err := os.Create(dst)
	if err != nil {
		return err
	}
	defer out.Close()

	io.Copy(out, src)
	return nil
}

// GetRealPath 判断传递的是否是相对路径，返回真实路径
func GetRealPath(path string) string {
	//如果是绝对路径
	if filepath.IsAbs(path) {
		return estring.FormatPath(path)
	}
	return estring.FormatPath(filepath.Join(CWD(), path))
}

func GetDSSRealPath(path string) string {
	//如果是绝对路径
	if strings.Contains(path, ":") || strings.HasPrefix(path, "/") {
		return estring.FormatPath(path)
	}
	return estring.FormatPath(filepath.Join(CWD(), "kernel", path))
}

func CWD() string {
	path, err := os.Executable()
	if err != nil {
		return ""
	}
	return filepath.Dir(path)
}

func GetEasyTrans() string {
	switch runtime.GOOS {
	case "windows":
		return filepath.Join(filepath.Join(CWD(), "mediaserver"), "trans/EasyTrans.exe")
	case "linux":
		path := filepath.Join(filepath.Join(CWD(), "mediaserver"), "trans/easytrans")
		os.Chmod(path, 0755)
		return path
	default:
	}

	return ""
}

func ReadFile(file string) string {
	defer func() {
		if p := recover(); p != nil {
			log.Println(p)
		}
	}()
	content := ""
	if Exisit(file) {
		c, err := ioutil.ReadFile(file)
		if err != nil {
			panic(err)
		}
		content = string(c)
	}
	return content
}

func WriteFile(file string, content string) {
	defer func() {
		if p := recover(); p != nil {
			log.Println(p)
		}
	}()
	err := ioutil.WriteFile(file, []byte(content), 0755)
	if err != nil {
		panic(err)
	}
}

// DirMove 移动文件夹
func DirMove(oldpath, newpath string) error {
	if Exisit(oldpath) {
		err := os.Rename(oldpath, newpath)
		if err != nil {
			if strings.Contains(err.Error(), "being used by another process") {
				return fmt.Errorf("移动失败，正在被使用中")
			}
			if strings.Contains(err.Error(), "move the file to a different disk drive") ||
				strings.Contains(err.Error(), "invalid cross-device link") {
				return fmt.Errorf("移动失败，无法从一个盘符移动到另一个盘符")
			}
			return err
		}
	}
	return nil
}

func Rename(oldpath, newpath string) (err error) {
	return DirMove(oldpath, newpath)
	// from, err := syscall.UTF16PtrFromString(oldpath)
	// if err != nil {
	// 	return err
	// }
	// to, err := syscall.UTF16PtrFromString(newpath)
	// if err != nil {
	// 	return err
	// }
	// return syscall.MoveFile(from, to)
}

// IsLink 判断是否是软连接
func IsLink(f os.FileInfo) bool {
	if strings.HasPrefix(f.Mode().String(), "L") ||
		strings.HasPrefix(f.Mode().String(), "l") {
		return true
	}
	return false
}

// IsLinkWithPath 根据路径判断是否是软连接
func IsLinkWithPath(path string) bool {
	f, err := os.Lstat(path)
	if err != nil {
		return false
	}
	return IsLink(f)
}

// 文件夹目录结构体
type Dir struct {
	Label    string `json:"label"`
	Path     string `json:"path"`
	Children []*Dir `json:"children"`
}

// 增加路径
// 子路径格式为 a/b/c/d/e/f
func (dir *Dir) AddChild(path string) {
	// 1. 查找是否有 a , 有 a 的话，路径变为 a.AddChild(b/c/d/e/f)
	// 2. 如果没有 a, 证明整个路径都要新建，全部创建
	if path == "" {
		return
	}

	// dirs := strings.Split(path, "/")
	// str := "adasds/dasd"
	// 获取 a
	seperator := "/"
	index := strings.Index(path, seperator)
	firstDir := ""
	restPath := ""
	if index == -1 {
		firstDir = path
	} else {
		firstDir = path[0:index]
		restPath = path[index+1:]
	}

	for _, child := range dir.Children {
		// 子目录查找到了 a，还有子目录，则创建在子目录中。没有剩余目录了，直接返回。
		if child.Label == firstDir {
			if restPath != "" {
				child.AddChild(restPath)
			}
			return
		}
	}

	newPath := ""
	if dir.Path == "" {
		newPath = firstDir
	} else {
		if dir.Path == seperator {
			newPath = seperator + firstDir
		} else {
			newPath = dir.Path + seperator + firstDir
		}
	}
	// 如果没有找到 a, 创建 a ，设置 a 为 dir 的孩子，用 a 创建剩余的所有路径
	newDir := &Dir{
		Label:    firstDir,
		Path:     newPath,
		Children: make([]*Dir, 0, 10),
	}
	dir.Children = append(dir.Children, newDir)
	newDir.AddChild(restPath)
}

// 判断是否有孩子
func (dir *Dir) HaveChildren() bool {
	return len(dir.Children) > 0
}

// 排序，有子目录在最下方，无子目录的在最上方
func (dir *Dir) Sort() {
	children := dir.Children
	length := len(dir.Children)

	for i := 0; i < length; i++ {
		if children[i].HaveChildren() {
			// 对孩子的孩子进行排序
			children[i].Sort()
			// 从后查找没有子目录的索引，找到后，同当前索引交换位置
			for j := length - 1; j >= 0; j-- {
				if !children[j].HaveChildren() {
					children[i], children[j] = children[j], children[i]
					break
				}
			}
		}
	}
}

// 文件夹目录结构体
type NDir struct {
	Label    string `json:"label"`
	Path     string `json:"path"`
	Children []NDir `json:"children"`
}

// 增加路径
// a/v/g
// 子路径格式为 a/b/c/d/e/f
func AddChild(oldDir NDir, path string) NDir {
	seperator := "/"
	pathStrs := strings.Split(path, seperator)

	// 构建新的目录结构
	/*for index, pathDir := range pathdirs {
		pathdirs[index] =
	}*/

	father := oldDir
	children := oldDir.Children
	find := false
	for index, pathStr := range pathStrs {

		for _, child := range children {
			// 如果找到了孩子，遍历下一层孩子
			if child.Label == pathStr {
				father = child
				children = child.Children
				find = true
				break
			}
		}

		if !find {
			restPath := pathStrs[index]
			for i := index + 1; i < len(pathStrs); i++ {
				restPath = restPath + seperator + pathStrs[i]
			}

			father.Children = append(father.Children, CreateNdir(father.Path, restPath))
			break
		}
	}

	return oldDir
}

func CreateNdir(root string, path string) NDir {
	seperator := "/"
	pathStrs := strings.Split(path, seperator)
	ndirs := make([]NDir, 0, 10)
	dirpath := root

	for _, pathStr := range pathStrs {
		dirpath = dirpath + seperator + pathStr
		newDir := NDir{
			Label:    pathStr,
			Path:     dirpath,
			Children: nil,
		}
		ndirs = append(ndirs, newDir)
	}

	deep := len(ndirs)
	for index := deep - 2; index >= 0; index-- {
		ndirs[index].Children = append(ndirs[index].Children, ndirs[index+1])
	}

	if deep > 0 {
		return ndirs[0]
	} else {
		return NDir{
			Label:    root,
			Path:     root,
			Children: nil,
		}
	}
}

// 判断 连续 三个71 中间间隔是否 187
func isTs(p *[]byte) bool {
	var (
		num int // 包含连续 71的个数
	)
	for index, value := range *p {
		if value != 71 {
			continue
		}
		lastindex := index
		for {
			ni := lastindex + 188
			if (len(*p) - 1) < ni {
				return false
			}
			if (*p)[ni] != 71 {
				break
			}
			num++
			if num >= 3 {
				return true
			}
			lastindex = ni
		}
	}
	return false
}

// 获取文件夹下的所有 TS 文件
// dirPath 文件路径
// isAsc true, 升序排列；false, 降序排列
func getAllTS(dirPath string, isAsc bool) (*[]string, error) {
	dir, err := os.Open(dirPath)
	if err != nil {
		return nil, err
	}
	defer dir.Close()

	dirInfo, err := dir.Stat()
	if err != nil {
		return nil, err
	}
	if !dirInfo.IsDir() {
		return nil, errors.New("input is not a dir")
	}

	fileNames := make([]string, 0)
	fileInfos, _ := dir.Readdir(0)
	for _, fileInfo := range fileInfos {
		// 如果不是文件夹，并且以 .ts 结尾则加入队列
		if !fileInfo.IsDir() && strings.HasSuffix(fileInfo.Name(), ".ts") {
			fileNames = append(fileNames, fileInfo.Name())
		}
	}

	if isAsc {
		// 以升序排列
		sort.Sort(sort.StringSlice(fileNames))
	} else {
		//降序排列
		sort.Sort(sort.Reverse(sort.StringSlice(fileNames)))
	}

	return &fileNames, nil
}

// 最多获取当天的ts文件
func GenerateM3U8ByTSS(dirPath string, strs *[]string, m3u8Name string, sDay string, id string) error {
	if !strings.HasSuffix(dirPath, "/") && !strings.HasSuffix(dirPath, "\\") {
		dirPath = dirPath + string(os.PathSeparator)
	}

	// 创建 video.m3u8 文件
	mfile, err := os.Create(dirPath + m3u8Name)
	if err != nil {
		return err
	}
	defer mfile.Close()

	w := bufio.NewWriter(mfile)
	fmt.Fprintln(w, "#EXTM3U")
	fmt.Fprintln(w, "#EXT-X-VERSION:3")
	fmt.Fprintln(w, "#EXT-X-MEDIA-SEQUENCE:0")
	timeStr := ""
	var timeAll float64
	var timeMax float64
	for _, tsname := range *strs {
		realTsPaths := strings.Split(tsname, sDay+"/")
		tsId := strings.Split(realTsPaths[1], "/")
		periodPath := filepath.Join(realTsPaths[0], sDay, tsId[0], fmt.Sprintf(`%s_record.m3u8`, id))
		timeStr = getTsTime(periodPath, tsId[1])
		time := strings.Split(strings.Split(timeStr, ":")[1], ",")[0]
		timeFloat, _ := strconv.ParseFloat(time, 64)
		if timeFloat > timeMax {
			timeMax = timeFloat
		}
		timeAll = timeAll + timeFloat
	}
	fmt.Fprintln(w, fmt.Sprintf("#EXT-X-TARGETDURATION:%s", fmt.Sprintf("%v", int(math.Ceil(timeMax)))))
	fmt.Fprintln(w, fmt.Sprintf("#EXT_X_TOTAL_DURATION:  %s", fmt.Sprintf("%v", timeAll)))

	for _, tsname := range *strs {
		realTsPaths := strings.Split(tsname, sDay+"/")
		tsId := strings.Split(realTsPaths[1], "/")
		periodPath := filepath.Join(realTsPaths[0], sDay, tsId[0], fmt.Sprintf(`%s_record.m3u8`, id))
		timeStr = getTsTime(periodPath, tsId[1])
		fmt.Fprintln(w, timeStr)
		fmt.Fprintln(w, realTsPaths[1])
	}

	fmt.Fprintln(w, "#EXT-X-ENDLIST")
	w.Flush()

	return nil
}

// ffmpeg获取ts的时间是不准确的
func getTsTime(path string, tsId string) string {
	data := ReadFile(path)
	reg, _ := regexp.Compile("#EXTINF:" + `.*,` + "\n" + tsId)
	tsTimeAndName := reg.FindString(data)
	reg_, _ := regexp.Compile("#EXTINF:" + `.*,`)
	tsTime := reg_.FindString(tsTimeAndName)
	return tsTime
}
