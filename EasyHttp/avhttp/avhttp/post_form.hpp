#ifndef AVHTTP_POST_FORM_HPP
#define AVHTTP_POST_FORM_HPP
#include "avhttp/http_stream.hpp"
#include <boost/format.hpp>
#include <sstream>

namespace avhttp {

	typedef std::map<std::string, std::string> KeyValues;

	AVHTTP_DECL std::string map_to_query(const KeyValues& key_values)
	{
		if(key_values.size() == 0)
		{
			return std::string();
		}

		std::stringstream ss;
		for(KeyValues::const_iterator iter = key_values.begin();
				iter != key_values.end();
				++iter)
		{
			ss << iter->first << "=" << iter->second << "&";
		}

		const std::string& temp = ss.str();
		return std::string(temp.begin(), temp.begin() + temp.size() - 1);
	}

	AVHTTP_DECL void post_form(http_stream& stream, const KeyValues& key_values)
	{
		const std::string& body = map_to_query(key_values);
		request_opts opts = stream.request_options();
		opts.remove(http_options::request_method);
		opts.remove(http_options::request_body);
		opts.remove(http_options::content_type);
		opts.remove(http_options::content_length);
		opts.insert(http_options::content_length, (boost::format("%1%") % body.size()).str());
		opts.insert(http_options::content_type, "application/x-www-form-urlencoded");
		opts.insert(http_options::request_body, body);
		opts.insert(http_options::request_method, "POST");
		stream.request_options(opts);
	}
}

#endif // AVHTTP_POST_FORM_HPP
