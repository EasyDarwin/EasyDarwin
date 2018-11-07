// +build !windows

package keyboard

import (
    "os"
    "os/signal"
    "syscall"
    "unicode/utf8"
    "strings"
    "fmt"
    "runtime"
    "unsafe"

    "golang.org/x/sys/unix"
)

type (
    input_event struct {
        data []byte
        err  error
    }
)

var (
    out   *os.File
    in    int

    // term specific keys
    keys  []string

    // termbox inner state
    orig_tios      unix.Termios

    sigio      = make(chan os.Signal, 1)
    quit       = make(chan int)
    inbuf      = make([]byte, 0, 128)
    input_buf  = make(chan input_event)
)

func fcntl(cmd int, arg int) error {
    _, _, e := syscall.Syscall(unix.SYS_FCNTL, uintptr(in), uintptr(cmd), uintptr(arg))
    if e != 0 {
        return e
    }

    return nil
}

func ioctl(cmd int, termios *unix.Termios) error {
    r, _, e := syscall.Syscall(unix.SYS_IOCTL, out.Fd(), uintptr(cmd), uintptr(unsafe.Pointer(termios)))
    if r != 0 {
        return os.NewSyscallError("SYS_IOCTL", e)
    }
    return nil
}

func parse_escape_sequence(buf []byte) (size int, event keyEvent) {
    bufstr := string(buf)
    for i, key := range keys {
        if strings.HasPrefix(bufstr, key) {
            event.rune = 0
            event.key = Key(0xFFFF - i)
            size = len(key)
            return
        }
    }
    return 0, event
}

func extract_event(inbuf []byte) int {
    if len(inbuf) == 0 {
        return 0
    }

    if inbuf[0] == '\033' {
        // possible escape sequence
        if size, event := parse_escape_sequence(inbuf); size != 0 {
            input_comm <- event
            return size
        }

        // it's not a recognized escape sequence, then return Esc
        input_comm <- keyEvent{key: KeyEsc}
        return len(inbuf)
    }

    // if we're here, this is not an escape sequence and not an alt sequence
    // so, it's a FUNCTIONAL KEY or a UNICODE character

    // first of all check if it's a functional key
    if Key(inbuf[0]) <= KeySpace || Key(inbuf[0]) == KeyBackspace2 {
        input_comm <- keyEvent{key: Key(inbuf[0])}
        return 1
    }

    // the only possible option is utf8 rune
    if r, n := utf8.DecodeRune(inbuf); r != utf8.RuneError {
        input_comm <- keyEvent{rune: r}
        return n
    }

    return 0
}

// Wait for an event and return it. This is a blocking function call.
func inputEventsProducer() {
    // try to extract event from input buffer, return on success
    size := extract_event(inbuf)
    if size != 0 {
        copy(inbuf, inbuf[size:])
        inbuf = inbuf[:len(inbuf)-size]
    }

    for {
        select {
        case ev := <-input_buf:
            if ev.err != nil {
                input_comm <- keyEvent{err: ev.err}
                return
            }

            inbuf = append(inbuf, ev.data...)
            size = extract_event(inbuf)
            if size != 0 {
                copy(inbuf, inbuf[size:])
                inbuf = inbuf[:len(inbuf)-size]
            }
        case <-quit:
            return
        }
    }
}

func initConsole() (err error) {
    out, err = os.OpenFile("/dev/tty", unix.O_WRONLY, 0)
    if err != nil {
        return
    }
    in, err = syscall.Open("/dev/tty", unix.O_RDONLY, 0)
    if err != nil {
        return
    }

    err = setup_term()
    if err != nil {
        return fmt.Errorf("Error while reading terminfo data: %v", err)
    }

    signal.Notify(sigio, unix.SIGIO)

    err = fcntl(unix.F_SETFL, unix.O_ASYNC|unix.O_NONBLOCK)
    if err != nil {
        return
    }
    err = fcntl(unix.F_SETOWN, unix.Getpid())
    if runtime.GOOS != "darwin" && err != nil {
        return
    }

    err = ioctl(ioctl_GETATTR, &orig_tios)
    if err != nil {
        return
    }

    tios := orig_tios
    tios.Iflag &^= unix.IGNBRK | unix.BRKINT | unix.PARMRK |
        unix.ISTRIP | unix.INLCR | unix.IGNCR |
        unix.ICRNL | unix.IXON
    tios.Lflag &^= unix.ECHO | unix.ECHONL | unix.ICANON |
        unix.ISIG | unix.IEXTEN
    tios.Cflag &^= unix.CSIZE | unix.PARENB
    tios.Cflag |= unix.CS8
    tios.Cc[unix.VMIN] = 1
    tios.Cc[unix.VTIME] = 0

    err = ioctl(ioctl_SETATTR, &tios)
    if err != nil {
        return err
    }

    go func() {
        buf := make([]byte, 128)
        for {
            select {
            case <-sigio:
                for {
                    n, err := syscall.Read(in, buf)
                    if err == unix.EAGAIN || err == unix.EWOULDBLOCK {
                        break
                    }
                    select {
                    case input_buf <- input_event{buf[:n], err}:
                        continue
                    case <-quit:
                        return
                    }
                }
            case <-quit:
                return
            }
        }
    }()

    go inputEventsProducer()
    return
}

func releaseConsole() {
    quit <- 1
    ioctl(ioctl_SETATTR, &orig_tios)
    out.Close()
    unix.Close(in)
}
