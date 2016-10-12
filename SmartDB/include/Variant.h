#pragma once
#include <typeindex>
#include <iostream>
#include <type_traits>
using namespace std;

template <typename T>
struct Function_Traits
	: public Function_Traits<decltype(&T::operator())>
{};
// For generic types, directly use the result of the signature of its 'operator()'

template <typename ClassType, typename ReturnType, typename... Args>
struct Function_Traits<ReturnType(ClassType::*)(Args...) const>
	// we specialize for pointers to member function
{
	enum { arity = sizeof...(Args) };
	// arity is the number of arguments.

	typedef ReturnType result_type;

	template <size_t i>
	struct arg
	{
		typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
		// the i-th argument is equivalent to the i-th tuple element of a tuple
		// composed of those arguments.
	};

	typedef std::function<ReturnType(Args...)> FunType;
	typedef std::tuple<Args...> ArgTupleType;
};

//获取最大的整数
template <size_t arg, size_t... rest>
struct IntegerMax;

template <size_t arg>
struct IntegerMax<arg> : std::integral_constant<size_t, arg>
{
	//static const size_t value = arg;
	//enum{value = arg};
};

//获取最大的align
template <size_t arg1, size_t arg2, size_t... rest>
struct IntegerMax<arg1, arg2, rest...> : std::integral_constant<size_t, arg1 >= arg2 ? IntegerMax<arg1, rest...>::value :
	IntegerMax<arg2, rest...>::value >
{
	/*static const size_t value = arg1 >= arg2 ? static_max<arg1, others...>::value :
	static_max<arg2, others...>::value;*/
};

template<typename T, typename... Args>
struct MaxAlign : std::integral_constant<int,
	(std::alignment_of<T>::value >MaxAlign<Args...>::value ? std::alignment_of<T>::value : MaxAlign<Args...>::value) >
{};

template<typename T>
struct MaxAlign<T> : std::integral_constant<int, std::alignment_of<T>::value >{};

//是否包含某个类型
template < typename T, typename... List >
struct Contains : std::true_type {};

template < typename T, typename Head, typename... Rest >
struct Contains<T, Head, Rest...>
	: std::conditional< std::is_same<T, Head>::value, std::true_type, Contains<T,Rest... >> ::type{};

template < typename T >
struct Contains<T> : std::false_type{};

//获取第一个T的索引位置
// Forward
template<typename Type, typename... Types>
struct GetLeftSize;

// Declaration
template<typename Type, typename First, typename... Types>
struct GetLeftSize<Type, First, Types...> : GetLeftSize<Type, Types...>
{
};

// Specialized
template<typename Type, typename... Types>
struct GetLeftSize<Type, Type, Types...> : std::integral_constant<int, sizeof...(Types)>
{
	//static const int ID = sizeof...(Types);
};

template<typename Type>
struct GetLeftSize<Type> : std::integral_constant<int, -1>
{
	//static const int ID = -1;
};

template<typename T, typename... Types>
struct Index : std::integral_constant<int, sizeof...(Types) - GetLeftSize<T, Types...>::value - 1>{};

//根据索引获取索引位置的类型
// Forward declaration
template<int index, typename... Types>
struct IndexType;

// Declaration
template<int index, typename First, typename... Types>
struct IndexType<index, First, Types...> : IndexType<index - 1, Types...>
{
};

// Specialized
template<typename First, typename... Types>
struct IndexType<0, First, Types...>
{
	typedef First DataType;
};

template<typename... Args>
struct VariantHelper;

template<typename T, typename... Args>
struct VariantHelper<T, Args...> {
	inline static void Destroy(type_index id, void * data)
	{
		if (id == type_index(typeid(T)))
			//((T*) (data))->~T();
			reinterpret_cast<T*>(data)->~T();
		else
			VariantHelper<Args...>::Destroy(id, data);
	}

	inline static void move(type_index old_t, void * old_v, void * new_v)
	{
		if (old_t == type_index(typeid(T)))
			new (new_v) T(std::move(*reinterpret_cast<T*>(old_v)));
		else
			VariantHelper<Args...>::move(old_t, old_v, new_v);
	}

	inline static void copy(type_index old_t, const void * old_v, void * new_v)
	{
		if (old_t == type_index(typeid(T)))
			new (new_v) T(*reinterpret_cast<const T*>(old_v));
		else
			VariantHelper<Args...>::copy(old_t, old_v, new_v);
	}
};

template<> struct VariantHelper<>  {
	inline static void Destroy(type_index id, void * data) {  }
	inline static void move(type_index old_t, void * old_v, void * new_v) { }
	inline static void copy(type_index old_t, const void * old_v, void * new_v) { }
};

template<typename... Types>
class Variant
{
	typedef VariantHelper<Types...> Helper_t;

	enum
	{
		data_size = IntegerMax<sizeof(Types)...>::value,
		//align_size = IntegerMax<alignof(Types)...>::value
		align_size = MaxAlign<Types...>::value //ctp才有alignof, 为了兼容用此版本
	};
	using data_t = typename std::aligned_storage<data_size, align_size>::type;
public:
	template<int index>
	using IndexType = typename IndexType<index, Types...>::DataType;

	Variant(void) :m_typeIndex(typeid(void)), m_index(-1)
	{
	}

	~Variant()
	{
		Helper_t::Destroy(m_typeIndex, &m_data);
	}

	Variant(Variant<Types...>&& old) : m_typeIndex(old.m_typeIndex)
	{
		Helper_t::move(old.m_typeIndex, &old.m_data, &m_data);
	}

	Variant(const Variant<Types...>& old) : m_typeIndex(old.m_typeIndex)
	{
		Helper_t::copy(old.m_typeIndex, &old.m_data, &m_data);
	}

	template <class T,
	class = typename std::enable_if<Contains<typename std::remove_reference<T>::type, Types...>::value>::type>
		Variant(T&& value) : m_typeIndex(typeid(void))
	{
			Helper_t::Destroy(m_typeIndex, &m_data);
			typedef typename std::remove_reference<T>::type U;
			new(&m_data) U(std::forward<T>(value));
			m_typeIndex = type_index(typeid(T));
	}

	template<typename T>
	bool Is() const
	{
		return (m_typeIndex == type_index(typeid(T)));
	}

	bool Empty() const
	{
		return m_typeIndex == type_index(typeid(void));
	}

	type_index Type() const
	{
		return m_typeIndex;
	}

	template<typename T>
	T& Get()
	{
		if (!Is<T>())
		{
			cout << typeid(T).name() << " is not defined. " << "current type is " <<
				m_typeIndex.name() << endl;
			throw std::bad_cast();
		}

		return *(T*) (&m_data);
	}

	template<typename T>
	int GetIndexOf()
	{
		return Index<T, Types...>::value;
	}

	template<typename F>
	void Visit(F&& f)
	{
		using T = typename Function_Traits<F>::arg<0>::type;
		if (Is<T>())
			f(Get<T>());
	}

	template<typename F, typename... Rest>
	void Visit(F&& f, Rest&&... rest)
	{
		using T = typename Function_Traits<F>::arg<0>::type;
		if (Is<T>())
			Visit(std::forward<F>(f));
		else 
			Visit(std::forward<Rest>(rest)...);
	}

	bool operator==(const Variant& rhs) const
	{
		return m_typeIndex == rhs.m_typeIndex;
	}

	bool operator<(const Variant& rhs) const
	{
		return m_typeIndex < rhs.m_typeIndex;
	}

private:
	data_t m_data;
	std::type_index m_typeIndex;//类型ID
};

