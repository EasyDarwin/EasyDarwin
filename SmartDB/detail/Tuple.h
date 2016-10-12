#pragma once
namespace detail
{
	template<int...>
	struct IndexTuple{};

	template<int N, int... Indexes>
	struct MakeIndexes : MakeIndexes<N - 1, N - 1, Indexes...>{};

	template<int... indexes>
	struct MakeIndexes<0, indexes...>
	{
		typedef IndexTuple<indexes...> type;
	};

	template<int... Indexes, class Tuple>
	int ExcecuteTuple(sqlite3_stmt *stmt, IndexTuple< Indexes... >&& in, Tuple&& t)
	{
		if (SQLITE_OK != detail::BindParams(stmt, 1, get<Indexes>(std::forward<Tuple>(t))...))
		{
			return false;
		}

		int code = sqlite3_step(stmt);
		sqlite3_reset(stmt);
		return code;
	}
}

