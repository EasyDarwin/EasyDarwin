//
// entry.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003, Arvid Norberg All rights reserved.
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __ENTRY_HPP__
#define __ENTRY_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/error_codec.hpp"
#include "detail/escape_string.hpp"

namespace avhttp {

namespace detail
{
	template<int v1, int v2>
	struct max2 { enum { value = v1 > v2 ? v1 : v2 }; };

	template<int v1, int v2, int v3>
	struct max3
	{
		enum
		{
			temp = max2<v1, v2>::value,
			value = temp > v3 ? temp : v3
		};
	};

	template<int v1, int v2, int v3, int v4>
	struct max4
	{
		enum
		{
			temp = max3<v1, v2, v3>::value,
			value = temp > v4 ? temp : v4
		};
	};
}

namespace
{
	template <class T>
	void call_destructor(T* o)
	{
		BOOST_ASSERT(o);
		o->~T();
	}
}

inline void throw_type_error(std::string str = "")
{
	if (str.empty())
		boost::throw_exception(boost::system::system_error(avhttp::errc::invalid_entry_type));
	boost::throw_exception(std::runtime_error(str.c_str()));
}

class entry;

class entry
{
public:
	// the key is always a string. If a generic entry would be allowed
	// as a key, sorting would become a problem (e.g. to compare a string
	// to a list). The definition doesn't mention such a limit though.
	typedef std::map<std::string, entry> dictionary_type;
	typedef std::string string_type;
	typedef std::list<entry> list_type;
	typedef boost::int64_t size_type;
	typedef boost::uint64_t unsigned_size_type;
	typedef size_type integer_type;

	enum data_type
	{
		int_t,
		string_t,
		list_t,
		dictionary_t,
		undefined_t
	};

	data_type type() const
	{
		return m_type;
	}

	entry(dictionary_type const& v)
		: m_type(undefined_t)
	{
		new(m_data) dictionary_type(v);
		m_type = dictionary_t;
	}

	entry(string_type const& v)
		: m_type(undefined_t)
	{
		new(m_data) string_type(v);
		m_type = string_t;
	}

	entry(list_type const& v)
		: m_type(undefined_t)
	{
		new(m_data) list_type(v);
		m_type = list_t;
	}

	entry(integer_type const& v)
		: m_type(undefined_t)
	{
		new(m_data) integer_type(v);
		m_type = int_t;
	}

	entry(void)
		: m_type(undefined_t)
	{}

	entry(data_type t)
		: m_type(undefined_t)
	{
		construct(t);
	}

	entry(entry const& e)
		: m_type(undefined_t)
	{
		copy(e);
	}

	~entry(void) { destruct(); }

	bool operator==(entry const& e) const
	{
		if (m_type != e.m_type) return false;

		switch(m_type)
		{
		case int_t:
			return integer() == e.integer();
		case string_t:
			return string() == e.string();
		case list_t:
			return list() == e.list();
		case dictionary_t:
			return dict() == e.dict();
		default:
			BOOST_ASSERT(m_type == undefined_t);
			return true;
		}
	}

	// void operator=(lazy_entry const&);
	void operator=(entry const& e)
	{
		destruct();
		copy(e);
	}

	void operator=(dictionary_type const& v)
	{
		destruct();
		new(m_data) dictionary_type(v);
		m_type = dictionary_t;
	}

	void operator=(string_type const& v)
	{
		destruct();
		new(m_data) string_type(v);
		m_type = string_t;
	}

	void operator=(list_type const& v)
	{
		destruct();
		new(m_data) list_type(v);
		m_type = list_t;
	}

	void operator=(integer_type const& v)
	{
		destruct();
		new(m_data) integer_type(v);
		m_type = int_t;
	}

	integer_type& integer()
	{
		if (m_type == undefined_t) construct(int_t);
#ifndef BOOST_NO_EXCEPTIONS
		if (m_type != int_t) throw_type_error();
#endif
		BOOST_ASSERT(m_type == int_t);
		return *reinterpret_cast<integer_type*>(m_data);
	}

	const integer_type& integer() const
	{
#ifndef BOOST_NO_EXCEPTIONS
		if (m_type != int_t) throw_type_error();
#endif
		BOOST_ASSERT(m_type == int_t);
		return *reinterpret_cast<const integer_type*>(m_data);
	}

	string_type& string()
	{
		if (m_type == undefined_t) construct(string_t);
#ifndef BOOST_NO_EXCEPTIONS
		if (m_type != string_t) throw_type_error();
#endif
		BOOST_ASSERT(m_type == string_t);
		return *reinterpret_cast<string_type*>(m_data);
	}

	const string_type& string() const
	{
#ifndef BOOST_NO_EXCEPTIONS
		if (m_type != string_t) throw_type_error();
#endif
		BOOST_ASSERT(m_type == string_t);
		return *reinterpret_cast<const string_type*>(m_data);
	}

	list_type& list()
	{
		if (m_type == undefined_t) construct(list_t);
#ifndef BOOST_NO_EXCEPTIONS
		if (m_type != list_t) throw_type_error();
#endif
		BOOST_ASSERT(m_type == list_t);
		return *reinterpret_cast<list_type*>(m_data);
	}

	const list_type& list() const
	{
#ifndef BOOST_NO_EXCEPTIONS
		if (m_type != list_t) throw_type_error();
#endif
		BOOST_ASSERT(m_type == list_t);
		return *reinterpret_cast<const list_type*>(m_data);
	}

	dictionary_type& dict()
	{
		if (m_type == undefined_t) construct(dictionary_t);
#ifndef BOOST_NO_EXCEPTIONS
		if (m_type != dictionary_t) throw_type_error();
#endif
		BOOST_ASSERT(m_type == dictionary_t);
		return *reinterpret_cast<dictionary_type*>(m_data);
	}

	const dictionary_type& dict() const
	{
#ifndef BOOST_NO_EXCEPTIONS
		if (m_type != dictionary_t) throw_type_error();
#endif
		BOOST_ASSERT(m_type == dictionary_t);
		return *reinterpret_cast<const dictionary_type*>(m_data);
	}

	void swap(entry& e)
	{
		// not implemented
		BOOST_ASSERT(false);
	}

	// these functions requires that the entry
	// is a dictionary, otherwise they will throw	
	entry& operator[](char const* key)
	{
		dictionary_type::iterator i = dict().find(key);
		if (i != dict().end()) return i->second;
		dictionary_type::iterator ret = dict().insert(
			std::pair<const std::string, entry>(key, entry())).first;
		return ret->second;
	}

	entry& operator[](std::string const& key)
	{
		dictionary_type::iterator i = dict().find(key);
		if (i != dict().end()) return i->second;
		dictionary_type::iterator ret = dict().insert(
			std::make_pair(key, entry())).first;
		return ret->second;
	}

#ifndef BOOST_NO_EXCEPTIONS
	const entry& operator[](char const* key) const
	{
		dictionary_type::const_iterator i = dict().find(key);
		if (i == dict().end()) throw_type_error(
			(std::string("key not found: ") + key).c_str());
		return i->second;
	}

	const entry& operator[](std::string const& key) const
	{
		return (*this)[key.c_str()];
	}
#endif

	entry* find_key(char const* key)
	{
		dictionary_type::iterator i = dict().find(key);
		if (i == dict().end()) return 0;
		return &i->second;
	}

	entry const* find_key(char const* key) const
	{
		dictionary_type::const_iterator i = dict().find(key);
		if (i == dict().end()) return 0;
		return &i->second;
	}

	entry* find_key(std::string const& key)
	{
		dictionary_type::iterator i = dict().find(key);
		if (i == dict().end()) return 0;
		return &i->second;
	}

	entry const* find_key(std::string const& key) const
	{
		dictionary_type::const_iterator i = dict().find(key);
		if (i == dict().end()) return 0;
		return &i->second;
	}

#ifdef _DEBUG
	void print(std::ostream& os, int indent = 0) const
	{
		BOOST_ASSERT(indent >= 0);
		for (int i = 0; i < indent; ++i) os << " ";
		switch (m_type)
		{
		case int_t:
			os << integer() << "\n";
			break;
		case string_t:
			{
				bool binary_string = false;
				for (std::string::const_iterator i = string().begin(); i != string().end(); ++i)
				{
					if (!detail::is_print(static_cast<unsigned char>(*i)))
					{
						binary_string = true;
						break;
					}
				}
				if (binary_string) os << detail::to_hex(string()) << "\n";
				else os << string() << "\n";
			} break;
		case list_t:
			{
				os << "list\n";
				for (list_type::const_iterator i = list().begin(); i != list().end(); ++i)
				{
					i->print(os, indent+1);
				}
			} break;
		case dictionary_t:
			{
				os << "dictionary\n";
				for (dictionary_type::const_iterator i = dict().begin(); i != dict().end(); ++i)
				{
					bool binary_string = false;
					for (std::string::const_iterator k = i->first.begin(); k != i->first.end(); ++k)
					{
						if (!detail::is_print(static_cast<unsigned char>(*k)))
						{
							binary_string = true;
							break;
						}
					}
					for (int j = 0; j < indent+1; ++j) os << " ";
					os << "[";
					if (binary_string) os << detail::to_hex(i->first);
					else os << i->first;
					os << "]";

					if (i->second.type() != entry::string_t
						&& i->second.type() != entry::int_t)
						os << "\n";
					else os << " ";
					i->second.print(os, indent+2);
				}
			} break;
		default:
			os << "<uninitialized>\n";
		}
	}
#endif

protected:

	void construct(data_type t)
	{
		switch(t)
		{
		case int_t:
			new(m_data) integer_type;
			break;
		case string_t:
			new(m_data) string_type;
			break;
		case list_t:
			new(m_data) list_type;
			break;
		case dictionary_t:
			new (m_data) dictionary_type;
			break;
		default:
			BOOST_ASSERT(t == undefined_t);
		}
		m_type = t;
	}

	void copy(const entry& e)
	{
		switch (e.type())
		{
		case int_t:
			new(m_data) integer_type(e.integer());
			break;
		case string_t:
			new(m_data) string_type(e.string());
			break;
		case list_t:
			new(m_data) list_type(e.list());
			break;
		case dictionary_t:
			new (m_data) dictionary_type(e.dict());
			break;
		default:
			BOOST_ASSERT(e.type() == undefined_t);
		}
		m_type = e.type();
	}

	void destruct()
	{
		switch(m_type)
		{
		case int_t:
			call_destructor(reinterpret_cast<integer_type*>(m_data));
			break;
		case string_t:
			call_destructor(reinterpret_cast<string_type*>(m_data));
			break;
		case list_t:
			call_destructor(reinterpret_cast<list_type*>(m_data));
			break;
		case dictionary_t:
			call_destructor(reinterpret_cast<dictionary_type*>(m_data));
			break;
		default:
			BOOST_ASSERT(m_type == undefined_t);
			break;
		}
		m_type = undefined_t;
	}

private:
	// the bitfield is used so that the m_type_queried
	// field still fits, so that the ABI is the same for
	// debug builds and release builds. It appears to be
	// very hard to match debug builds with debug versions
	// of libtorrent
	data_type m_type:31;

#if (defined(_MSC_VER) && _MSC_VER < 1310)
	// workaround for msvc-bug.
	// assumes sizeof(map<string, char>) == sizeof(map<string, entry>)
	// and sizeof(list<char>) == sizeof(list<entry>)
	enum { union_size
		= detail::max4<sizeof(std::list<char>)
		, sizeof(std::map<std::string, char>)
		, sizeof(string_type)
		, sizeof(integer_type)>::value
	};
#else
	enum { union_size
		= detail::max4<sizeof(list_type)
		, sizeof(dictionary_type)
		, sizeof(string_type)
		, sizeof(integer_type)>::value
	};
#endif

	integer_type m_data[(union_size + sizeof(integer_type) - 1)
		/ sizeof(integer_type)];
};

} // namespace avhttp

#endif // __ENTRY_HPP__
