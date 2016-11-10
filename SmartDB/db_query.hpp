#ifndef _DB_QUERY_H
#define _DB_QUERY_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include "define.hpp"

namespace smartdb
{

class db_query
{
public:
    db_query() = default;
    db_query(std::vector<std::vector<db_variant>>& buf, int& code) : _buf(buf), _code(code) {}

    bool is_select(sqlite3_stmt* statement)
    {
        _col_count = sqlite3_column_count(statement);
        return _col_count == 0 ? false : true;
    }
    
    bool read_table(sqlite3_stmt* statement)
    {
        _buf.clear();
        while (true)
        {
            _code = sqlite3_step(statement);
            if (_code == SQLITE_DONE)
            {
                break;
            }

            if (!read_row(statement))
            {
                sqlite3_reset(statement);
                _buf.clear();
                return false;
            }
        }

        sqlite3_reset(statement);
        return true;
    }

private:
    bool read_row(sqlite3_stmt* statement)
    {
        std::vector<db_variant> rowBuf;
        rowBuf.reserve(_col_count);
        for (int i = 0; i < _col_count; ++i)
        {
            if (!read_value(statement, i, rowBuf))
            {
                return false;
            }
        }

        _buf.emplace_back(std::move(rowBuf));
        return true;
    }

    bool read_value(sqlite3_stmt* statement, int index, std::vector<db_variant>& buf)
    {
        int type = sqlite3_column_type(statement, index);
        switch (type)
        {
        case SQLITE_INTEGER: 
            {
                buf.emplace_back(sqlite3_column_int64(statement, index));
                break;
            }
        case SQLITE_FLOAT: 
            {
                buf.emplace_back(sqlite3_column_double(statement, index));
                break; 
            }
        case SQLITE_TEXT:
            {
                buf.emplace_back(std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, index))));
                break; 
            }
        case SQLITE_BLOB: 
            {
                buf.emplace_back(std::string(reinterpret_cast<const char*>(sqlite3_column_blob(statement, index)), 
                                             sqlite3_column_bytes(statement, index)));
                break;
            }
        case SQLITE_NULL:
            {
                buf.emplace_back(std::string(""));
                break; 
            }
        default: 
            {
                std::cout << "Invaild type: " << type << std::endl;
                return false;
            }
        }

        return true;
    }

private:
    std::vector<std::vector<db_variant>>& _buf;
    int& _code;
    int _col_count = 0;
};

}
#endif
