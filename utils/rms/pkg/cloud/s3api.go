package cloud

type ConnConfig struct {
	AccessKey  string
	SecretKey  string
	EndPoint   string
	DisableSSL bool
	Region     string
	Bucket     string // TODO: 待实现
}

func NewConfig(accessKey, secretKey, endPoint string, disableSSL bool, region string, s3ForcePathStyle bool) *ConnConfig {
	return &ConnConfig{
		AccessKey:  accessKey,
		SecretKey:  secretKey,
		EndPoint:   endPoint,
		DisableSSL: disableSSL,
		Region:     region,
	}
}
