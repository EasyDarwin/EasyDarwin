#ifndef _DEFINE_H
#define _DEFINE_H

#include <string>
#include <boost/variant.hpp>
#include "sqlite3/sqlite3.h"

const std::string begin_str = "begin";
const std::string commit_str = "commit";
const std::string rollback_str = "rollback";

namespace smartdb
{

struct db_blob
{
    const char* buf = nullptr;
    std::size_t size = 0;
};

using db_variant = 
boost::variant<int, uint32_t, double, sqlite3_int64, char*, const char*, std::string, db_blob, std::nullptr_t>;

}

#endif
