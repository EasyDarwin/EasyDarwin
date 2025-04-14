package broadcast

import (
	"net"
	"net/http"
	"net/http/httputil"

	"github.com/google/uuid"
)

const broadcastPort = 49000

var broadcastIP = net.ParseIP("239.255.255.250")

const xRequestID = "X-Request-ID"

func request(req *http.Request) (string, []byte, error) {
	callID := uuid.New().String()
	req.Header.Set("X-Request-ID", callID)
	b, err := httputil.DumpRequest(req, true)
	return callID, b, err
}
