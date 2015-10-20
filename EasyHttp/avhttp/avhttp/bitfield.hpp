//
// Copyright (c) 2003, Arvid Norberg All rights reserved.
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __BITFIELD_HPP__
#define __BITFIELD_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/assert.hpp>

namespace avhttp {

// 一个位图的类实现.
//

struct bitfield
{
public:
    bitfield(void) : m_bytes(0), m_size(0), m_own(false) {}
    bitfield(int bits): m_bytes(0), m_size(0)
    { resize(bits); }
    bitfield(int bits, bool val): m_bytes(0), m_size(0)
    { resize(bits, val); }
    bitfield(char const* b, int bits): m_bytes(0), m_size(0)
    { assign(b, bits); }
    bitfield(bitfield const& rhs): m_bytes(0), m_size(0), m_own(false)
    { assign(rhs.bytes(), rhs.size()); }

    void borrow_bytes(char* b, int bits)
    {
        dealloc();
        m_bytes = (unsigned char*)b;
        m_size = bits;
        m_own = false;
    }

    ~bitfield() { dealloc(); }
    void assign(char const* b, int bits)
    { resize(bits); std::memcpy(m_bytes, b, (bits + 7) / 8); clear_trailing_bits(); }

    bool operator[](int index) const
    { return get_bit(index); }

    bool get_bit(int index) const
    {
        BOOST_ASSERT(index >= 0);
        BOOST_ASSERT(index < m_size);
        return (m_bytes[index / 8] & (0x80 >> (index & 7))) != 0;
    }

    void clear_bit(int index)
    {
        BOOST_ASSERT(index >= 0);
        BOOST_ASSERT(index < m_size);
        m_bytes[index / 8] &= ~(0x80 >> (index & 7));
    }

    void set_bit(int index)
    {
        BOOST_ASSERT(index >= 0);
        BOOST_ASSERT(index < m_size);
        m_bytes[index / 8] |= (0x80 >> (index & 7));
    }

    std::size_t bytes_size() const { return m_size / 8 + (m_size % 8 == 0 ? 0 : 1); }
    std::size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }

    char const* bytes() const { return (char*)m_bytes; }

    bitfield& operator=(bitfield const& rhs)
    {
        assign(rhs.bytes(), rhs.size());
        return *this;
    }

    int count() const
    {
        // 0000, 0001, 0010, 0011, 0100, 0101, 0110, 0111,
        // 1000, 1001, 1010, 1011, 1100, 1101, 1110, 1111
        const static char num_bits[] =
        {
            0, 1, 1, 2, 1, 2, 2, 3,
            1, 2, 2, 3, 2, 3, 3, 4
        };

        int ret = 0;
        const int num_bytes = m_size / 8;
        for (int i = 0; i < num_bytes; ++i)
        {
            ret += num_bits[m_bytes[i] & 0xf] + num_bits[m_bytes[i] >> 4];
        }

        int rest = m_size - num_bytes * 8;
        for (int i = 0; i < rest; ++i)
        {
            ret += (m_bytes[num_bytes] >> (7-i)) & 1;
        }
        BOOST_ASSERT(ret <= m_size);
        BOOST_ASSERT(ret >= 0);
        return ret;
    }

    struct const_iterator
    {
        friend struct bitfield;

        typedef bool value_type;
        typedef ptrdiff_t difference_type;
        typedef bool const* pointer;
        typedef bool& reference;
        typedef std::forward_iterator_tag iterator_category;

        bool operator*() { return (*byte & bit) != 0; }
        const_iterator& operator++() { inc(); return *this; }
        const_iterator operator++(int)
        { const_iterator ret(*this); inc(); return ret; }
        const_iterator& operator--() { dec(); return *this; }
        const_iterator operator--(int)
        { const_iterator ret(*this); dec(); return ret; }

        const_iterator(): byte(0), bit(0x80) {}
        bool operator==(const_iterator const& rhs) const
        { return byte == rhs.byte && bit == rhs.bit; }

        bool operator!=(const_iterator const& rhs) const
        { return byte != rhs.byte || bit != rhs.bit; }

        const_iterator& operator+(boost::uint64_t rhs)
        { for (int i = 0; i < rhs; i++)inc(); return *this; }

    private:
        void inc()
        {
            BOOST_ASSERT(byte);
            if (bit == 0x01)
            {
                bit = 0x80;
                ++byte;
            }
            else
            {
                bit >>= 1;
            }
        }
        void dec()
        {
            BOOST_ASSERT(byte);
            if (bit == 0x80)
            {
                bit = 0x01;
                --byte;
            }
            else
            {
                bit <<= 1;
            }
        }
        const_iterator(unsigned char const* ptr, int offset)
            : byte(ptr), bit(0x80 >> offset) {}
        unsigned char const* byte;
        int bit;
    };

    const_iterator begin() const { return const_iterator(m_bytes, 0); }
    const_iterator end() const { return const_iterator(m_bytes + m_size / 8, m_size & 7); }

    void resize(int bits, bool val)
    {
        int s = m_size;
        int b = m_size & 7;
        resize(bits);
        if (s >= m_size) return;
        int old_size_bytes = (s + 7) / 8;
        int new_size_bytes = (m_size + 7) / 8;
        if (val)
        {
            if (old_size_bytes && b) m_bytes[old_size_bytes - 1] |= (0xff >> b);
            if (old_size_bytes < new_size_bytes)
                std::memset(m_bytes + old_size_bytes, 0xff, new_size_bytes - old_size_bytes);
            clear_trailing_bits();
        }
        else
        {
            if (old_size_bytes < new_size_bytes)
                std::memset(m_bytes + old_size_bytes, 0x00, new_size_bytes - old_size_bytes);
        }
    }

    void set_all()
    {
        std::memset(m_bytes, 0xff, (m_size + 7) / 8);
        clear_trailing_bits();
    }

    void clear_all()
    {
        std::memset(m_bytes, 0x00, (m_size + 7) / 8);
    }

    void resize(int bits)
    {
        const int b = (bits + 7) / 8;
        if (m_bytes)
        {
            if (m_own)
            {
                m_bytes = (unsigned char*)std::realloc(m_bytes, b);
                m_own = true;
            }
            else if (bits > m_size)
            {
                unsigned char* tmp = (unsigned char*)std::malloc(b);
                std::memcpy(tmp, m_bytes, (std::min)(int(m_size + 7)/ 8, b));
                m_bytes = tmp;
                m_own = true;
            }
        }
        else
        {
            m_bytes = (unsigned char*)std::malloc(b);
            m_own = true;
        }
        m_size = bits;
        clear_trailing_bits();
    }

    void free() { dealloc(); m_size = 0; }

private:

    void clear_trailing_bits()
    {
        // clear the tail bits in the last byte.
        if (m_size & 7) m_bytes[(m_size + 7) / 8 - 1] &= 0xff << (8 - (m_size & 7));
    }

    void dealloc() { if (m_own) std::free(m_bytes); m_bytes = 0; }
    unsigned char* m_bytes;
    int m_size:31; // in bits.
    bool m_own:1;
};

} // namespace avhttp

#endif // __BITFIELD_HPP__

