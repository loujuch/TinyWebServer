#ifndef _HTTP_HEAD_PARSER_HPP__
#define _HTTP_HEAD_PARSER_HPP__

#include <string>
#include <unordered_map>

namespace web {

class HttpHeadParser {
	int line_;
	bool finish_;
	bool error_;
	std::string method_;
	std::string query_;
	std::string file_path_;
	std::string version_;
	std::string leave_data_;
	std::unordered_map<std::string, std::string> field_;

	void commit(const char *buffer, int len);
public:
	explicit HttpHeadParser();

	bool parser(const char *buffer, int size);

	inline int line() const {
		return line_;
	}

	inline bool finish() const {
		return finish_;
	}

	inline bool error() const {
		return error_;
	}

	inline const std::string &method() const {
		return method_;
	}

	inline const std::string &query() const {
		return query_;
	}

	inline const std::string &file_path() const {
		return file_path_;
	}

	inline const std::string &version() const {
		return file_path_;
	}

	inline const std::string &leave_data() const {
		return leave_data_;
	}

	inline const std::string &field(const std::string &key) {
		return field_[key];
	}

	inline bool hasField(const std::string &key) {
		return field_.count(key) != 0;
	}
}; // class HttpHeadParser

} // namespace web

#endif // _HTTP_HEAD_PARSER_HPP__