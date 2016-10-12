#pragma once
namespace detail
{
	template <typename T>
	typename std::enable_if<std::is_floating_point<T>::value, int>::type
		BindValue(sqlite3_stmt *statement, int current, T t)
	{
			return sqlite3_bind_double(statement, current, std::forward<T>(t));
		}

	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, int>::type
		BindValue(sqlite3_stmt *statement, int current, T t)
	{
			return BindIntValue(statement, current, t);
		}

	template <typename T>
	typename std::enable_if<std::is_same<T, int64_t>::value || std::is_same<T, uint64_t>::value, int>::type
		BindIntValue(sqlite3_stmt *statement, int current, T t)
	{
			return sqlite3_bind_int64(statement, current, std::forward<T>(t));
		}

	template <typename T>
	typename std::enable_if<!std::is_same<T, int64_t>::value&&!std::is_same<T, uint64_t>::value, int>::type
		BindIntValue(sqlite3_stmt *statement, int current, T t)
	{
			return sqlite3_bind_int(statement, current, std::forward<T>(t));
		}


	template <typename T>
	typename std::enable_if<std::is_same<std::string, T>::value, int>::type
		BindValue(sqlite3_stmt *statement, int current, const T& t)
	{
			return sqlite3_bind_text(statement, current, t.data(), t.length(), SQLITE_TRANSIENT);
		}

	template <typename T>
	typename std::enable_if<std::is_same<char*, T>::value || std::is_same<const char*, T>::value, int>::type
		BindValue(sqlite3_stmt *statement, int current, T t)
	{
			return sqlite3_bind_text(statement, current, t, strlen(t) + 1, SQLITE_TRANSIENT);
		}

	template <typename T>
	typename std::enable_if<std::is_same<blob, T>::value, int>::type
		BindValue(sqlite3_stmt *statement, int current, const T& t)
	{
			return sqlite3_bind_blob(statement, current, t.pBuf, t.size, SQLITE_TRANSIENT);
		}

	template <typename T>
	typename std::enable_if<std::is_same<nullptr_t, T>::value, int>::type
		BindValue(sqlite3_stmt *statement, int current, const T& t)
	{
			return sqlite3_bind_null(statement, current);
		}

	template <typename T, typename... Args>
	inline int BindParams(sqlite3_stmt *statement, int current, T&&first, Args&&... args)
	{
		int code = BindValue(statement, current, first);
		if (code != SQLITE_OK)
			return code;

		code = BindParams(statement, current + 1, std::forward<Args>(args)...);

		return code;
	}

	inline int BindParams(sqlite3_stmt *statement, int current)
	{
		return SQLITE_OK;
	}
}

