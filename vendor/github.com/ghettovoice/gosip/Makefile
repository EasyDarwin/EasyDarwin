VERSION=$(shell git describe --tags)
LDFLAGS=-ldflags "-X gosip.Version=${VERSION}"
GOFLAGS=

install:
	#go get -v github.com/wadey/gocovmerge
	go get -v -t ./...
	#go install -mod=mod github.com/onsi/ginkgo/...

test:
	#ginkgo -r --trace --race --compilers=2 $(GOFLAGS)
	go test -race ./...

test-%:
	ginkgo -r --trace --race --compilers=2 $(GOFLAGS) ./$*

test-watch:
	ginkgo watch -r --trace --race $(GOFLAGS)

test-watch-%:
	ginkgo watch -r --trace --race $(GOFLAGS) ./$*

test-linux:
	docker run -it --rm \
			-v `pwd`:/go/src/github.com/ghettovoice/gosip \
			-v ~/.ssh:/root/.ssh \
			-w /go/src/github.com/ghettovoice/gosip \
			golang:latest \
			make install && make test

cover-report: cover-merge
	go tool cover -html=./gosip.full.coverprofile

cover-merge:
	gocovmerge \
		./gosip.coverprofile \
		./sip/sip.coverprofile \
		./sip/parser/parser.coverprofile \
		./timing/timing.coverprofile \
		./transaction/transaction.coverprofile \
		./transport/transport.coverprofile \
	> ./gosip.full.coverprofile
