package user

import (
	"errors"

	"easydarwin/utils/pkg/web"

	"easydarwin/utils/pkg/orm"
	"github.com/lib/pq"
)

const MaxLevel = 12

type GroupStorer interface {
	GetGroupByID(*UserGroup, int) error                                               // 通过 id 查询用户组
	CountGroupByPID(pid int) (int64, error)                                           // 查询指定用户组下的用户总数(含所有子级)
	DeleteGroupByPID(ids *[]int, id int, pid int) error                               // 根据 id 删除区域
	InsertGroup(v *UserGroup) error                                                   // 插入用户组
	FindGroups(bs *[]*UserGroup, pid, limit, offset int) (int64, error)               // 查询用户组
	UpdateGroupBySort(srcID, dstID int, srcSort, dstSort int) error                   // 用户组排序
	NextSeq(orm.Tabler) (nextID int, err error)                                       // 查询下一个索引
	FindGroupsByName(bs *[]*UserGroup, name string, limit, offset int) (int64, error) // 查询用户名
}

// CreateGroup 创建一个用户组
func (c Core) CreateGroup(uid, pid int, name string) (*UserGroup, error) {
	return &UserGroup{}, nil
	// 检查名称是否合法
	// name, err := model.CheckName(name)
	// if err != nil {
	// 	return nil, err
	// }
	// // 检查上级资源是否存在
	// if pid <= 0 {
	// 	return nil, web.ErrBadRequest.Msg("上级资源不存在").Withf("group pid[%d]<=0", pid)
	// }
	// // 初始化用户组
	// var out UserGroup
	// out.Name = name
	// out.UID = uid
	// out.PID = pid
	// // 获取用户组的层级和树结构
	// out.Level, out.Tree, err = c.getLevelAndTree(pid)
	// if err != nil {
	// 	return nil, err
	// }
	// // 获取下一个序列号
	// id, err := c.Store.NextSeq(DefaultSequenceName)
	// if err != nil {
	// 	return nil, web.ErrDB.With(err.Error())
	// }
	// out.ID = id
	// out.Tree = append(out.Tree, int64(out.ID))
	// // 插入用户组
	// if err := c.Store.InsertGroup(&out); errors.Is(err, orm.ErrDuplicatedKey) {
	// 	return nil, web.ErrBadRequest.Msg("名称不能重复").With(err.Error())
	// } else if err != nil {
	// 	return nil, web.ErrDB.With(err.Error())
	// }
	// return &out, nil
}

// 根据pid获取层级和树
func (c Core) getLevelAndTree(pid int) (level int8, tree pq.Int64Array, err error) {
	// 如果pid小于等于0，则返回2，空数组，nil
	if pid <= 0 {
		return 2, pq.Int64Array{}, nil
	}
	// 定义一个UserGroup类型的变量pNode
	var pNode UserGroup
	// 根据pid获取UserGroup类型的pNode
	err2 := c.Store.GetGroupByID(&pNode, pid)
	// 如果获取失败，并且错误类型为orm.ErrRevordNotFound，则返回错误信息
	if errors.Is(err2, orm.ErrRevordNotFound) {
		err = web.ErrBadRequest.Msg("上级资源不存在").Withf(`err[%s] pid[%d]`, err2, pid)
		return
	}
	// 如果获取失败，则返回错误信息
	if err2 != nil {
		err = web.ErrDB.With(err2.Error())
		return
	}
	// 如果pNode的层级大于等于最大层级，则返回错误信息
	if pNode.Level >= MaxLevel {
		err = web.ErrUsedLogic.Msg("不允许创建，超出最大层级").Withf("level >= %d", MaxLevel)
		return
	}
	// 返回pNode的层级加1，pNode的树，nil
	return pNode.Level + 1, pNode.Tree, nil
}
