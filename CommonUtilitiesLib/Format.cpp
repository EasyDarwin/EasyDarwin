#include "Format.h"

#include <sstream>

std::size_t parseIndex(std::string::const_iterator& itFmt, const std::string::const_iterator& endFmt)
{
	int index = 0;
	while (itFmt != endFmt && isdigit(*itFmt))
	{
		index = 10 * index + *itFmt - '0';
		++itFmt;
	}
	if (itFmt != endFmt && *itFmt == ']') ++itFmt;
	return index;
}

void writeAnyInt(std::ostream& str, const boost::any& any)
{
	if (any.type() == typeid(char))
		str << static_cast<int>(boost::any_cast<char>(any));
	else if (any.type() == typeid(signed char))
		str << static_cast<int>(boost::any_cast<signed char>(any));
	else if (any.type() == typeid(unsigned char))
		str << static_cast<unsigned>(boost::any_cast<unsigned char>(any));
	else if (any.type() == typeid(short))
		str << boost::any_cast<short>(any);
	else if (any.type() == typeid(unsigned short))
		str << boost::any_cast<unsigned short>(any);
	else if (any.type() == typeid(int))
		str << boost::any_cast<int>(any);
	else if (any.type() == typeid(unsigned int))
		str << boost::any_cast<unsigned int>(any);
	else if (any.type() == typeid(long))
		str << boost::any_cast<long>(any);
	else if (any.type() == typeid(unsigned long))
		str << boost::any_cast<unsigned long>(any);
	else if (any.type() == typeid(long long))
		str << boost::any_cast<long long>(any);
	else if (any.type() == typeid(unsigned long long))
		str << boost::any_cast<unsigned long long>(any);
	else if (any.type() == typeid(bool))
		str << boost::any_cast<bool>(any);
}

void parseFlags(std::ostream& str, std::string::const_iterator& itFmt, const std::string::const_iterator& endFmt)
{
	bool isFlag = true;
	while (isFlag && itFmt != endFmt)
	{
		switch (*itFmt)
		{
		case '-': str.setf(std::ios::left); ++itFmt; break;
		case '+': str.setf(std::ios::showpos); ++itFmt; break;
		case '0': str.fill('0'); str.setf(std::ios::internal); ++itFmt; break;
		case '#': str.setf(std::ios::showpoint | std::ios::showbase); ++itFmt; break;
		default:  isFlag = false; break;
		}
	}
}

void parseWidth(std::ostream& str, std::string::const_iterator& itFmt, const std::string::const_iterator& endFmt)
{
	int width = 0;
	while (itFmt != endFmt && isdigit(*itFmt))
	{
		width = 10 * width + *itFmt - '0';
		++itFmt;
	}
	if (width != 0) str.width(width);
}

void parsePrec(std::ostream& str, std::string::const_iterator& itFmt, const std::string::const_iterator& endFmt)
{
	if (itFmt != endFmt && *itFmt == '.')
	{
		++itFmt;
		int prec = 0;
		while (itFmt != endFmt && isdigit(*itFmt))
		{
			prec = 10 * prec + *itFmt - '0';
			++itFmt;
		}
		if (prec >= 0) str.precision(prec);
	}
}

char parseMod(std::string::const_iterator& itFmt, const std::string::const_iterator& endFmt)
{
	char mod = 0;
	if (itFmt != endFmt)
	{
		switch (*itFmt)
		{
		case 'l':
		case 'h':
		case 'L':
		case '?': mod = *itFmt++; break;
		}
	}
	return mod;
}

void prepareFormat(std::ostream& str, char type)
{
	switch (type)
	{
	case 'd':
	case 'i': str << std::dec; break;
	case 'o': str << std::oct; break;
	case 'x': str << std::hex; break;
	case 'X': str << std::hex << std::uppercase; break;
	case 'e': str << std::scientific; break;
	case 'E': str << std::scientific << std::uppercase; break;
	case 'f': str << std::fixed; break;
	}
}

void FormatOne(std::string& result, std::string::const_iterator& itFmt, const std::string::const_iterator& endFmt, std::vector<boost::any>::const_iterator& itVal)
{
	std::ostringstream str;
	str.imbue(std::locale::classic());
	try
	{
		parseFlags(str, itFmt, endFmt);
		parseWidth(str, itFmt, endFmt);
		parsePrec(str, itFmt, endFmt);
		char mod = parseMod(itFmt, endFmt);
		if (itFmt != endFmt)
		{
			char type = *itFmt++;
			prepareFormat(str, type);
			switch (type)
			{
			case 'b':
				str << boost::any_cast<bool>(*itVal++);
				break;
			case 'c':
				str << boost::any_cast<char>(*itVal++);
				break;
			case 'd':
			case 'i':
				switch (mod)
				{
				case 'l': str << boost::any_cast<long>(*itVal++); break;
				case 'L': str << boost::any_cast<long long>(*itVal++); break;
				case 'h': str << boost::any_cast<short>(*itVal++); break;
				case '?': writeAnyInt(str, *itVal++); break;
				default:  str << boost::any_cast<int>(*itVal++); break;
				}
				break;
			case 'o':
			case 'u':
			case 'x':
			case 'X':
				switch (mod)
				{
				case 'l': str << boost::any_cast<unsigned long>(*itVal++); break;
				case 'L': str << boost::any_cast<unsigned long long>(*itVal++); break;
				case 'h': str << boost::any_cast<unsigned short>(*itVal++); break;
				case '?': writeAnyInt(str, *itVal++); break;
				default:  str << boost::any_cast<unsigned>(*itVal++); break;
				}
				break;
			case 'e':
			case 'E':
			case 'f':
				switch (mod)
				{
				case 'l': str << boost::any_cast<long double>(*itVal++); break;
				case 'L': str << boost::any_cast<long double>(*itVal++); break;
				case 'h': str << boost::any_cast<float>(*itVal++); break;
				default:  str << boost::any_cast<double>(*itVal++); break;
				}
				break;
			case 's':
				str << boost::any_cast<std::string>(*itVal++);
				break;
			case 'z':
				str << boost::any_cast<std::size_t>(*itVal++);
				break;
			case 'I':
			case 'D':
			default:
				str << type;
			}
		}
	}
	catch (std::bad_cast&)
	{
		str << "[ERRFMT]";
	}
	result.append(str.str());
}

#if 0
std::string Format(const std::string& fmt, const boost::any& value)
{
	std::string result;
	Format(result, fmt, value);
	return result;
}


std::string Format(const std::string& fmt, const boost::any& value1, const boost::any& value2)
{
	std::string result;
	Format(result, fmt, value1, value2);
	return result;
}


std::string Format(const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3)
{
	std::string result;
	Format(result, fmt, value1, value2, value3);
	return result;
}


std::string Format(const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4)
{
	std::string result;
	Format(result, fmt, value1, value2, value3, value4);
	return result;
}


std::string Format(const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5)
{
	std::string result;
	Format(result, fmt, value1, value2, value3, value4, value5);
	return result;
}


std::string Format(const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5, const boost::any& value6)
{
	std::string result;
	Format(result, fmt, value1, value2, value3, value4, value5, value6);
	return result;
}


std::string Format(const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5, const boost::any& value6, const boost::any& value7)
{
	std::string result;
	Format(result, fmt, value1, value2, value3, value4, value5, value6, value7);
	return result;
}


std::string Format(const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5, const boost::any& value6, const boost::any& value7, const boost::any& value8)
{
	std::string result;
	Format(result, fmt, value1, value2, value3, value4, value5, value6, value7, value8);
	return result;
}


std::string Format(const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5, const boost::any& value6, const boost::any& value7, const boost::any& value8, const boost::any& value9)
{
	std::string result;
	Format(result, fmt, value1, value2, value3, value4, value5, value6, value7, value8, value9);
	return result;
}


std::string Format(const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5, const boost::any& value6, const boost::any& value7, const boost::any& value8, const boost::any& value9, const boost::any& value10)
{
	std::string result;
	Format(result, fmt, value1, value2, value3, value4, value5, value6, value7, value8, value9, value10);
	return result;
}

void Format(std::string& result, const std::string& fmt, const boost::any& value)
{
	std::vector<boost::any> args;
	args.push_back(value);
	Format(result, fmt, args);
}


void Format(std::string& result, const std::string& fmt, const boost::any& value1, const boost::any& value2)
{
	std::vector<boost::any> args;
	args.push_back(value1);
	args.push_back(value2);
	Format(result, fmt, args);
}


void Format(std::string& result, const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3)
{
	std::vector<boost::any> args;
	args.push_back(value1);
	args.push_back(value2);
	args.push_back(value3);
	Format(result, fmt, args);
}


void Format(std::string& result, const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4)
{
	std::vector<boost::any> args;
	args.push_back(value1);
	args.push_back(value2);
	args.push_back(value3);
	args.push_back(value4);
	Format(result, fmt, args);
}


void Format(std::string& result, const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5)
{
	std::vector<boost::any> args;
	args.push_back(value1);
	args.push_back(value2);
	args.push_back(value3);
	args.push_back(value4);
	args.push_back(value5);
	Format(result, fmt, args);
}


void Format(std::string& result, const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5, const boost::any& value6)
{
	std::vector<boost::any> args;
	args.push_back(value1);
	args.push_back(value2);
	args.push_back(value3);
	args.push_back(value4);
	args.push_back(value5);
	args.push_back(value6);
	Format(result, fmt, args);
}


void Format(std::string& result, const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5, const boost::any& value6, const boost::any& value7)
{
	std::vector<boost::any> args;
	args.push_back(value1);
	args.push_back(value2);
	args.push_back(value3);
	args.push_back(value4);
	args.push_back(value5);
	args.push_back(value6);
	args.push_back(value7);
	Format(result, fmt, args);
}


void Format(std::string& result, const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5, const boost::any& value6, const boost::any& value7, const boost::any& value8)
{
	std::vector<boost::any> args;
	args.push_back(value1);
	args.push_back(value2);
	args.push_back(value3);
	args.push_back(value4);
	args.push_back(value5);
	args.push_back(value6);
	args.push_back(value7);
	args.push_back(value8);
	Format(result, fmt, args);
}


void Format(std::string& result, const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5, const boost::any& value6, const boost::any& value7, const boost::any& value8, const boost::any& value9)
{
	std::vector<boost::any> args;
	args.push_back(value1);
	args.push_back(value2);
	args.push_back(value3);
	args.push_back(value4);
	args.push_back(value5);
	args.push_back(value6);
	args.push_back(value7);
	args.push_back(value8);
	args.push_back(value9);
	Format(result, fmt, args);
}


void Format(std::string& result, const std::string& fmt, const boost::any& value1, const boost::any& value2, const boost::any& value3, const boost::any& value4, const boost::any& value5, const boost::any& value6, const boost::any& value7, const boost::any& value8, const boost::any& value9, const boost::any& value10)
{
	std::vector<boost::any> args;
	args.push_back(value1);
	args.push_back(value2);
	args.push_back(value3);
	args.push_back(value4);
	args.push_back(value5);
	args.push_back(value6);
	args.push_back(value7);
	args.push_back(value8);
	args.push_back(value9);
	args.push_back(value10);
	Format(result, fmt, args);
}

#endif

void Format(std::string& result, const char *fmt, const std::vector<boost::any>& values)
{
	Format(result, std::string(fmt), values);
}

void Format(std::string& result, const std::string& fmt, const std::vector<boost::any>& values)
{
	std::string::const_iterator itFmt = fmt.begin();
	std::string::const_iterator endFmt = fmt.end();
	std::vector<boost::any>::const_iterator itVal = values.begin();
	std::vector<boost::any>::const_iterator endVal = values.end();
	while (itFmt != endFmt)
	{
		switch (*itFmt)
		{
		case '%':
			++itFmt;
			if (itFmt != endFmt && (itVal != endVal || *itFmt == '['))
			{
				if (*itFmt == '[')
				{
					++itFmt;
					std::size_t index = parseIndex(itFmt, endFmt);
					if (index < values.size())
					{
						std::vector<boost::any>::const_iterator it = values.begin() + index;
						FormatOne(result, itFmt, endFmt, it);
					}
					else throw std::invalid_argument("Format argument index out of range");
				}
				else
				{
					FormatOne(result, itFmt, endFmt, itVal);
				}
			}
			else if (itFmt != endFmt)
			{
				result += *itFmt++;
			}
			break;
		default:
			result += *itFmt;
			++itFmt;
		}
	}
}