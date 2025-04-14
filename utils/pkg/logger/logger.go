package logger

import (
	"io"
	"log/slog"
	"os"
	"path/filepath"
	"runtime/debug"
	"strings"
	"time"

	// "github.com/go-kratos/kratos/v2/log"
	rotatelogs "github.com/lestrrat-go/file-rotatelogs"
	"go.uber.org/zap"
	"go.uber.org/zap/exp/zapslog"
	"go.uber.org/zap/zapcore"
)

// Level 日志级别
var Level = zap.NewAtomicLevelAt(zap.InfoLevel)

// SetLevel 设置日志级别 debug/warn/error
func SetLevel(l string) {
	switch strings.ToLower(l) {
	case "debug":
		Level.SetLevel(zap.DebugLevel)
	case "warn":
		Level.SetLevel(zap.WarnLevel)
	case "error":
		Level.SetLevel(zap.ErrorLevel)
	default:
		Level.SetLevel(zap.InfoLevel)
	}
}

// NewJSONLogger 创建JSON日志
func NewJSONLogger(debug bool, w io.Writer) *zap.Logger {
	config := zap.NewProductionEncoderConfig()
	config.EncodeTime = zapcore.TimeEncoderOfLayout("2006-01-02 15:04:05.000")
	config.NameKey = ""
	mulitWriteSyncer := []zapcore.WriteSyncer{
		zapcore.AddSync(w),
	}
	if debug {
		mulitWriteSyncer = append(mulitWriteSyncer, zapcore.AddSync(os.Stdout))
	}
	// level = zap.ErrorLevel
	core := zapcore.NewSamplerWithOptions(zapcore.NewCore(
		zapcore.NewJSONEncoder(config),
		zapcore.NewMultiWriteSyncer(mulitWriteSyncer...),
		Level,
	), time.Second, 5, 5)
	return zap.New(core, zap.AddCaller())
}

func rotatelog(dir string, maxAge, duration time.Duration, size int64) *rotatelogs.RotateLogs {
	if maxAge <= 0 {
		maxAge = 7 * 24 * time.Hour
	}
	if duration <= 0 {
		duration = 12 * time.Hour
	}
	if size <= 0 {
		size = 10 * 1024 * 1024
	}
	r, _ := rotatelogs.New(
		filepath.Join(dir, "%Y%m%d_%H_%M_%S.log"),
		rotatelogs.WithMaxAge(maxAge),
		rotatelogs.WithRotationTime(duration),
		rotatelogs.WithRotationSize(size),
	)
	return r
}

// func TracingValue(key string, lo log.Valuer) slog.Attr {
// 	return slog.Attr{
// 		Key:   key,
// 		Value: slog.AnyValue(ValueFunc(lo)),
// 	}
// }

// type ValueFunc log.Valuer

// // MarshalJSON implements json.Marshaler.
// func (v ValueFunc) MarshalJSON() ([]byte, error) {
// 	data := log.Valuer(v)(context.TODO())
// 	return json.Marshal(data)
// }

// func (vf ValueFunc) Value(ctx context.Context) interface{} {
// 	return log.Valuer(vf)(ctx)
// }

// var _ json.Marshaler = (*ValueFunc)(nil)

// var _ log.Logger = (*Logger)(nil)

// type KLogger struct {
// 	log *slog.Logger
// }

// func NewLogger(slog *slog.Logger) log.Logger {
// 	return &Logger{slog}
// }

// func (l *KLogger) Log(level log.Level, keyvals ...interface{}) error {
// 	keylen := len(keyvals)
// 	if keylen == 0 || keylen%2 != 0 {
// 		l.log.Warn(fmt.Sprint("Keyvalues must appear in pairs: ", keyvals))
// 		return nil
// 	}

// 	switch level {
// 	case log.LevelDebug:
// 		l.log.Debug("", keyvals...)
// 	case log.LevelInfo:
// 		l.log.Info("", keyvals...)
// 	case log.LevelWarn:
// 		l.log.Warn("", keyvals...)
// 	case log.LevelError:
// 		l.log.Error("", keyvals...)
// 	case log.LevelFatal:
// 		l.log.Error("", keyvals...)
// 	}
// 	return nil
// }

// Config ....
type Config struct {
	Dir          string
	ID           string
	Name         string
	Version      string
	Debug        bool
	MaxAge       time.Duration
	RotationTime time.Duration
	RotationSize int64  // 单位字节
	Level        string // debug/info/warn/error
}

// func getLevel(level string) zapcore.Level {
// 	switch strings.ToLower(level) {
// 	case "debug":
// 		return zap.DebugLevel
// 	case "info":
// 		return zapcore.InfoLevel
// 	case "warn":
// 		return zap.WarnLevel
// 	case "error":
// 		return zap.ErrorLevel
// 	default:
// 		return zap.InfoLevel
// 	}
// }

// NewDefaultConfig 创建默认配置
func NewDefaultConfig() Config {
	return Config{
		ID:           "test",
		Dir:          "./logs",
		Version:      "0.0.1",
		Debug:        true,
		MaxAge:       7 * 24 * time.Hour,
		RotationTime: 1 * time.Hour,
		RotationSize: 1 * 1024 * 1024,
	}
}

// SetupSlog 初始化日志
func SetupSlog(cfg Config) (*slog.Logger, func()) {
	SetLevel(cfg.Level)
	r := rotatelog(cfg.Dir, cfg.MaxAge, cfg.RotationTime, cfg.RotationSize)
	log := slog.New(
		zapslog.NewHandler(
			NewJSONLogger(cfg.Debug, r).Core(),
			zapslog.WithCaller(cfg.Debug),
		),
	)
	if cfg.ID != "" {
		log = log.With("serviceID", cfg.ID)
	}
	if cfg.Version != "" {
		log = log.With("serviceVersion", cfg.Version)
	}
	slog.SetDefault(log)

	crashFile, err := os.OpenFile(filepath.Join(cfg.Dir, "crash.log"), os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0o600)
	if err == nil {
		_ = SetCrashOutput(crashFile)
	}
	return log, func() {
		crashFile.Close()
	}
}

// SetCrashOutput recover panic
func SetCrashOutput(f *os.File) error {
	return debug.SetCrashOutput(f, debug.CrashOptions{})
}
