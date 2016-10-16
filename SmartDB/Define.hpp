#ifndef _DEFINE_H
#define _DEFINE_H

#include <string>
#include "Variant.hpp"
#include "sqlite3/sqlite3.h"

const std::string Begin = "begin";
const std::string Commit = "commit";
const std::string Rollback = "rollback";

namespace smartdb
{

struct Blob
{
    const char* buf = nullptr;
    std::size_t size = 0;
};

using DBVariant = 
Variant<int, uint32_t, double, sqlite3_int64, char*, const char*, std::string, Blob, std::nullptr_t>;

}

#endif
