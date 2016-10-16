#ifndef _TUPLE_H
#define _TUPLE_H

#include <tuple>
#include "sqlite3/sqlite3.h"
#include "BindParames.hpp"

namespace smartdb
{

template<typename Tuple, std::size_t N>
struct TuplePrinter
{
    static int foreach(sqlite3_stmt* statement, const Tuple& t)
    {
        int code = TuplePrinter<Tuple, N - 1>::foreach(statement, t);
        if (code != SQLITE_OK)
        {
            return code;
        }
        return bindValue(statement, N, std::get<N - 1>(t));
    }
};

template<typename Tuple>
struct TuplePrinter<Tuple, 1>
{
    static int foreach(sqlite3_stmt* statement, const Tuple& t)
    {
        return bindValue(statement, 1, std::get<0>(t));
    }
};

template<typename... Args>
int addBindTuple(sqlite3_stmt* statement, const std::tuple<Args...>& t)
{
    return TuplePrinter<decltype(t), sizeof...(Args)>::foreach(statement, t);
}

}

#endif
