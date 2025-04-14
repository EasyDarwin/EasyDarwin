FROM alpine:latest

ENV TZ=Asia/Shanghai

RUN apk --no-cache add ca-certificates \
	tzdata

WORKDIR /app

ADD ./build/linux_amd64/bin ./

LABEL Name=goweb Version=0.0.1

EXPOSE 8080

CMD [ "./bin" ]