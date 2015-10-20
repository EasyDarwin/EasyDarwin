//
// impl/file.ipp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_FILE_IPP
#define AVHTTP_FILE_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#ifdef WIN32	// windows part
#define atoll _atoi64

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winioctl.h>

#else			// posix part

#define _FILE_OFFSET_BITS	64

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE		600
#endif

#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#if !defined(__APPLE__) && !defined(__OpenBSD__) && !defined(__ANDROID__)
#     include <sys/statvfs.h>
#     define BOOST_STATVFS statvfs
#     define BOOST_STATVFS_F_FRSIZE vfs.f_frsize
#else
#     ifdef __ANDROID__
#     include <sys/vfs.h>
#     endif
#     ifdef __OpenBSD__
#     include <sys/param.h>
#     endif
#     include <sys/mount.h>
#     define BOOST_STATVFS statfs
#     define BOOST_STATVFS_F_FRSIZE static_cast<boost::uintmax_t>(vfs.f_bsize)
#endif

#endif

#ifdef WIN32

#include <malloc.h>
#define AVHTTP_ALLOCA(t, n) static_cast<t*>(_alloca(sizeof(t) * (n)))

#else

#include <stdlib.h>
#define AVHTTP_ALLOCA(t, n) static_cast<t*>(alloca(sizeof(t) * (n)))

#endif

namespace avhttp {

file::file()
#ifdef WIN32
	: m_file_handle(INVALID_HANDLE_VALUE)
#else
	: m_fd(-1)
#endif
#if defined WIN32 || defined __linux__ || defined DEBUG
	, m_page_size(0)
#endif
	, m_open_mode(0)
#if defined WIN32 || defined __linux__
	, m_sector_size(0)
#endif
{}

file::file(fs::path const& p, int m, boost::system::error_code& ec)
#ifdef WIN32
	: m_file_handle(INVALID_HANDLE_VALUE)
#else
	: m_fd(-1)
#endif
#if defined WIN32 || defined __linux__ || defined DEBUG
	, m_page_size(0)
#endif
	, m_open_mode(0)
#if defined WIN32 || defined __linux__
	, m_sector_size(0)
#endif
{
	open(p, m, ec);
}

file::~file(void)
{
	close();
}

void file::open(fs::path const& p, int m)
{
	boost::system::error_code ec;
	open(p, m, ec);
	if (ec)
	{
		boost::throw_exception(boost::system::system_error(ec));
	}
}

void file::open(fs::path const& path, int mode, boost::system::error_code& ec)
{
	ec = boost::system::error_code();

	close();

#ifdef WIN32

	struct open_mode_t
	{
		DWORD rw_mode;
		DWORD share_mode;
		DWORD create_mode;
		DWORD flags;
	};

	const static open_mode_t mode_array[] =
	{
		// read_only
		{GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS},
		// write_only
		{GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS},
		// read_write
		{GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS},
		// invalid option
		{0, 0, 0, 0},
		// read_only no_buffer
		{GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING },
		// write_only no_buffer
		{GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING },
		// read_write no_buffer
		{GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING },
		// invalid option
		{0, 0, 0, 0}
	};

	const static DWORD attrib_array[] =
	{
		FILE_ATTRIBUTE_NORMAL, // no attrib
		FILE_ATTRIBUTE_HIDDEN, // hidden
		FILE_ATTRIBUTE_NORMAL, // executable
		FILE_ATTRIBUTE_HIDDEN, // hidden + executable
	};

	m_path = path.string();

	BOOST_ASSERT((mode & mode_mask) < sizeof(mode_array)/sizeof(mode_array[0]));
	open_mode_t const& m = mode_array[mode & mode_mask];
	DWORD a = attrib_array[(mode & attribute_mask) >> 12];
	m_file_handle = ::CreateFileA(m_path.c_str(), m.rw_mode, m.share_mode, 0
		, m.create_mode, m.flags | (a ? a : FILE_ATTRIBUTE_NORMAL), 0);

	if (m_file_handle == INVALID_HANDLE_VALUE)
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return;
	}

	// try to make the file sparse if supported
	if (mode & file::sparse)
	{
		DWORD temp;
		::DeviceIoControl(m_file_handle, FSCTL_SET_SPARSE, 0, 0
			, 0, 0, &temp, 0);
	}
#else
	// rely on default umask to filter x and w permissions
	// for group and others
	int permissions = S_IRUSR | S_IWUSR
		| S_IRGRP | S_IWGRP
		| S_IROTH | S_IWOTH;

	if (mode & attribute_executable)
		permissions |= S_IXGRP | S_IXOTH | S_IXUSR;

	static const int mode_array[] = {O_RDONLY, O_WRONLY | O_CREAT, O_RDWR | O_CREAT};
#ifdef O_DIRECT
	static const int no_buffer_flag[] = {0, O_DIRECT};
#else
	static const int no_buffer_flag[] = {0, 0};
#endif

	m_fd = ::open(path.string().c_str()
		, mode_array[mode & rw_mask] | no_buffer_flag[(mode & no_buffer) >> 2], permissions);

#ifdef __linux__
	// workaround for linux bug
	// https://bugs.launchpad.net/ubuntu/+source/linux/+bug/269946
	if (m_fd == -1 && (mode & no_buffer) && errno == EINVAL)
	{
		mode &= ~no_buffer;
		m_fd = ::open(path.string().c_str()
			, mode & (rw_mask | no_buffer), permissions);
	}

#endif
	if (m_fd == -1)
	{
		ec = boost::system::error_code(errno, boost::system::generic_category());
		BOOST_ASSERT(ec);
		return;
	}

#ifdef F_NOCACHE
	if (mode & no_buffer)
	{
		int yes = 1;
		fcntl(m_fd, F_NOCACHE, &yes);
	}
#endif

#ifdef POSIX_FADV_RANDOM
	// disable read-ahead
	posix_fadvise(m_fd, 0, 0, POSIX_FADV_RANDOM);
#endif

#endif
	m_open_mode = mode;

	BOOST_ASSERT(is_open());
	return;
}

bool file::is_open() const
{
#ifdef WIN32
	return m_file_handle != INVALID_HANDLE_VALUE;
#else
	return m_fd != -1;
#endif
}

int file::pos_alignment() const
{
	// on linux and windows, file offsets needs
	// to be aligned to the disk sector size
#ifdef __ANDROID__
	return 1;
#elif defined __linux__
	if (m_sector_size == 0)
	{
		struct BOOST_STATVFS vfs;
		if (fstatvfs(m_fd, &vfs) == 0)
			m_sector_size = BOOST_STATVFS_F_FRSIZE ;
		else
			m_sector_size = 4096;
	}
	return m_sector_size;
#elif defined WIN32
	if (m_sector_size == 0)
	{
		DWORD sectors_per_cluster;
		DWORD bytes_per_sector;
		DWORD free_clusters;
		DWORD total_clusters;

		char backslash = '\\';
		if (::GetDiskFreeSpaceA(m_path.substr(0, m_path.find_first_of(backslash)+1).c_str()
			, &sectors_per_cluster, &bytes_per_sector
			, &free_clusters, &total_clusters))
		{
			m_sector_size = bytes_per_sector;
			m_cluster_size = sectors_per_cluster * bytes_per_sector;
		}
		else
		{
			// make a conservative guess
			m_sector_size = 512;
			m_cluster_size = 4096;
		}
	}
	return m_sector_size;
#else
	return 1;
#endif
}

int file::buf_alignment() const
{
#if defined WIN32
	init_file();
	return m_page_size;
#else
	return pos_alignment();
#endif
}

int file::size_alignment() const
{
#if defined WIN32
	init_file();
	return m_page_size;
#else
	return pos_alignment();
#endif
}

void file::close()
{
#if defined WIN32 || defined __linux__
	m_sector_size = 0;
#endif

#ifdef WIN32
	if (m_file_handle == INVALID_HANDLE_VALUE) return;
	::CloseHandle(m_file_handle);
	m_file_handle = INVALID_HANDLE_VALUE;
	m_path.clear();
#else
	if (m_fd == -1) return;
	::close(m_fd);
	m_fd = -1;
#endif
	m_open_mode = 0;
#if defined WIN32 || defined __linux__ || defined DEBUG
	m_page_size = 0;
#endif
}

inline int bufs_size(file::iovec_t const* bufs, int num_bufs)
{
	int size = 0;
	for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
		size += i->iov_len;
	return size;
}

static inline int page_size()
{
	static int s = 0;
	if (s != 0) return s;

#ifdef WIN32
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	s = si.dwPageSize;
#else
	s = sysconf(_SC_PAGESIZE);
#endif
	// assume the page size is 4 kiB if we
	// fail to query it
	if (s <= 0) s = 4096;
	return s;
}

#if defined WIN32 || defined __linux__ || defined DEBUG

void file::init_file() const
{
	if (m_page_size != 0) return;
	m_page_size = page_size();
}

#endif

file::size_type file::read(size_type offset, char* buf, int size)
{
	file::iovec_t bufs;
	bufs.iov_base = (void*)buf;
	bufs.iov_len = size;
	boost::system::error_code ec;
	size_type bytes = readv(offset, &bufs, 1, ec);
	BOOST_ASSERT(!ec);
	return bytes;
}

file::size_type file::read(char* buf, int size)
{
	file::iovec_t bufs;
	bufs.iov_base = (void*)buf;
	bufs.iov_len = size;
	boost::system::error_code ec;
	size_type bytes = readv(-1, &bufs, 1, ec);
	BOOST_ASSERT(!ec);
	return bytes;
}

file::size_type file::readv(file::size_type file_offset,
	iovec_t const* bufs, int num_bufs, boost::system::error_code& ec)
{
	ec = boost::system::error_code();

	BOOST_ASSERT((m_open_mode & rw_mask) == read_only || (m_open_mode & rw_mask) == read_write);
	BOOST_ASSERT(bufs);
	BOOST_ASSERT(num_bufs > 0);
	BOOST_ASSERT(is_open());

#if defined WIN32 || defined __linux__ || defined DEBUG
	// make sure m_page_size is initialized
	init_file();
#endif

#ifdef WIN32
	if (file_offset == -1)
	{
		LARGE_INTEGER position = { 0 };
		if (::SetFilePointerEx(m_file_handle, position, &position, FILE_CURRENT) == FALSE)
		{
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			return -1;
		}
		file_offset = position.QuadPart;
	}
#else
	if (file_offset == -1)
	{
		file_offset = lseek(m_fd, 0, SEEK_CUR);
		if (file_offset < 0)
		{
			ec = boost::system::error_code(errno, boost::system::generic_category());
			return -1;
		}
	}
	else
	{
		size_type ret = lseek(m_fd, file_offset, SEEK_SET);
		if (ret < 0)
		{
			ec = boost::system::error_code(errno, boost::system::generic_category());
			return -1;
		}
	}
#endif // WIN32

#ifdef DEBUG
	if (m_open_mode & no_buffer)
	{
		bool eof = false;
		int size = 0;
		// when opened in no_buffer mode, the file_offset must
		// be aligned to pos_alignment()
		BOOST_ASSERT((file_offset & (pos_alignment()-1)) == 0);
		for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
		{
			BOOST_ASSERT((uintptr_t(i->iov_base) & (buf_alignment()-1)) == 0);
			// every buffer must be a multiple of the page size
			// except for the last one
			BOOST_ASSERT((i->iov_len & (size_alignment()-1)) == 0 || i == end-1);
			if ((i->iov_len & (size_alignment()-1)) != 0) eof = true;
			size += i->iov_len;
		}
		boost::system::error_code code;
		if (eof) BOOST_ASSERT(file_offset + size >= get_size(code));
	}
#endif

#ifdef WIN32

	DWORD ret = 0;

	// since the ReadFileScatter requires the file to be opened
	// with no buffering, and no buffering requires page aligned
	// buffers, open the file in non-buffered mode in case the
	// buffer is not aligned. Most of the times the buffer should
	// be aligned though

	if ((m_open_mode & no_buffer) == 0)
	{
		// this means the buffer base or the buffer size is not aligned
		// to the page size. Use a regular file for this operation.

		LARGE_INTEGER offs;
		offs.QuadPart = file_offset;
		if (::SetFilePointerEx(m_file_handle, offs, &offs, FILE_BEGIN) == FALSE)
		{
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			return -1;
		}

		for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
		{
			DWORD intermediate = 0;
			if (::ReadFile(m_file_handle, (char*)i->iov_base
				, (DWORD)i->iov_len, &intermediate, 0) == FALSE)
			{
				ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
				return -1;
			}
			ret += intermediate;
		}
		return ret;
	}

	int size = bufs_size(bufs, num_bufs);
	// number of pages for the read. round up
	int num_pages = (size + m_page_size - 1) / m_page_size;
	// allocate array of FILE_SEGMENT_ELEMENT for ReadFileScatter
	FILE_SEGMENT_ELEMENT* segment_array = AVHTTP_ALLOCA(FILE_SEGMENT_ELEMENT, num_pages + 1);
	FILE_SEGMENT_ELEMENT* cur_seg = segment_array;

	for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
	{
		for (std::size_t k = 0; k < i->iov_len; k += m_page_size)
		{
			cur_seg->Buffer = ((char*)i->iov_base) + k;
			++cur_seg;
		}
	}
	// terminate the array
	cur_seg->Buffer = 0;

	OVERLAPPED ol;
	ol.Internal = 0;
	ol.InternalHigh = 0;
	ol.OffsetHigh = file_offset >> 32;
	ol.Offset = file_offset & 0xffffffff;
	ol.hEvent = ::CreateEvent(0, true, false, 0);

	ret += size;
	size = num_pages * m_page_size;
	if (::ReadFileScatter(m_file_handle, segment_array, size, 0, &ol) == 0)
	{
		DWORD last_error = ::GetLastError();
		if (last_error != ERROR_IO_PENDING)
		{
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			::CloseHandle(ol.hEvent);
			return -1;
		}
		if (::GetOverlappedResult(m_file_handle, &ol, &ret, true) == 0)
		{
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			::CloseHandle(ol.hEvent);
			return -1;
		}
	}
	::CloseHandle(ol.hEvent);
	return ret;

#else // WIN32

#if AVHTTP_USE_READV

#ifdef __linux__
	bool aligned = false;
	int size = 0;
	// if we're not opened in no-buffer mode, we don't need alignment
	if ((m_open_mode & no_buffer) == 0) aligned = true;
	if (!aligned)
	{
		size = bufs_size(bufs, num_bufs);
		if ((size & (size_alignment()-1)) == 0) aligned = true;
	}
	if (aligned)
#endif // __linux__
	{
		ret = ::readv(m_fd, bufs, num_bufs);
		if (ret < 0)
		{
			ec = boost::system::error_code(errno, boost::system::generic_category());
			return -1;
		}
		return ret;
	}
#ifdef __linux__
	file::iovec_t* temp_bufs = AVHTTP_ALLOCA(file::iovec_t, num_bufs);
	memcpy(temp_bufs, bufs, sizeof(file::iovec_t) * num_bufs);
	iovec_t& last = temp_bufs[num_bufs-1];
	last.iov_len = (last.iov_len & ~(size_alignment()-1)) + m_page_size;
	ret = ::readv(m_fd, temp_bufs, num_bufs);
	if (ret < 0)
	{
		ec = boost::system::error_code(errno, boost::system::generic_category());
		return -1;
	}
	return (std::min)(ret, size_type(size));
#endif // __linux__

#else // AVHTTP_USE_READV

	file::size_type ret = 0;

	for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
	{
		int tmp = ::read(m_fd, i->iov_base, i->iov_len);
		if (tmp < 0)
		{
			ec = boost::system::error_code(errno, boost::system::generic_category());
			return -1;
		}
		ret += tmp;
		if (static_cast<std::size_t>(tmp) < i->iov_len) break;
	}
	return ret;

#endif // AVHTTP_USE_READV

#endif // WIN32
}

file::size_type file::write(const char* buf, int size)
{
	file::iovec_t bufs;
	bufs.iov_base = (void*)buf;
	bufs.iov_len = size;
	boost::system::error_code ec;
	size_type bytes = writev(-1, &bufs, 1, ec);
	BOOST_ASSERT(!ec);
	return bytes;
}

file::size_type file::write(size_type offset, const char* buf, int size)
{
	file::iovec_t bufs;
	bufs.iov_base = (void*)buf;
	bufs.iov_len = size;
	boost::system::error_code ec;
	size_type bytes = writev(offset, &bufs, 1, ec);
	BOOST_ASSERT(!ec);
	return bytes;
}

file::size_type file::writev(file::size_type file_offset, iovec_t const* bufs, int num_bufs, boost::system::error_code& ec)
{
	ec = boost::system::error_code();

	BOOST_ASSERT((m_open_mode & rw_mask) == write_only || (m_open_mode & rw_mask) == read_write);
	BOOST_ASSERT(bufs);
	BOOST_ASSERT(num_bufs > 0);
	BOOST_ASSERT(is_open());

#if defined WIN32 || defined __linux__ || defined DEBUG
	// make sure m_page_size is initialized
	init_file();
#endif

#ifdef WIN32
	if (file_offset == -1)
	{
		LARGE_INTEGER position = { 0 };
		if (::SetFilePointerEx(m_file_handle, position, &position, FILE_CURRENT) == FALSE)
		{
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			return -1;
		}
		file_offset = position.QuadPart;
	}
#else
	if (file_offset != -1)
	{
		size_type ret = lseek(m_fd, file_offset, SEEK_SET);
		if (ret < 0)
		{
			ec = boost::system::error_code(errno, boost::system::generic_category());
			return -1;
		}
	}
	else
	{
		file_offset = lseek(m_fd, 0, SEEK_CUR);
		if (file_offset < 0)
		{
			ec = boost::system::error_code(errno, boost::system::generic_category());
			return -1;
		}
	}
#endif // WIN32

#ifdef DEBUG
	if (m_open_mode & no_buffer)
	{
		bool eof = false;
		int size = 0;
		// when opened in no_buffer mode, the file_offset must
		// be aligned to pos_alignment()
		if (file_offset != -1)
		{
			BOOST_ASSERT((file_offset & (pos_alignment()-1)) == 0);
		}
		for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
		{
			BOOST_ASSERT((uintptr_t(i->iov_base) & (buf_alignment()-1)) == 0);
			// every buffer must be a multiple of the page size
			// except for the last one
			BOOST_ASSERT((i->iov_len & (size_alignment()-1)) == 0 || i == end-1);
			if ((i->iov_len & (size_alignment()-1)) != 0) eof = true;
			size += i->iov_len;
		}
		boost::system::error_code code;
		if (eof && file_offset != -1) BOOST_ASSERT(file_offset + size >= get_size(code));
	}
#endif

#ifdef WIN32

	DWORD ret = 0;

	// since the ReadFileScatter requires the file to be opened
	// with no buffering, and no buffering requires page aligned
	// buffers, open the file in non-buffered mode in case the
	// buffer is not aligned. Most of the times the buffer should
	// be aligned though

	if ((m_open_mode & no_buffer) == 0)
	{
		// this means the buffer base or the buffer size is not aligned
		// to the page size. Use a regular file for this operation.

		LARGE_INTEGER offs;
		offs.QuadPart = file_offset;
		if (::SetFilePointerEx(m_file_handle, offs, &offs, FILE_BEGIN) == FALSE)
		{
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			return -1;
		}

		for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
		{
			DWORD intermediate = 0;
			if (::WriteFile(m_file_handle, (char const*)i->iov_base
				, (DWORD)i->iov_len, &intermediate, 0) == FALSE)
			{
				ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
				return -1;
			}
			ret += intermediate;
		}
		return ret;
	}

	int size = bufs_size(bufs, num_bufs);
	// number of pages for the write. round up
	int num_pages = (size + m_page_size - 1) / m_page_size;
	// allocate array of FILE_SEGMENT_ELEMENT for WriteFileGather
	FILE_SEGMENT_ELEMENT* segment_array = AVHTTP_ALLOCA(FILE_SEGMENT_ELEMENT, num_pages + 1);
	FILE_SEGMENT_ELEMENT* cur_seg = segment_array;

	for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
	{
		for (std::size_t k = 0; k < i->iov_len; k += m_page_size)
		{
			cur_seg->Buffer = ((char*)i->iov_base) + k;
			++cur_seg;
		}
	}
	// terminate the array
	cur_seg->Buffer = 0;

	OVERLAPPED ol;
	ol.Internal = 0;
	ol.InternalHigh = 0;
	ol.OffsetHigh = file_offset >> 32;
	ol.Offset = file_offset & 0xffffffff;
	ol.hEvent = ::CreateEvent(0, true, false, 0);

	ret += size;
	// if file_size is > 0, the file will be opened in unbuffered
	// mode after the write completes, and truncate the file to
	// file_size.
	size_type file_size = 0;

	if ((size & (m_page_size-1)) != 0)
	{
		// if size is not an even multiple, this must be the tail
		// of the file. Write the whole page and then open a new
		// file without FILE_FLAG_NO_BUFFERING and set the
		// file size to file_offset + size

		file_size = file_offset + size;
		size = num_pages * m_page_size;
	}

	if (::WriteFileGather(m_file_handle, segment_array, size, 0, &ol) == 0)
	{
		if (::GetLastError() != ERROR_IO_PENDING)
		{
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			::CloseHandle(ol.hEvent);
			return -1;
		}
		DWORD tmp;
		if (::GetOverlappedResult(m_file_handle, &ol, &tmp, true) == 0)
		{
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			::CloseHandle(ol.hEvent);
			return -1;
		}
		if (tmp < ret) ret = tmp;
	}
	::CloseHandle(ol.hEvent);

	if (file_size > 0)
	{
		HANDLE f = ::CreateFileA(m_path.c_str(), GENERIC_WRITE
			, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING
			, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);

		if (f == INVALID_HANDLE_VALUE)
		{
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			return -1;
		}

		LARGE_INTEGER offs;
		offs.QuadPart = file_size;
		if (::SetFilePointerEx(f, offs, &offs, FILE_BEGIN) == FALSE)
		{
			::CloseHandle(f);
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			return -1;
		}
		if (::SetEndOfFile(f) == FALSE)
		{
			ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
			::CloseHandle(f);
			return -1;
		}
		::CloseHandle(f);
	}

	return ret;
#else

#if AVHTTP_USE_WRITEV

#ifdef __linux__
	bool aligned = false;
	int size = 0;
	// if we're not opened in no-buffer mode, we don't need alignment
	if ((m_open_mode & no_buffer) == 0) aligned = true;
	if (!aligned)
	{
		size = bufs_size(bufs, num_bufs);
		if ((size & (size_alignment()-1)) == 0) aligned = true;
	}
	if (aligned)
#endif
	{
		ret = ::writev(m_fd, bufs, num_bufs);
		if (ret < 0)
		{
			ec = boost::system::error_code(errno, boost::system::generic_category());
			return -1;
		}
		return ret;
	}
#ifdef __linux__
	file::iovec_t* temp_bufs = AVHTTP_ALLOCA(file::iovec_t, num_bufs);
	memcpy(temp_bufs, bufs, sizeof(file::iovec_t) * num_bufs);
	iovec_t& last = temp_bufs[num_bufs-1];
	last.iov_len = (last.iov_len & ~(size_alignment()-1)) + size_alignment();
	ret = ::writev(m_fd, temp_bufs, num_bufs);
	if (ret < 0)
	{
		ec = boost::system::error_code(errno, boost::system::generic_category());
		return -1;
	}
	if (ftruncate(m_fd, file_offset + size) < 0)
	{
		ec = boost::system::error_code(errno, boost::system::generic_category());
		return -1;
	}
	return (std::min)(ret, size_type(size));
#endif // __linux__

#else // AVHTTP_USE_WRITEV

	file::size_type ret = 0;
	for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
	{
		int tmp = ::write(m_fd, i->iov_base, i->iov_len);
		if (tmp < 0)
		{
			ec = boost::system::error_code(errno, boost::system::generic_category());
			return -1;
		}
		ret += tmp;
		if (static_cast<std::size_t>(tmp) < i->iov_len) break;
	}
	return ret;

#endif // AVHTTP_USE_WRITEV

#endif // WIN32
}

bool file::flush()
{
#ifdef WIN32
	BOOL ret = ::FlushFileBuffers(m_file_handle);
	return ret ? true : false;
#else
	int ret = fsync(m_fd);
	return ret == 0 ? true : false;
#endif
	return false;
}

file::size_type file::offset(boost::system::error_code& ec)
{
	ec = boost::system::error_code();
#ifdef WIN32
	LARGE_INTEGER position = { 0 };
	if (::SetFilePointerEx(m_file_handle, position, &position, FILE_CURRENT) == FALSE)
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return -1;
	}
	return position.QuadPart;
#else
	size_type file_offset = lseek(m_fd, 0, SEEK_CUR);
	if (file_offset < 0)
	{
		ec = boost::system::error_code(errno, boost::system::generic_category());
		return -1;
	}
	return file_offset;
#endif // WIN32
}

file::size_type file::offset(file::size_type offset, boost::system::error_code& ec)
{
	ec = boost::system::error_code();
#ifdef WIN32
	LARGE_INTEGER offs;
	offs.QuadPart = offset;
	if (::SetFilePointerEx(m_file_handle, offs, &offs, FILE_BEGIN) == FALSE)
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return -1;
	}
	return offs.QuadPart;
#else
	size_type ret = lseek(m_fd, offset, SEEK_SET);
	if (ret < 0)
	{
		ec = boost::system::error_code(errno, boost::system::generic_category());
		return -1;
	}
	return ret;
#endif // WIN32
}

file::size_type file::phys_offset(file::size_type offset)
{
#ifdef FIEMAP_EXTENT_UNKNOWN
	// for documentation of this feature
	// http://lwn.net/Articles/297696/
	struct
	{
		struct fiemap fiemap;
		struct fiemap_extent extent;
	} fm;

	memset(&fm, 0, sizeof(fm));
	fm.fiemap.fm_start = offset;
	fm.fiemap.fm_length = size_alignment();
	// this sounds expensive
	fm.fiemap.fm_flags = FIEMAP_FLAG_SYNC;
	fm.fiemap.fm_extent_count = 1;

	if (ioctl(m_fd, FS_IOC_FIEMAP, &fm) == -1)
		return 0;

	if (fm.fiemap.fm_extents[0].fe_flags & FIEMAP_EXTENT_UNKNOWN)
		return 0;

	// the returned extent is not guaranteed to start
	// at the requested offset, adjust for that in
	// case they differ
	BOOST_ASSERT(offset >= fm.fiemap.fm_extents[0].fe_logical);
	return fm.fiemap.fm_extents[0].fe_physical + (offset - fm.fiemap.fm_extents[0].fe_logical);

#elif defined F_LOG2PHYS
	// for documentation of this feature
	// http://developer.apple.com/mac/library/documentation/Darwin/Reference/ManPages/man2/fcntl.2.html

	log2phys l;
	size_type ret = lseek(m_fd, offset, SEEK_SET);
	if (ret < 0) return 0;
	if (fcntl(m_fd, F_LOG2PHYS, &l) == -1) return 0;
	return l.l2p_devoffset;
#elif defined WIN32
	// for documentation of this feature
	// http://msdn.microsoft.com/en-us/library/aa364572(VS.85).aspx
	STARTING_VCN_INPUT_BUFFER in;
	RETRIEVAL_POINTERS_BUFFER out;
	DWORD out_bytes;

	// query cluster size
	pos_alignment();
	in.StartingVcn.QuadPart = offset / m_cluster_size;
	int cluster_offset = in.StartingVcn.QuadPart % m_cluster_size;

	if (::DeviceIoControl(m_file_handle, FSCTL_GET_RETRIEVAL_POINTERS, &in
		, sizeof(in), &out, sizeof(out), &out_bytes, 0) == 0)
	{
		DWORD error = ::GetLastError();
		BOOST_ASSERT(error != ERROR_INVALID_PARAMETER);

		// insufficient buffer error is expected, but we're
		// only interested in the first extent anyway
		if (error != ERROR_MORE_DATA) return 0;
	}
	if (out_bytes < sizeof(out)) return 0;
	if (out.ExtentCount == 0) return 0;
	if (out.Extents[0].Lcn.QuadPart == (LONGLONG)-1) return 0;
	BOOST_ASSERT(in.StartingVcn.QuadPart >= out.StartingVcn.QuadPart);
	return (out.Extents[0].Lcn.QuadPart
		+ (in.StartingVcn.QuadPart - out.StartingVcn.QuadPart))
		* m_cluster_size + cluster_offset;
#endif
	return 0;
}

bool file::set_size(size_type s, boost::system::error_code& ec)
{
	ec = boost::system::error_code();
	BOOST_ASSERT(is_open());
	BOOST_ASSERT(s >= 0);

#ifdef WIN32
	LARGE_INTEGER offs;
	LARGE_INTEGER cur_size;
	if (::GetFileSizeEx(m_file_handle, &cur_size) == FALSE)
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return false;
	}
	offs.QuadPart = s;
	// only set the file size if it's not already at
	// the right size. We don't want to update the
	// modification time if we don't have to
	if (cur_size.QuadPart != s)
	{
		if (::SetFilePointerEx(m_file_handle, offs, &offs, FILE_BEGIN) == FALSE)
		{
			ec.assign(::GetLastError(), boost::system::system_category());
			return false;
		}
		if (::SetEndOfFile(m_file_handle) == FALSE)
		{
			ec.assign(::GetLastError(), boost::system::system_category());
			return false;
		}
	}
#if _WIN32_WINNT >= 0x501
	if ((m_open_mode & sparse) == 0)
	{
		// only allocate the space if the file
		// is not fully allocated
		DWORD high_dword = 0;
		offs.LowPart = ::GetCompressedFileSizeA(m_path.c_str(), &high_dword);
		offs.HighPart = high_dword;
		ec.assign(::GetLastError(), boost::system::system_category());
		if (ec) return false;
		if (offs.QuadPart != s)
		{
			// if the user has permissions, avoid filling
			// the file with zeroes, but just fill it with
			// garbage instead
			::SetFileValidData(m_file_handle, offs.QuadPart);
		}
	}
#endif // _WIN32_WINNT >= 0x501
#else
	struct stat st;
	if (fstat(m_fd, &st) != 0)
	{
		ec.assign(errno, boost::system::generic_category());
		return false;
	}

	// only truncate the file if it doesn't already
	// have the right size. We don't want to update
	if (st.st_size != s && ftruncate(m_fd, s) < 0)
	{
		ec.assign(errno, boost::system::generic_category());
		return false;
	}

	// if we're not in sparse mode, allocate the storage
	// but only if the number of allocated blocks for the file
	// is less than the file size. Otherwise we would just
	// update the modification time of the file for no good
	// reason.
	if ((m_open_mode & sparse) == 0
		&& st.st_blocks < (s + st.st_blksize - 1) / st.st_blksize)
	{
		// How do we know that the file is already allocated?
		// if we always try to allocate the space, we'll update
		// the modification time without actually changing the file
		// but if we don't do anything if the file size is
#ifdef F_PREALLOCATE
		fstore_t f = {F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, s, 0};
		if (fcntl(m_fd, F_PREALLOCATE, &f) < 0)
		{
			ec = boost::system::error_code(errno, boost::system::generic_category());
			return false;
		}
#endif // F_PREALLOCATE

#if HAVE_FALLOCATE
		{
			int ret = fallocate(m_fd, 0, 0, s);
			// if we return 0, everything went fine
			// the fallocate call succeeded
			if (ret == 0) return true;
			// otherwise, something went wrong. If the error
			// is ENOSYS, just keep going and do it the old-fashioned
			// way. If fallocate failed with some other error, it
			// probably means the user should know about it, error out
			// and report it.
			if (errno != ENOSYS)
			{
				ec.assign(ret, boost::system::generic_category());
				return false;
			}
		}
#endif // __linux__

#if AVHTTP_HAS_FALLOCATE
		{
			// if fallocate failed, we have to use posix_fallocate
			// which can be painfully slow
			// if you get a compile error here, you might want to
			// define AVHTTP_HAS_FALLOCATE to 0.
			int ret = posix_fallocate(m_fd, 0, s);
			if (ret != 0)
			{
				ec = boost::system::error_code(ret, boost::system::generic_category());
				return false;
			}
		}
#endif // AVHTTP_HAS_FALLOCATE
	}
#endif // WIN32
	return true;
}

file::size_type file::get_size(boost::system::error_code& ec) const
{
	ec = boost::system::error_code();
#ifdef WIN32
	LARGE_INTEGER file_size;
	if (!::GetFileSizeEx(m_file_handle, &file_size))
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return -1;
	}
	return file_size.QuadPart;
#else
	struct stat fs;
	if (fstat(m_fd, &fs) != 0)
	{
		ec = boost::system::error_code(errno, boost::system::generic_category());
		return -1;
	}
	return fs.st_size;
#endif
}

file::size_type file::sparse_end(size_type start) const
{
#ifdef WIN32
#if defined(__MINGW32__) || defined(MINGW32)
	typedef struct _FILE_ALLOCATED_RANGE_BUFFER {
		LARGE_INTEGER FileOffset;
		LARGE_INTEGER Length;
	} FILE_ALLOCATED_RANGE_BUFFER, *PFILE_ALLOCATED_RANGE_BUFFER;
#define FSCTL_QUERY_ALLOCATED_RANGES ((0x9 << 16) | (1 << 14) | (51 << 2) | 3)
#endif
	FILE_ALLOCATED_RANGE_BUFFER buffer;
	DWORD bytes_returned = 0;
	FILE_ALLOCATED_RANGE_BUFFER in;
	boost::system::error_code ec;
	size_type file_size = get_size(ec);
	if (ec) return start;
	in.FileOffset.QuadPart = start;
	in.Length.QuadPart = file_size - start;
	if (!::DeviceIoControl(m_file_handle, FSCTL_QUERY_ALLOCATED_RANGES
		, &in, sizeof(FILE_ALLOCATED_RANGE_BUFFER)
		, &buffer, sizeof(FILE_ALLOCATED_RANGE_BUFFER), &bytes_returned, 0))
	{
		int err = ::GetLastError();
		if (err != ERROR_INSUFFICIENT_BUFFER) return start;
	}

	// if there are no allocated regions within the rest
	// of the file, return the end of the file
	if (bytes_returned == 0) return file_size;

	// assume that this range overlaps the start of the
	// region we were interested in, and that start actually
	// resides in an allocated region.
	if (buffer.FileOffset.QuadPart < start) return start;

	// return the offset to the next allocated region
	return buffer.FileOffset.QuadPart;

#elif defined SEEK_DATA
	// this is supported on solaris
	size_type ret = lseek(m_fd, start, SEEK_DATA);
	if (ret < 0) return start;
	return ret;
#else
	return start;
#endif
}

} // namespace avhttp

#endif // AVHTTP_FILE_IPP
