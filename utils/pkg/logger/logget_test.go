package logger

import "testing"

func TestSlog(t *testing.T) {
	log, _ := SetupSlog(Config{
		Debug: false,
	})
	log.Info("Hello World")
}
