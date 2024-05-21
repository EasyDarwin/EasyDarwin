# 使用 Go 语言官方基础镜像
FROM golang:latest

# 设置工作目录
WORKDIR /app

# 复制所有文件到工作目录
COPY . .

# 初始化 Go module
RUN go mod init

# 构建应用
RUN go build -o easydarwin .

# 暴露所需的端口
EXPOSE 554
EXPOSE 10008

# 启动应用
CMD ["./easydarwin"]
