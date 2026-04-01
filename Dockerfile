FROM golang:alpine⁠ as builder

# Create app directory
RUN mkdir -p /home/app
WORKDIR /home/app
COPY . ./app

ENV GOPATH="/home/app:${GOPATH}"
# 安装 git
#RUN apk add git && git config --global http.sslVerify "false"
# && git config --global --add url."git@code.aliyun.com:".insteadOf "https://code.aliyun.com/"
#COPY .ssh /root/
#RUN chown 1000:1000 /root/.ssh/id_rsa
#go env -w GONOPROXY="code.aliyun.com,gopkg.in" &&  go env -w  GONOSUMDB="code.aliyun.com,gopkg.in" && go mod vendor &&
RUN cd app &&  go build -mod vendor -ldflags "-w -s" -o ../bin/app ./cmd/server/*.go && \
  chmod +x ../bin/app

FROM alpine:3.22.0

RUN apk add ffmpeg

ENV TZ=Asia/Shanghai

RUN apk --no-cache add ca-certificates \
	tzdata

WORKDIR /app

COPY --from=builder /home/app/bin .
COPY --from=builder /home/app/app/configs ./configs
COPY --from=builder /home/app/app/web ./web

LABEL Name=goweb Version=0.0.1

EXPOSE 8080

CMD [ "./app" ]
