go-mp4
------

[![Go Reference](https://pkg.go.dev/badge/github.com/abema/go-mp4.svg)](https://pkg.go.dev/github.com/abema/go-mp4)
![Test](https://github.com/abema/go-mp4/actions/workflows/test.yml/badge.svg)
[![Coverage Status](https://coveralls.io/repos/github/abema/go-mp4/badge.svg)](https://coveralls.io/github/abema/go-mp4)
[![Go Report Card](https://goreportcard.com/badge/github.com/abema/go-mp4)](https://goreportcard.com/report/github.com/abema/go-mp4)

go-mp4 is Go library which provides low-level I/O interfaces of MP4.
This library supports you to parse or build any MP4 boxes(atoms) directly.

go-mp4 provides very flexible interfaces for reading boxes.
If you want to read only specific parts of MP4 file, this library extracts those boxes via io.ReadSeeker interface.

On the other hand, this library is not suitable for complex data conversions.

## Integration with your Go application

### Reading

You can parse MP4 file as follows:

```go
// expand all boxes
_, err := mp4.ReadBoxStructure(file, func(h *mp4.ReadHandle) (interface{}, error) {
	fmt.Println("depth", len(h.Path))

	// Box Type (e.g. "mdhd", "tfdt", "mdat")
	fmt.Println("type", h.BoxInfo.Type.String())

	// Box Size
	fmt.Println("size", h.BoxInfo.Size)

	if h.BoxInfo.IsSupportedType() {
		// Payload
		box, _, err := h.ReadPayload()
		if err != nil {
			return nil, err
		}
		str, err := mp4.Stringify(box, h.BoxInfo.Context)
		if err != nil {
			return nil, err
		}
		fmt.Println("payload", str)

		// Expands children
		return h.Expand()
	}
	return nil, nil
})
```

```go
// extract specific boxes
boxes, err := mp4.ExtractBoxWithPayload(file, nil, mp4.BoxPath{mp4.BoxTypeMoov(), mp4.BoxTypeTrak(), mp4.BoxTypeTkhd()})
if err != nil {
   :
}
for _, box := range boxes {
  tkhd := box.Payload.(*mp4.Tkhd)
  fmt.Println("track ID:", tkhd.TrackID)
}
```

```go
// get basic informations
info, err := mp4.Probe(bufseekio.NewReadSeeker(file, 1024, 4))  
if err != nil {
   :
}
fmt.Println("track num:", len(info.Tracks))
```

### Writing

Writer helps you to write box tree.
The following sample code edits emsg box and writes to another file.

```go
r := bufseekio.NewReadSeeker(inputFile, 128*1024, 4)
w := mp4.NewWriter(outputFile)
_, err = mp4.ReadBoxStructure(r, func(h *mp4.ReadHandle) (interface{}, error) {
	switch h.BoxInfo.Type {
	case mp4.BoxTypeEmsg():
		// write box size and box type
		_, err := w.StartBox(&h.BoxInfo)
		if err != nil {
			return nil, err
		}
		// read payload
		box, _, err := h.ReadPayload()
		if err != nil {
			return nil, err
		}
		// update MessageData
		emsg := box.(*mp4.Emsg)
		emsg.MessageData = []byte("hello world")
		// write box playload
		if _, err := mp4.Marshal(w, emsg, h.BoxInfo.Context); err != nil {
			return nil, err
		}
		// rewrite box size
		_, err = w.EndBox()
		return nil, err
	default:
		// copy all
		return nil, w.CopyBox(r, &h.BoxInfo)
	}
})
```

### User-defined Boxes

You can create additional box definition as follows:

```go
func BoxTypeXxxx() BoxType { return mp4.StrToBoxType("xxxx") }

func init() {
	mp4.AddBoxDef(&Xxxx{}, 0)
}

type Xxxx struct {
	FullBox  `mp4:"0,extend"`
	UI32      uint32 `mp4:"1,size=32"`
	ByteArray []byte `mp4:"2,size=8,len=dynamic"`
}

func (*Xxxx) GetType() BoxType {
	return BoxTypeXxxx()
}
```

### Buffering

go-mp4 has no buffering feature for I/O.
If you should reduce Read function calls, you can wrap the io.ReadSeeker by [bufseekio](https://github.com/sunfish-shogi/bufseekio).

## Command Line Tool

Install mp4tool as follows:

```sh
go install github.com/abema/go-mp4/cmd/mp4tool@latest

mp4tool -help
```

For example, `mp4tool dump MP4_FILE_NAME` command prints MP4 box tree as follows:

```
[moof] Size=504
  [mfhd] Size=16 Version=0 Flags=0x000000 SequenceNumber=1
  [traf] Size=480
    [tfhd] Size=28 Version=0 Flags=0x020038 TrackID=1 DefaultSampleDuration=9000 DefaultSampleSize=33550 DefaultSampleFlags=0x1010000
    [tfdt] Size=20 Version=1 Flags=0x000000 BaseMediaDecodeTimeV1=0
    [trun] Size=424 ... (use -a option to show all)
[mdat] Size=44569 Data=[...] (use -mdat option to expand)
```
