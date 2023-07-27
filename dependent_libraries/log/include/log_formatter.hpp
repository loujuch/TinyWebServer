#ifndef _LOG_FORMATTER_HPP__
#define _LOG_FORMATTER_HPP__

#include "log_level.hpp"

namespace logger {

class LogFormatter {
	LogLevel level_;
	std::string &format_str_;
public:
	LogFormatter(std::string &s, LogLevel level);

	LogFormatter &operator<<(const bool t);
	LogFormatter &operator<<(const LogLevel level);
	LogFormatter &operator<<(const char t);
	LogFormatter &operator<<(const unsigned char t);
	LogFormatter &operator<<(const int16_t t);
	LogFormatter &operator<<(const uint16_t t);
	LogFormatter &operator<<(const int32_t t);
	LogFormatter &operator<<(const uint32_t t);
	LogFormatter &operator<<(const int64_t t);
	LogFormatter &operator<<(const uint64_t t);

	LogFormatter &operator<<(const float t);
	LogFormatter &operator<<(const double t);

	LogFormatter &operator<<(const char *t);
	LogFormatter &operator<<(const void *t);

	// 对一些常用容器添加操作
	LogFormatter &operator<<(const std::string &t);
}; // class LogFormatter

} // namespace log

#endif // _LOG_FORMATER_HPP__