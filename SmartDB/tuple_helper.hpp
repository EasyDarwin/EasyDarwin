#ifndef _TUPLE_HELPER_H
#define _TUPLE_HELPER_H

#include <tuple>
#include "sqlite3/sqlite3.h"
#include "bind_parames.hpp"

namespace smartdb
{

template<typename Tuple, std::size_t N>
struct tuple_printer
{
    static int foreach(sqlite3_stmt* statement, const Tuple& t)
    {
        int code = tuple_printer<Tuple, N - 1>::foreach(statement, t);
        if (code != SQLITE_OK)
        {
            return code;
        }
        return bind_value(statement, N, std::get<N - 1>(t));
    }
};

template<typename Tuple>
struct tuple_printer<Tuple, 1>
{
    static int foreach(sqlite3_stmt* statement, const Tuple& t)
    {
        return bind_value(statement, 1, std::get<0>(t));
    }
};

template<typename... Args>
int add_bind_tuple(sqlite3_stmt* statement, const std::tuple<Args...>& t)
{
    return tuple_printer<decltype(t), sizeof...(Args)>::foreach(statement, t);
}

}

#endif
