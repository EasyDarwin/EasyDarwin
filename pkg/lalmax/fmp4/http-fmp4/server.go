package httpfmp4

import (
	"github.com/gin-gonic/gin"
)

type HttpFmp4Server struct {
}

func NewHttpFmp4Server() *HttpFmp4Server {
	svr := &HttpFmp4Server{}

	return svr
}

func (s *HttpFmp4Server) HandleRequest(c *gin.Context) {
	streamid := c.Param("streamid")

	session := NewHttpFmp4Session(streamid)
	session.handleSession(c)
}
