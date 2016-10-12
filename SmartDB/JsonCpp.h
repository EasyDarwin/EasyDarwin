#pragma once
/*
*文件名称：JsonCpp.hpp
*文件标识：
*摘要：json序列化类，将结构体序列化成字符串；
 结构体必须提供字段名和字段值信息；
 结构体中不能有指针类型，可以是char定长数组；
 结构体中可以嵌套结构体，但是嵌套的结构体中不能有指针

*当前版本：1.0.0
*作者：祁宇
*完成日期：2013年10月31日
*/
#include <string>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
using namespace rapidjson;

class JsonCpp
{
	typedef Writer<StringBuffer> JsonWriter;
public:

	JsonCpp() : m_writer(m_buf)
	{
	}

	~JsonCpp()
	{
	}

	/**
	* 序列化结构体数组之前需调用此接口，然后再循环去Serialize
	*/
	void StartArray()
	{
		m_writer.StartArray();
	}

	/**
	* 序列化结构体数组之后需调用此接口，循环Serialize完成之后调用
	*/
	void EndArray()
	{
		m_writer.EndArray();
	}

	void StartObject()
	{
		m_writer.StartObject();
	}

	void EndObject()
	{
		m_writer.EndObject();
	}

	template<typename T>
	void WriteJson(string& key, T&& value)
	{
		m_writer.String(key.c_str());
		WriteValue(std::forward<T>(value));
	}

	template<typename T>
	void WriteJson(const char* key, T&& value)
	{
		m_writer.String(key);
		WriteValue(std::forward<T>(value));	
	}
	

	///**
	//* 返回对象序列化后端json字符串
	//*/
	const char* GetString() const
	{
		return m_buf.GetString();
	}

private:
	template<typename V>
	typename std::enable_if<std::is_same<V, int>::value>::type WriteValue(V value)
	{
		m_writer.Int(value);
	}

	template<typename V>
	typename std::enable_if<std::is_same<V, unsigned int>::value>::type WriteValue(V value)
	{
		m_writer.Uint(value);
	}

	template<typename V>
	typename std::enable_if<std::is_same<V, int64_t>::value>::type WriteValue(V value)
	{
		m_writer.Int64(value);
	}

	template<typename V>
	typename std::enable_if<std::is_floating_point<V>::value>::type WriteValue(V value)
	{
		m_writer.Double(value);
	}

	template<typename V>
	typename std::enable_if<std::is_same<V, bool>::value>::type WriteValue(V value)
	{
		m_writer.Bool(value);
	}

	template<typename V>
	typename std::enable_if<std::is_pointer<V>::value>::type WriteValue(V value)
	{
		m_writer.String(value);
	}

	template<typename V>
	typename std::enable_if<std::is_array<V>::value>::type WriteValue(V value)
	{
		m_writer.String(value);
	}

	template<typename V>
	typename std::enable_if<std::is_same<V, std::nullptr_t>::value>::type WriteValue(V value)
	{
		m_writer.Null();
	}

private:
	StringBuffer m_buf; //json字符串的buf
	JsonWriter m_writer; //json写入器

	Document m_doc;
};

