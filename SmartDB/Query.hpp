#ifndef _QUERY_H
#define _QUERY_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include "Define.hpp"

namespace smartdb
{

class Query
{
public:
    Query() = default;
    Query(std::vector<std::vector<DBVariant>>& buf, int& code) : m_buf(buf), m_code(code) {}

    bool isSelect(sqlite3_stmt* statement)
    {
        m_colCount = sqlite3_column_count(statement);
        return m_colCount == 0 ? false : true;
    }
    
    bool readTable(sqlite3_stmt* statement)
    {
        m_buf.clear();
        while (true)
        {
            m_code = sqlite3_step(statement);
            if (m_code == SQLITE_DONE)
            {
                break;
            }

            if (!readRow(statement))
            {
                sqlite3_reset(statement);
                m_buf.clear();
                return false;
            }
        }

        sqlite3_reset(statement);
        return true;
    }

private:
    bool readRow(sqlite3_stmt* statement)
    {
        std::vector<DBVariant> rowBuf;
        rowBuf.reserve(m_colCount);
        for (int i = 0; i < m_colCount; ++i)
        {
            if (!readValue(statement, i, rowBuf))
            {
                return false;
            }
        }

        m_buf.emplace_back(std::move(rowBuf));
        return true;
    }

    bool readValue(sqlite3_stmt* statement, int index, std::vector<DBVariant>& buf)
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
    std::vector<std::vector<DBVariant>>& m_buf;
    int& m_code;
    int m_colCount = 0;
};

}
#endif
