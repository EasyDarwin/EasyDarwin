package rotatelogs

func (h HandlerFunc) Handle(e Event) {
	h(e)
}

func (e *FileRotatedEvent) Type() EventType {
	return FileRotatedEventType
}

func (e *FileRotatedEvent) PreviousFile() string {
	return e.prev
}

func (e *FileRotatedEvent) CurrentFile() string {
	return e.current
}
