#ifndef _DATABASE_H
#define _DATABASE_H

#include <assert.h>
#include <string>
#include <memory>
#include <tuple>
#include <functional>
#include <unordered_map>
#include <type_traits>
#include "traits.hpp"
#include "bind_parames.hpp"
#include "tuple_helper.hpp"
#include "db_query.hpp"

namespace smartdb
{

class database
{
public:
    database(const database&) = delete;
    database& operator=(const database&) = delete;
    database() : _query(_buf, _code) {}
    ~database()
    {
        close();
    }

    bool open(const std::string& databaseName)
    {
        _code = sqlite3_open(databaseName.c_str(), &_db_handle);
        return _code == SQLITE_OK;
    }

    bool close()
    {
        if (_db_handle == nullptr)
        {
            return true;
        }

        sqlite3_finalize(_statement);
        _code = close_db_handle();
        _statement = nullptr;
        _db_handle = nullptr;
        return _code == SQLITE_OK;
    }

    bool execute(const std::string& sql)
    {
        if (!prepare(sql))
        {
            return false;
        }

        if (!execute())
        {
            return false;
        }
        return try_select();
    }

    template<typename... Args>
    bool execute(const std::string& sql, Args&&... args)
    {
        if (!prepare(sql))
        {
            return false;
        }

        if (!add_bind_value(std::forward<Args>(args)...))
        {
            return false;
        }
        return try_select();
    }

    template<typename Tuple>
    typename std::enable_if<is_tuple<Tuple>::value, bool>::type execute(const std::string& sql, Tuple&& t)
    {
        if (!prepare(sql))        
        {
            return false;
        }

        if (!add_bind_value(std::forward<Tuple>(t)))
        {
            return false;
        }
        return try_select();
    }

    bool try_select()
    {
        if (_query.is_select(_statement))
        {
            if (!_query.read_table(_statement))
            {
                return false;
            }
            move_first();
        }
        return true;
    }

    bool prepare(const std::string& sql)
    {
        _code = sqlite3_prepare_v2(_db_handle, sql.c_str(), -1, &_statement, nullptr);
        return _code == SQLITE_OK;
    }

    template<typename... Args>
    bool add_bind_value(Args&&... args)
    {
        _code = bind_params(_statement, 1, std::forward<Args>(args)...);
        if (_code != SQLITE_OK)
        {
            return false;
        }
        return execute();
    }

    template<typename Tuple>
    typename std::enable_if<is_tuple<Tuple>::value, bool>::type add_bind_value(Tuple&& t)
    {
        _code = add_bind_tuple(_statement, std::forward<Tuple>(t)); 
        if (_code != SQLITE_OK)
        {
            return false;
        }
        return execute();
    }

    template<typename T>
    T& get(int index)
    {
        return boost::get<T>((*_iter)[index]);
    }

    void move_first()
    {
        _iter = _buf.begin();
    }

    void move_next()
    {
        if (!is_end())
        {
            ++_iter;
        }
    }

    bool is_end()
    {
        if (_iter == _buf.end())
        {
            _buf.clear();
            move_first();
            return true;
        }
        return false;
    }

    std::size_t record_count()
    {
        return _buf.size();
    }

    bool begin()
    {
        _code = sqlite3_exec(_db_handle, begin_str.c_str(), nullptr, nullptr, nullptr);
        return _code == SQLITE_OK;
    }

    bool commit()
    {
        _code = sqlite3_exec(_db_handle, commit_str.c_str(), nullptr, nullptr, nullptr);
        return _code == SQLITE_OK;
    }

    bool rollback()
    {
        _code = sqlite3_exec(_db_handle, rollback_str.c_str(), nullptr, nullptr, nullptr);
        return _code == SQLITE_OK;
    }

    int affected_rows()
    {
        return sqlite3_changes(_db_handle);
    }

    int get_error_code() const
    {
        return _code; 
    }

    const char* get_error_string() const
    {
        return sqlite3_errstr(_code);
    }

    const char* get_error_message() const
    {
        return sqlite3_errmsg(_db_handle);
    }

private:
    int close_db_handle()
    {
        int ret = sqlite3_close(_db_handle);
        while (ret == SQLITE_BUSY)
        {
            sqlite3_stmt* stmt = sqlite3_next_stmt(_db_handle, nullptr);
            if (stmt == nullptr)
            {
                break;
            }

            ret = sqlite3_finalize(stmt);
            if (ret == SQLITE_OK)
            {
                ret = sqlite3_close(_db_handle);
            }
        }

        return ret;
    }

    bool execute()
    {
        _code = sqlite3_step(_statement);
        sqlite3_reset(_statement);
        // 当执行insert、update、drop等操作时成功返回SQLITE_DONE.
        // 当执行select操作时，成功返回SQLITE_ROW.
        return (_code == SQLITE_DONE || _code == SQLITE_ROW || _code == SQLITE_OK);
    }

private:
    sqlite3* _db_handle = nullptr;
    sqlite3_stmt* _statement = nullptr;
    int _code = 0;
    std::vector<std::vector<db_variant>> _buf;
    std::vector<std::vector<db_variant>>::iterator _iter = _buf.end();
    db_query _query;
};

}

#endif
