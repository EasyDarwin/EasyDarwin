package machine

// 定义一个接口
type OsMachineInterface interface {
	GetMachine() Information
	GetBoardSerialNumber() (string, error)
	GetPlatformUUID() (string, error)
	GetCpuSerialNumber() (string, error)
}
