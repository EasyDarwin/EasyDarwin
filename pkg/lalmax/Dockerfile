FROM golang:1.21.1
ENV GOPROXY=https://goproxy.cn,https://goproxy.io,direct
LABEL maintainer="Kevin Zang"

WORKDIR /code
COPY . .
RUN /bin/bash ./build.sh

EXPOSE 1935 8080 4433 5544 8083 8084 1290 30000-30100/udp 6001/udp 4888/udp

CMD export LD_LIBRARY_PATH=/usr/local/lib/ && ./run.sh