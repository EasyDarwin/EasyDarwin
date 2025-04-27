package video

// Storer 依赖反转的数据持久化接口
type Storer interface {
}

// Core 业务对象
type Core struct {
	Storer Storer
}

// NewCore 创建业务对象
func NewCore(store Storer) *Core {
	return &Core{
		Storer: store,
	}
}
