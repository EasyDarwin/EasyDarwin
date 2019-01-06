package models

type Stream struct {
	URL               string `gorm:"type:varchar(256);primary_key;unique"`
	CustomPath        string `gorm:"type:varchar(256)"`
	IdleTimeout       int
	HeartbeatInterval int
}
