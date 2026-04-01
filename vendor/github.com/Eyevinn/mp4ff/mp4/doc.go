/*
Package mp4 is a library for parsing and writing MP4/ISOBMFF files with a focus on fragmented files.

Most boxes have their own file named after the box four-letter name in the ISO/IEC 14996-12 standard,
but in some cases, there may be multiple boxes that have the same content, and the code is then having a
generic name like visualsampleentry.go.

# Structure and usage

The top level structure for both non-fragmented and fragmented mp4 files is [File].

In a progressive (non-fragmented) [File], the top-level attributes "Ftyp", "Moov", and "Mdat"
point to the corresponding top level boxes.

A fragmented [File] can be more or less complete, like a single init segment,
one or more media segments, or a combination of both, like a CMAF track which renders
into a playable one-track asset. It can also have multiple tracks.
For fragmented files, the following high-level attributes are used:

  - Init is an [*mp4.InitSegment] and contains a ftyp and a moov box and provides the
    general metadata for a fragmented file, track definitions including time scale and sample descriptors.
    It corresponds to a CMAF header. It can also contain one or more `sidx` boxes.
  - Segments is a slice of [mp4.MediaSegment] which start with an optional [mp4.StypBox],
    possibly one or more [mp4.SidxBox] and then one or more [mp4.Fragment].
  - [mp4.Fragment] is a mp4 fragment with exactly one [mp4.MoofBox] followed by a [mp4.MdatBox] where the latter
    contains the media data. It should have one or more [mp4.TrunBox] containing the metadata
    for the samples. The fragment can start with one or more [mp4.EmsgBox].

It should be noted that it is sometimes hard to decide what should belong to a Segment or Fragment.

All child boxes of container boxes such as [mp4.MoofBox] are listed in the Children attribute, but the
most prominent child boxes have direct links with names which makes it possible to write a path such
as

	fragment.Moof.Traf.Trun

to access the (single or first) [mp4.TrunBox] in a fragment inside the (single or first) [mp4.TrafBox]
of a fragment.

There are corresponding structures with a plural form for accessing later boxes of the same type, e.g.

	fragment.Moof.Trafs[1].Trun[1]

to get the second [mp4.TrunBox] of the second [mp4.TrafBox] (provided that they exist). Care must be
taken to assert that none of the intermediate pointers are nil to avoid panic.

# Creating new fragmented files

A typical use case is to generate a fragmented file consisting of an init segment
followed by a series of media segments.

The first step is to create the init segment. This is done in three steps as can be seen in
[examples/initcreator]:

	init := mp4.CreateEmptyInit()
	init.AddEmptyTrack(timescale, mediatype, language)
	init.Moov.Trak.SetHEVCDescriptor("hvc1", vpsNALUs, spsNALUs, ppsNALUs)

Here the third step fills in codec-specific parameters into the sample descriptor of the single track.

The second step is to start producing media segments. They should use the timescale that
was set when creating the init segment. Generally, that timescale should be chosen so that the
sample durations have exact values without rounding errors, e.g. 48000 for 48kHz audio.

A media segment contains one or more fragments.
If all samples are available before the segment is created, one can use a single
fragment in each segment. Example code for this can be found in [examples/segmenter].
For low-latency MPEG-DASH generation, short-duration fragments are added to the segment as the
corresponding media samples become available.

A simple, but not optimal, way of creating a media segment is to first create a slice of
[mp4.FullSample] with the data needed.

The [mp4.Sample] part is what will be written into the [mp4.TrunBox].
Once a number of such full samples are available, they can be added to a media segment like

	seg := mp4.NewMediaSegment()
	frag := mp4.CreateFragment(uint32(segNr), mp4.DefaultTrakID)
	seg.AddFragment(frag)

	for _, sample := range samples {
		frag.AddFullSample(sample)
	}

This segment can finally be output to a [io.Writer]
as

	err := seg.Encode(w)

or to a [bits.SliceWriter] as

	err := seg.EncodeSW(sw)

For multi-track segments, the code is a bit more involved. Please have a look at [examples/segmenter]
to see how it is done. A more optimal way of handling media sample is
to handle them lazily, or using intervals, as explained next.

# Lazy decoding and writing of mdat data

For video and audio, the dominating part of a mp4 file is the media data which is stored
in one or more [mp4.MdatBox]. In some cases, for example when segmenting large progressive
files, it is much more memory efficient to just read the movie or fragment metadata
from the [mp4.MoovBox] or [mp4.MoofBox] and defer the reading of the media data from
the [mp4.MdatBox] to later.

For decoding, this is supported by running
[DecodeFile] in lazy mode as

	parsedMp4, err = mp4.DecodeFile(ifd, mp4.WithDecodeMode(mp4.DecModeLazyMdat))

In this case, the media data of the [mp4.MdatBox] box will not be read, but only its size is being saved.
To read or copy the actual data corresponding to a sample, one must calculate the
corresponding byte range and either call

	func (m *MdatBox) ReadData(start, size int64, rs io.ReadSeeker) ([]byte, error)

or

	func (m *MdatBox) CopyData(start, size int64, rs io.ReadSeeker, w io.Writer) (nrWritten int64, err error)

Example code for this, including lazy writing of [mp4.MdatBox], can be found in [examples/segmenter]
with the lazy mode set.

# More efficient I/O using SliceReader and SliceWriter

The use of the interfaces [io.Reader] and [io.Writer] for reading and writing boxes gives a lot of
flexibility, but is not optimal when it comes to memory allocation. In particular, the
Read(p []byte) method needs a slice "p" of the proper size to read data, which leads to a
lot of allocations and copying of data.
In order to achieve better performance, it is advantageous to read the full top level boxes into
one, or a few, slices and decode these. This is the reason that [bits.SliceReader] and [bits.SliceWriter]
were introduced and that there are double methods for decoding and encoding all boxes using
either of the interfaces. For benchmarks, see the [README.md of the mp4ff module].

Fur further reduction of memory allocation, use a buffered top-level reader, especially when
when reading the [mp4.MdatBox] box of a progressive file.

# More about mp4 boxes

The mp4 package contains a lot of box implementations.

The [Box] interface is specified in box.go. It decodes box size and type in the box header and
dispatches decode for each individual box depending on its type.

There is also a [ContainerBox] interface which is used for boxes that contain other boxes.d

Most boxes have their own file named after the box, but in some cases, there may be multiple boxes
that have the same content, and the box structure and the source code file then has a generic name like
[mp4.VisualSampleEntryBox]

The interfaces define common Box methods including encode (writing),
but not the decode (parsing) methods which have distinct names for each box type and are
dispatched from the parsed box name.

That dispatch based on box name is defined by the tables "mp4.decodersSR" and "mp4.decoders"
for the functions "mp4.DecodeBoxSR" and "mp4.DecodeBox", respectively.
The "SR" variant that uses [bits/SliceReader] should normally be used for better performance.
If a box name is unkonwn, it will result in an [mp4.UnknownBox] being created.

# How to implement a new box

To implement a new box "fooo", the following is needed.
 1. Create a new file "fooo.go" and create a struct type "FoooBox".
 2. "FoooBox" must implement the [mp4.Box] interface methods
 3. It also needs its own decode methods "DecodeFoooSR" and  "DecodeFooo",
    which must be added in the "decodersSR" map and "decoders" map, respectively
    For a simple example, look at the [mp4.PrftBox].
 4. A test file `fooo_test.go` should also have a test using the method "boxDiffAfterEncodeAndDecode"
    to check that the box information is equal after encoding and decoding.

# Direct changes of attributes

Many attributes are public and can therefore be changed in freely.
The advantage of this is that it is possible to write code that can manipulate boxes
in many different ways, but one must be cautious to avoid breaking links to sub boxes or
create inconsistent states in the boxes.

As an example, container boxes such as [mp4.TrafBox] have a method "AddChild" which
adds a box to "Children", its slice of children boxes, but also sets a specific
member reference such as "Tfdt" to point to that box. If "Children" is manipulated
directly, that link may no longer be valid.

# Encoding modes and optimizations

For fragmented files, one can choose to either encode all boxes in a [mp4.File], or only code
the ones which are included in the init and media segments. The attribute that controls that
is called [mp4.FragEncMode].
Another attribute [mp4.EncOptimize] controls possible optimizations of the file encoding process.
Currently, there is only one possible optimization called [mp4.OptimizeTrun].
It can reduce the size of the [mp4.TrunBox] by finding and writing default
values in the [mp4.TfhdBox] and omitting the corresponding values from the [mp4.TrunBox].
Note that this may change the size of all ancestor boxes of the [mp4.TrunBox].

# Sample Number Offset

Following the ISOBMFF standard, sample numbers and other numbers start at 1 (one-based).
This applies to arguments of functions and methods.
The actual storage in slices is zero-based, so sample nr 1 has index 0 in the corresponding slice.

[examples/initcreator]: https://pkg.go.dev/Eyevinn/mp4ff/examples/initcreator
[examples/segmenter]: https://pkg.go.dev/Eyevinn/mp4ff/examples/segmenter
[README.md of the mp4ff module]: https://pkg.go.dev/github.com/Eyevinn/mp4ff#section-readme
[bits/SliceReader]: https://pkg.go.dev/Eyevinn/mp4ff/bits#SliceReader

[bits/SliceWriter]: https://pkg.go.dev/Eyevinn/mp4ff/bits#SliceWriter
*/
package mp4
