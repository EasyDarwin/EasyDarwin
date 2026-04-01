.PHONY: bench realclean cover viewcover test lint

bench:
	go test -tags bench -benchmem -bench .
	@git checkout go.mod 
	@rm go.sum

realclean:
	rm coverage.out

test:
	go test -v -race ./...

cover:
ifeq ($(strip $(STRFTIME_TAGS)),)
	go test -v -race -coverpkg=./... -coverprofile=coverage.out ./...
else
	go test -v -tags $(STRFTIME_TAGS) -race -coverpkg=./... -coverprofile=coverage.out ./...
endif

viewcover:
	go tool cover -html=coverage.out

lint:
	golangci-lint run ./...

imports:
	goimports -w ./

