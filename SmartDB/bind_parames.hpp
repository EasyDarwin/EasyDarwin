#ifndef _BIND_PARAMES_H
#define _BIND_PARAMES_H

#include <assert.h>
#include <string.h>
#include <iostream>
#include <type_traits>
#include "sqlite3/sqlite3.h"
#include "define.hpp"

namespace smartdb
{

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, int>::type 
bind_value(sqlite3_stmt* statement, int index, T t)
{
    return sqlite3_bind_double(statement, index, std::forward<T>(t));
}

template<typename T>
typename std::enable_if<std::is_same<T, int64_t>::value || std::is_same<T, uint64_t>::value, int>::type
bind_int_value(sqlite3_stmt* statement, int index, T t)
{
    return sqlite3_bind_int64(statement, index, std::forward<T>(t));
}

template<typename T>
typename std::enable_if<!std::is_same<T, int64_t>::value && !std::is_same<T, uint64_t>::value, int>::type
bind_int_value(sqlite3_stmt* statement, int index, T t)
{
    return sqlite3_bind_int(statement, index, std::forward<T>(t));
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value, int>::type
bind_value(sqlite3_stmt* statement, int index, T t)
{
    return bind_int_value(statement, index, t);
}

template<typename T>
typename std::enable_if<std::is_same<std::string, T>::value, int>::type
bind_value(sqlite3_stmt* statement, int index, const T& t)
{
    return sqlite3_bind_text(statement, index, t.c_str(), t.length(), SQLITE_TRANSIENT);
}

template<typename T>
typename std::enable_if<std::is_same<char*, T>::value || std::is_same<const char*, T>::value, int>::type
bind_value(sqlite3_stmt* statement, int index, T t)
{
    assert(t);
    return sqlite3_bind_text(statement, index, t, strlen(t) + 1, SQLITE_TRANSIENT);
}

template<typename T>
typename std::enable_if<std::is_same<db_blob, T>::value, int>::type
bind_value(sqlite3_stmt* statement, int index, const T& t)
{
    assert(t.buf);
    return sqlite3_bind_blob(statement, index, t.buf, t.size, SQLITE_TRANSIENT);
}

template<typename T>
typename std::enable_if<std::is_same<std::nullptr_t, T>::value, int>::type
bind_value(sqlite3_stmt* statement, int index, const T&)
{
    return sqlite3_bind_null(statement, index);
}

#if 1
inline int bind_params(sqlite3_stmt*, int)
{
    return SQLITE_OK;
}
#else
template<typename T>
inline int bind_params(sqlite3_stmt* statement, int index, T&& first)
{
    return bind_value(statement, index, first);
}
#endif

template<typename T, typename... Args>
inline int bind_params(sqlite3_stmt* statement, int index, T&& first, Args&&... args)
{
    int code = bind_value(statement, index, first);
    if (code != SQLITE_OK)
    {
        return code;
    }

    code = bind_params(statement, index + 1, std::forward<Args>(args)...);
    return code;
}

}

#endif
