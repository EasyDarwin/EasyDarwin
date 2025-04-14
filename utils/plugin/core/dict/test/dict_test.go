package test

import (
	"encoding/json"
	"fmt"
	"log/slog"
	"os"
	"strings"
	"testing"
	"time"

	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/pkg/web"
	"easydarwin/lnton/plugin/core/dict"
	"easydarwin/lnton/plugin/core/dict/store/dictdb"
	"github.com/glebarez/sqlite"
	"github.com/stretchr/testify/require"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

var (
	core dict.Core
	db   dictdb.DB
)

func getDialector(dsn string) (gorm.Dialector, bool) {
	if strings.HasPrefix(dsn, "postgres") {
		return postgres.New(postgres.Config{
			DriverName: "pgx",
			DSN:        dsn,
		}), false
	}
	return sqlite.Open(dsn), true
}

func TestMain(m *testing.M) {
	dsn := `./data.db` // os.Getenv("TEST_DSN")
	a, _ := getDialector(dsn)
	gdb, err := orm.New(true, a, orm.Config{
		MaxIdleConns:    10,
		MaxOpenConns:    10,
		ConnMaxLifetime: 1,
	}, orm.NewLogger(slog.Default(), true, time.Second))
	if err != nil {
		panic(err)
	}
	db = dictdb.NewDB(gdb)
	core = dict.NewCore(db, slog.Default())
	os.Exit(m.Run())
}

func TestAddDictType(t *testing.T) {
	t.Parallel()
	code := orm.GenerateRandomString(12)
	testCases := []struct {
		desc   string
		code   string
		name   string
		result bool
	}{
		{
			desc:   "code 不能为空",
			code:   "",
			name:   "123",
			result: false,
		},
		{
			desc:   "name 不能为空",
			code:   "1234",
			name:   "",
			result: false,
		},
		{
			desc:   "创建成功",
			code:   code,
			name:   "test",
			result: true,
		},
		{
			desc:   "code 不能重复",
			code:   code,
			name:   "test2",
			result: false,
		},
	}
	deleteIDs := make([]string, 0, 5)
	for _, tC := range testCases {
		t.Run(tC.desc, func(t *testing.T) {
			out, err := core.AddDictType(dict.AddDictTypeInput{
				Code: tC.code,
				Name: tC.name,
			})
			require.EqualValues(t, tC.result, err == nil)
			if out != nil {
				deleteIDs = append(deleteIDs, out.ID)
				require.EqualValues(t, tC.code, out.Code)
				require.EqualValues(t, tC.name, out.Name)
			}
		})
	}

	for _, v := range deleteIDs {
		require.NoError(t, core.DeleteDictType(v))
	}
}

func TestEditDictType(t *testing.T) {
	t.Parallel()
	v, err := core.AddDictType(dict.AddDictTypeInput{
		Code: orm.GenerateRandomString(12),
		Name: "test",
	})
	require.NoError(t, err)
	defer core.DeleteDictType(v.ID)

	testCases := []struct {
		desc   string
		name   string
		result bool
	}{
		{
			desc:   "name 不能为空",
			name:   "",
			result: false,
		},
		{
			desc:   "成功案例",
			name:   "test2",
			result: true,
		},
	}
	for _, tC := range testCases {
		t.Run(tC.desc, func(t *testing.T) {
			err := core.EditDictType(v.ID, tC.name)
			require.EqualValues(t, tC.result, err == nil, err)
			if err == nil {
				var dic dict.DictType
				require.NoError(t, db.GetDictTypeByID(&dic, v.ID))
				require.EqualValues(t, tC.name, dic.Name)
			}
		})
	}
}

func TestCreateDictData(t *testing.T) {
	typ, err := core.AddDictType(dict.AddDictTypeInput{
		Code: orm.GenerateRandomString(12),
		Name: "test",
	})
	require.NoError(t, err)
	defer func() {
		err := core.DeleteDictType(typ.ID)
		require.NoError(t, err, err)
	}()
	testCases := []struct {
		desc    string
		code    string
		label   string
		value   string
		sort    int
		enabled bool
		remark  string
		flag    string
		result  bool
	}{
		{
			desc:   "code 不存在",
			code:   orm.GenerateRandomString(12),
			label:  "test",
			value:  "1",
			result: false,
		},
		{
			desc:   "成功案例",
			code:   typ.Code,
			label:  "test",
			value:  "1",
			sort:   5,
			remark: "测试数据",
			flag:   "cn",
			result: true,
		},
		{
			desc:   "同类型的 label 不能重复",
			code:   typ.Code,
			label:  "test",
			value:  "121",
			sort:   5,
			remark: "测试数据",
			flag:   "cn",
			result: false,
		},
		{
			desc:   "同类型的 value 不能重复",
			code:   typ.Code,
			label:  "test121",
			value:  "1",
			sort:   5,
			remark: "测试数据",
			flag:   "cn",
			result: false,
		},
		{
			desc:   "同类型不重复的成功案例",
			code:   typ.Code,
			label:  "t12321",
			value:  "1123",
			sort:   5,
			remark: "测试数据",
			flag:   "cn",
			result: true,
		},
	}
	for _, tC := range testCases {
		t.Run(tC.desc, func(t *testing.T) {
			out, err := core.AddDictData(dict.CreateDictDataInput{
				Code:    tC.code,
				Label:   tC.label,
				Value:   tC.value,
				Sort:    tC.sort,
				Enabled: tC.enabled,
				Remark:  tC.remark,
				Flag:    tC.flag,
			})
			require.EqualValues(t, tC.result, err == nil)
			if out != nil {
				require.EqualValues(t, out.Code, typ.Code)
			}
		})
	}

	t.Run("查询字典列表", func(t *testing.T) {
		out, total, err := core.FindDictData(dict.FindDictDataInput{
			Code: typ.Code,
			Flag: "cn",
		})
		require.NoError(t, err)
		require.EqualValues(t, 2, total)
		require.EqualValues(t, 2, len(out))
	})
}

func TestEditDictData(t *testing.T) {
	t.Parallel()
	typ, err := core.AddDictType(dict.AddDictTypeInput{
		Code: orm.GenerateRandomString(12),
		Name: "test",
	})
	require.NoError(t, err)
	defer func() {
		err := core.DeleteDictType(typ.ID)
		require.NoError(t, err, err)
	}()
	v, err := core.AddDictData(dict.CreateDictDataInput{
		Code:    typ.Code,
		Label:   "123asd123",
		Value:   "!23123",
		Sort:    2,
		Enabled: true,
		Remark:  "asdasd",
		Flag:    "",
	})
	require.NoError(t, err)

	testCases := []struct {
		desc    string
		code    string
		label   string
		value   string
		sort    int
		enabled bool
		remark  string
		flag    string
		result  bool
	}{
		{
			desc:   "code 不存在",
			code:   orm.GenerateRandomString(12),
			label:  "test",
			value:  "1",
			result: false,
		},
		{
			desc:   "成功案例",
			code:   typ.Code,
			label:  "test",
			value:  "1",
			sort:   5,
			remark: "测试数据",
			flag:   "cn",
			result: true,
		},
	}
	for _, tC := range testCases {
		t.Run(tC.desc, func(t *testing.T) {
			out, err := core.EditDictData(dict.EditDictDataInput{
				Code:    tC.code,
				Label:   tC.label,
				Value:   tC.value,
				Sort:    tC.sort,
				Enabled: tC.enabled,
				Remark:  tC.remark,
				Flag:    tC.flag,
			}, v.ID)
			require.EqualValues(t, tC.result, err == nil)
			if out != nil {
				require.EqualValues(t, out.Code, typ.Code)
				require.EqualValues(t, out.Label, tC.label)
				require.EqualValues(t, out.Value, tC.value)
			}
		})
	}
}

func TestMarshal(t *testing.T) {
	a := dict.DictType{CreatedAt: orm.Now()}
	b, _ := json.Marshal(a)

	var c dict.DictType
	if err := json.Unmarshal(b, &c); err != nil {
		panic(err)
	}
	fmt.Println(c.CreatedAt)
}

func TestFindDict(t *testing.T) {
	{
		data, _, err := core.FindDictData(dict.FindDictDataInput{})
		if err != nil {
			panic(err)
		}
		file, _ := os.OpenFile("/Users/xugo/Desktop/xufan/project/cvs/internal/web/api/dict_data.json", os.O_TRUNC|os.O_CREATE|os.O_WRONLY, os.ModePerm)
		defer file.Close()

		if err := json.NewEncoder(file).Encode(data); err != nil {
			panic(err)
		}
	}

	{
		data, _, err := core.FindDictType(dict.FindDictTypeInput{PagerFilter: web.PagerFilter{
			Page: 1,
			Size: 1000,
		}})
		if err != nil {
			panic(err)
		}

		file, _ := os.OpenFile("/Users/xugo/Desktop/xufan/project/cvs/internal/web/api/dict_type.json", os.O_TRUNC|os.O_CREATE|os.O_WRONLY, os.ModePerm)
		defer file.Close()

		if err := json.NewEncoder(file).Encode(data); err != nil {
			panic(err)
		}
	}
}
