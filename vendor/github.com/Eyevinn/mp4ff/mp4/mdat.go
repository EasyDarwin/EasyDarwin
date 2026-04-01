package mp4

import (
	"errors"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// MdatBox - Media Data Box (mdat)
// The mdat box contains media chunks/samples.
// DataParts is to be able to gather output data without
// new allocations
type MdatBox struct {
	StartPos     uint64
	Data         []byte
	DataParts    [][]byte
	lazyDataSize uint64
	LargeSize    bool
}

const maxNormalPayloadSize = (1 << 32) - 1 - 8

// DecodeMdat - box-specific decode
func DecodeMdat(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	largeSize := hdr.Hdrlen > boxHeaderSize
	return &MdatBox{startPos, data, nil, 0, largeSize}, nil
}

// DecodeMdatSR decodes an mdat box
//
// Currently no content and no error is returned if not full length available.
// If not enough content, an accumulated error is stored in sr, though
func DecodeMdatSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	largeSize := hdr.Hdrlen > boxHeaderSize
	return &MdatBox{startPos, sr.ReadBytes(hdr.payloadLen()), nil, 0, largeSize}, nil
}

// IsLazy - is the mdat data handled lazily (with separate writer/reader).
func (m *MdatBox) IsLazy() bool {
	return m.lazyDataSize > 0
}

// DecodeMdatLazily - box-specific decode but Data is not in memory
func DecodeMdatLazily(hdr BoxHeader, startPos uint64) (Box, error) {
	largeSize := hdr.Hdrlen > boxHeaderSize
	decLazyDataSize := hdr.Size - uint64(hdr.Hdrlen)
	return &MdatBox{startPos, nil, nil, decLazyDataSize, largeSize}, nil
}

// SetLazyDataSize - set size of mdat lazy data so that the data can be written separately
// Don't put any data in m.Data in this mode.
func (m *MdatBox) SetLazyDataSize(newSize uint64) {
	m.lazyDataSize = newSize
}

// GetLazyDataSize - size of the box if filled with data
func (m *MdatBox) GetLazyDataSize() uint64 {
	return m.lazyDataSize
}

// Type - return box type
func (m *MdatBox) Type() string {
	return "mdat"
}

// Size - return calculated size, depending on largeSize set or not
func (m *MdatBox) Size() uint64 {
	dataSize := m.DataLength()

	if m.lazyDataSize > 0 {
		dataSize = m.lazyDataSize
	}
	if dataSize > maxNormalPayloadSize {
		m.LargeSize = true
	}
	size := boxHeaderSize + dataSize
	if m.LargeSize {
		size += 8
	}
	return size
}

// AddSampleData -  a sample data to an mdat box
func (m *MdatBox) AddSampleData(s []byte) {
	m.Data = append(m.Data, s...)
}

// SetData - set the mdat data to given slice. No copying is done
func (m *MdatBox) SetData(data []byte) {
	m.Data = data
	m.lazyDataSize = 0
}

// AddSampleDataPart - add a data part (for output)
func (m *MdatBox) AddSampleDataPart(s []byte) {
	if len(m.Data) != 0 {
		panic("cannot mix sample parts with monolithic sample data")
	}
	if len(m.DataParts) == 0 {
		m.DataParts = make([][]byte, 0, 8) // Reasonable size
	}
	m.DataParts = append(m.DataParts, s)
}

// Encode - write box to w. If m.lazyDataSize > 0, the mdat data needs to be written separately
func (m *MdatBox) Encode(w io.Writer) error {
	err := EncodeHeaderWithSize("mdat", m.Size(), m.LargeSize, w)
	if err != nil {
		return err
	}
	if len(m.DataParts) > 0 {
		for _, dp := range m.DataParts {
			_, err = w.Write(dp)
			if err != nil {
				return err
			}
		}
	} else {
		_, err = w.Write(m.Data)
	}

	return err
}

// Encode - write box to sw. If m.lazyDataSize > 0, the mdat data needs to be written separately
func (m *MdatBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderWithSizeSW("mdat", m.Size(), m.LargeSize, sw)
	if err != nil {
		return err
	}
	if len(m.DataParts) > 0 {
		for _, dp := range m.DataParts {
			sw.WriteBytes(dp)
		}
	} else {
		sw.WriteBytes(m.Data)
	}

	return sw.AccError()
}

// DataLength - length of data stored in box either as one or multiple parts
func (m *MdatBox) DataLength() uint64 {
	dataLength := len(m.Data)
	if len(m.DataParts) > 0 {
		dataLength = 0
		for i := range m.DataParts {
			dataLength += len(m.DataParts[i])
		}
	}
	return uint64(dataLength)
}

// Info - write box-specific information
func (m *MdatBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, m, -1, 0)
	return bd.err
}

// HeaderSize - 8 or 16 (bytes) depending o whether largeSize is used
func (m *MdatBox) HeaderSize() uint64 {
	hSize := boxHeaderSize
	if m.LargeSize {
		hSize += largeSizeLen
	}
	return uint64(hSize)
}

// PayloadAbsoluteOffset - position of mdat payload start (works after header)
func (m *MdatBox) PayloadAbsoluteOffset() uint64 {
	return m.StartPos + m.HeaderSize()
}

// ReadData reads Mdat data specified by the start and size.
// Input argument start is the position relative to the start of a file.
// The ReadSeeker is used for lazily loaded mdat case.
func (m *MdatBox) ReadData(start, size int64, rs io.ReadSeeker) ([]byte, error) {
	// The Mdat box was decoded lazily
	if m.lazyDataSize > 0 {
		if rs == nil {
			return nil, errors.New("lazy mdat mode - expects non-nil readseeker to read data")
		}

		_, err := rs.Seek(start, io.SeekStart)
		if err != nil {
			return nil, fmt.Errorf("lazy mdat mode - unable to seek to %d", start)
		}

		buf := make([]byte, size)
		_, err = io.ReadFull(rs, buf)
		if err != nil {
			return nil, err
		}
		return buf, nil
	}

	// Otherwise, all Mdat data is in memory, either as parts or as one big slice
	mdatPayloadStart := m.PayloadAbsoluteOffset()
	offsetInMdatData := uint64(start) - mdatPayloadStart
	endIndexInMdatData := offsetInMdatData + uint64(size)

	// validate if indexes are valid to avoid panics
	dataLen := m.DataLength()
	if offsetInMdatData >= dataLen || endIndexInMdatData >= dataLen {
		return nil, fmt.Errorf("normal mdat mode - invalid range provided")
	}
	if len(m.DataParts) > 0 {
		return nil, fmt.Errorf("extraction of range from dataParts not yet implemented")
	}
	return m.Data[offsetInMdatData : offsetInMdatData+uint64(size)], nil

}

// CopyData - copy data range from mdat to w.
// The ReadSeeker is used for lazily loaded mdat case.
func (m *MdatBox) CopyData(start, size int64, rs io.ReadSeeker, w io.Writer) (nrWritten int64, err error) {
	// The Mdat box was decoded lazily
	if m.lazyDataSize > 0 {
		if rs == nil {
			return 0, errors.New("lazy mdat mode - expects non-nil readseeker to read data")
		}

		_, err := rs.Seek(start, io.SeekStart)
		if err != nil {
			return 0, fmt.Errorf("lazy mdat mode - unable to seek to %d", start)
		}
		return io.CopyN(w, rs, size)
	}

	// Otherwise, all Mdat data is in memory
	mdatPayloadStart := m.PayloadAbsoluteOffset()
	offsetInMdatData := uint64(start) - mdatPayloadStart
	endIndexInMdatData := offsetInMdatData + uint64(size)

	// validate if indexes are valid to avoid panics
	dataLen := m.DataLength()
	if offsetInMdatData >= dataLen || endIndexInMdatData >= dataLen {
		return 0, fmt.Errorf("normal mdat mode - invalid range provided")
	}
	if len(m.DataParts) > 0 {
		return 0, fmt.Errorf("extraction of range from dataParts not yet implemented")
	}
	var n int
	n, err = w.Write(m.Data[offsetInMdatData : offsetInMdatData+uint64(size)])
	return int64(n), err
}
