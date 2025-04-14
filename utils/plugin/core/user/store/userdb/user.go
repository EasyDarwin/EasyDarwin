package userdb

import (
	"context"
	"strconv"
	"time"

	"easydarwin/utils/pkg/orm"
	"easydarwin/utils/plugin/core/user"
	"gorm.io/gorm"
	"gorm.io/gorm/clause"
)

var _ user.Storer = DB{}

type DB struct {
	db *gorm.DB
	orm.Engine
	user orm.Type[user.User]
}

// // FirstOrCreate implements user.Storer.
// // Subtle: this method shadows the method (Engine).FirstOrCreate of DB.Engine.
// func (d DB) FirstOrCreate(any) (bool, error) {
// 	panic("unimplemented")
// }

// // InsertOne implements user.Storer.
// // Subtle: this method shadows the method (Engine).InsertOne of DB.Engine.
// func (d DB) InsertOne(orm.Tabler) error {
// 	panic("unimplemented")
// }

// // NextSeq implements user.Storer.
// // Subtle: this method shadows the method (Engine).NextSeq of DB.Engine.
// func (d DB) NextSeq(string) (nextID int, err error) {
// 	panic("unimplemented")
// }

// // UpdateOne implements user.Storer.
// // Subtle: this method shadows the method (Engine).UpdateOne of DB.Engine.
// func (d DB) UpdateOne(model orm.Tabler, id int, data map[string]any) error {
// 	panic("unimplemented")
// }

func NewDB(db *gorm.DB) *DB {
	return &DB{
		db:     db,
		Engine: orm.NewEngine(db),
		user:   orm.NewType[user.User](db),
	}
}

func (d DB) AutoMigrate(ok bool) *DB {
	if !ok {
		return &d
	}
	if err := d.db.AutoMigrate(
		new(user.User),
		new(user.UserGroup),
		new(user.Vcode),
	); err != nil {
		panic(err)
	}
	return &d
}

func (d DB) GetDB() *gorm.DB {
	return d.db
}

func (d DB) GetGroupByID(bean *user.UserGroup, id int) error {
	return d.db.Model(new(user.UserGroup)).Where("id=?", id).First(bean).Error
}

func (d DB) UpdateUser2(ctx context.Context, model *user.User, id int, fn func(*user.User) error) error {
	return d.user.Edit(ctx, model, fn, orm.Where("id=?", id))
}

func (d DB) GetUserByUserName(u *user.User, userName string) error {
	return d.db.Model(&user.User{}).Where(`user_name=?`, userName).First(u).Error
}

func (d DB) CountGroupByPID(pid int) (int64, error) {
	const sql = `SELECT COALESCE(SUM(user_count),0) FROM user_groups WHERE tree @> ARRAY[?::bigint]`
	var count int64
	err := d.db.Raw(sql, pid).Count(&count).Error
	return count, err
}

// 根据PID删除用户组
func (d DB) DeleteGroupByPID(ids *[]int, id int, pid int) error {
	// 开启事务
	return d.db.Transaction(func(tx *gorm.DB) error {
		// 创建一个返回id的查询
		db := tx.Clauses(clause.Returning{Columns: []clause.Column{{Name: "id"}}})
		// 删除用户组
		if err := db.Model(&user.UserGroup{}).Where(`(tree @> ARRAY[?::bigint]) AND user_count=0`, id).
			Delete(ids).Error; err != nil {
			return err
		}
		// 更新用户组子节点数量
		return tx.Model(&user.UserGroup{}).Where("id=?", pid).Update("child_count", gorm.Expr("child_count - ?", 1)).Error
	})
}

// InsertGroup 函数用于向数据库中插入一个用户组
func (d DB) InsertGroup(v *user.UserGroup) error {
	// 开始一个事务
	return d.db.Transaction(func(tx *gorm.DB) error {
		// 在事务中创建一个用户组，并返回插入后的id和sort
		if err := tx.Clauses(clause.Returning{Columns: []clause.Column{{Name: "id"}, {Name: "sort"}}}).
			Omit("sort").Create(v).Error; err != nil {
			return err
		}
		// 更新父级用户组的child_count字段，加1
		return tx.Model(&user.UserGroup{}).Where("id=?", v.PID).Update("child_count", gorm.Expr("child_count + ?", 1)).Error
	})
}

func (d DB) FindGroupsByName(bs *[]*user.UserGroup, name string, limit, offset int) (int64, error) {
	const sql = `
	WITH RECURSIVE tree AS (
		SELECT
			*
		FROM
			user_groups
		WHERE
			"name" LIKE ?
		LIMIT ? OFFSET ?
	),
	tab AS (
		SELECT
			*
		FROM
			tree
		UNION
		SELECT
			ug.*
		FROM
			user_groups ug
			INNER JOIN tree t ON ug. "id" = t. "pid"
	)
	SELECT
		*
	FROM
		tab t
	ORDER BY
		sort ASC;
	`

	// var where string
	var count int64
	{
		db := d.db.Model(&user.UserGroup{}).Where("name LIKE ?", "%"+name+"%")

		if err := db.Count(&count).Error; err != nil {
			return count, err
		}
		if count == 0 {
			return count, nil
		}
	}
	return count, d.db.Raw(sql, "%"+name+"%", limit, offset).Find(bs).Error
}

// FindGroups 函数用于查找用户组
func (d DB) FindGroups(bs *[]*user.UserGroup, pid, limit, offset int) (int64, error) {
	// 定义变量count，用于存储用户组数量
	var count int64
	// 定义db变量，用于查询用户组
	db := d.db.Model(&user.UserGroup{}).Where("pid=?", pid)
	// 普通用户组只能查看自己当前域，超管不受权限
	// 查询用户组数量
	if err := db.Count(&count).Error; err != nil {
		// 如果查询出错，返回错误
		return 0, err
	}
	// 如果用户组数量为0，返回nil
	if count == 0 {
		return 0, nil
	}
	// 查询用户组，按照sort字段升序排列，限制返回数量，偏移量
	err := db.Order("sort ASC").Limit(limit).Offset(offset).Find(bs).Error
	// 返回用户组数量和错误
	return count, err
}

// 根据排序更新用户组
func (d DB) UpdateGroupBySort(srcID, dstID int, srcSort, dstSort int) error {
	// 开启事务
	return d.db.Transaction(func(tx *gorm.DB) error {
		// 更新源用户组的排序
		if err := tx.Model(&user.UserGroup{}).Where("id=?", srcID).UpdateColumn("sort", dstSort).Error; err != nil {
			return err
		}
		// 更新目标用户组的排序
		return tx.Model(&user.UserGroup{}).Where("id=?", dstID).UpdateColumn("sort", srcSort).Error
	})
}

// 根据ID获取验证码
func (d DB) GetCaptchaByID(v *user.Vcode, id int) error {
	return d.db.Where("id=?", id).First(v).Error
}

// 根据用户名获取用户
func (d DB) GetUserByUsername(v *user.User, username string) error {
	return d.db.Where("username=?", username).First(v).Error
}

// 根据ID获取用户
func (d DB) GetUserByID(v *user.User, id int) error {
	return d.db.Where("id=?", id).First(v).Error
}

// 更新验证码已使用状态
func (d DB) UpdateVcodeUsed(id int, username string) error {
	return d.db.Model(&user.Vcode{}).Where("id=?", id).Updates(map[string]any{
		"used_at": time.Now(),
		"key":     username,
	}).Error
}

// 更新密码尝试次数
func (d DB) UpdatePasswordAttempts(id int, limitdAt int64) error {
	// 定义要更新的数据
	data := map[string]any{
		"password_attempts": gorm.Expr("password_attempts + ?", 1),
	}
	// 更新密码尝试次数
	if err := d.db.Model(&user.User{}).Where("id=?", id).Updates(data).Error; err != nil {
		return err
	}
	// 如果限制时间小于等于0，则返回nil
	if limitdAt <= 0 {
		return nil
	}
	// 定义更新最后登录信息的SQL语句
	const sql = `UPDATE users SET last_login_info = last_login_info || jsonb_build_object('limited_at', ?::int) WHERE id = ?`
	// 执行SQL语句
	return d.db.Exec(sql, limitdAt, id).Error
}

func (d DB) UpdateDevice(v *user.User, id string, fn func(*user.User)) error {
	return d.db.Transaction(func(tx *gorm.DB) error {
		if err := d.db.Model(&user.User{}).Clauses(clause.Locking{Strength: "UPDATE"}).Where("id=?", id).First(v).Error; err != nil {
			return err
		}
		fn(v)
		return d.db.Save(v).Error
	})
}

// UpdateUser 函数用于更新用户信息
func (d DB) UpdateUser(v *user.User, id int, fn func(user *user.User)) error {
	return d.db.Transaction(func(tx *gorm.DB) error {
		if err := tx.Model(&user.User{}).Clauses(clause.Locking{Strength: "UPDATE"}).Where("id=?", id).First(v).Error; err != nil {
			return err
		}
		fn(v)
		return tx.Save(v).Error
	})
}

func (d DB) UpdateUserByUserName(v *user.User, un string, fn func(user *user.User)) error {
	return d.db.Transaction(func(tx *gorm.DB) error {
		if err := tx.Model(&user.User{}).Clauses(clause.Locking{Strength: "UPDATE"}).Where("username=?", un).First(v).Error; err != nil {
			return err
		}
		fn(v)
		return tx.Save(v).Error
	})
}

// DeleteUser函数用于删除用户
func (d DB) DeleteUser(id, groupID int) error {
	// 获取当前时间
	now := time.Now()
	// 开始事务
	return d.db.Transaction(func(tx *gorm.DB) error {
		// 更新用户表中的用户名和删除时间
		if err := tx.Model(&user.User{}).Where("id=?", id).Updates(map[string]any{
			"username":   gorm.Expr("'del_'||username||'_'||? ", strconv.FormatInt(now.Unix(), 10)),
			"deleted_at": now,
		}).Error; err != nil {
			return err
		}
		// 更新用户组表中的用户数量
		return tx.Model(&user.UserGroup{}).Where("id=?", groupID).Update("user_count", gorm.Expr("user_count - ?", 1)).Error
	})
}

func (d DB) DelUserByUserName(un string, groupID int) error {
	// 获取当前时间
	now := time.Now()
	// 开始事务
	return d.db.Transaction(func(tx *gorm.DB) error {
		// 更新用户表中的用户名和删除时间
		if err := tx.Model(&user.User{}).Where("username =?", un).Updates(map[string]any{
			"username":   gorm.Expr("'del_'||username||'_'||? ", strconv.FormatInt(now.Unix(), 10)),
			"deleted_at": now,
		}).Error; err != nil {
			return err
		}
		// 更新用户组表中的用户数量
		return tx.Model(&user.UserGroup{}).Where("id=?", groupID).Update("user_count", gorm.Expr("user_count - ?", 1)).Error
	})
}

// CreateUser 函数用于创建用户
func (d DB) CreateUser(v *user.User) error {
	// 开始事务
	return d.db.Transaction(func(tx *gorm.DB) error {
		// 创建用户
		if err := tx.Create(v).Error; err != nil {
			return err
		}
		// 更新用户组中的用户数量
		return tx.Model(&user.UserGroup{}).Where("id=?", v.GroupID).
			UpdateColumn("user_count", gorm.Expr("user_count + ?", 1)).Error
	})
}

// 根据传入的参数，查询用户信息
func (d DB) FindUsers(v *[]*user.User, pUID int, in *user.FindUsersInput, level int) (int64, error) {
	// 创建一个数据库查询对象
	db := d.db.Model(&user.User{})
	//db.Where(" group_tree @> ARRAY[?::bigint]", groupID)
	//// 如果不是管理员，则查询该用户及其子用户
	//if !isAdmin {
	//	db = db.Where("  tree @> ARRAY[?::bigint] OR id=?  ", pUID, pUID)
	//}
	// 如果传入的用户名不为空，则查询包含该用户名的用户
	if in.Username != "" {
		db = db.Where("username like ?", "%"+in.Username+"%")
	}
	// 将传入的enabled参数转换为布尔值
	ena, err := strconv.ParseBool(in.Enabled)
	if err == nil {
		db = db.Where("enabled = ?", ena)
	}
	if level != 0 { // 用来查询比自己小的用户
		db = db.Where("level > ?", level)
	}
	if in.Level != 0 { // 用来查询指定等级的用户
		db = db.Where("level = ?", in.Level)
	}

	// 查询用户总数
	var count int64
	if err := db.Count(&count).Error; err != nil {
		return 0, err
	}
	// 如果查询结果为空，则返回nil
	if count <= 0 {
		return 0, nil
	}
	// 查询用户信息，并按照id降序排列，限制查询结果数量，并跳过指定数量的结果
	err = db.Order("id DESC").Limit(in.Limit()).Offset(in.Offset()).Find(v).Error
	// 返回查询结果总数和错误信息
	return count, err
}

// 根据传入的参数，查询数据库中符合条件的用户，并返回符合条件的用户总数和查询结果
func (d DB) FindApps(bs *[]*user.User, limit, offset int) (int64, error) {
	// 使用传入的数据库实例，查询用户表中类型为开发者的用户
	db := d.db.Model(new(user.User)).Where("type=?", user.UserTypeDeveloper)
	var total int64
	// 查询符合条件的用户总数
	if err := db.Count(&total).Error; err != nil {
		return 0, err
	}
	// 查询符合条件的用户，并按照id降序排列，限制返回的条数，并设置偏移量
	err := db.Limit(limit).Offset(offset).Order("id DESC").Find(bs).Error
	// 返回符合条件的用户总数和查询结果
	return total, err
}

// 根据id和type删除用户
func (d DB) DeleteApp(id int) error {
	// 在数据库中查找id和type匹配的用户
	return d.db.Where("id=? AND type=?", id, user.UserTypeDeveloper).Delete(new(user.User)).Error
}
