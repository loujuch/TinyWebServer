#ifndef _LOG_INTERFACE_HPP__
#define _LOG_INTERFACE_HPP__

#include "log_level.hpp"
#include "log_formatter.hpp"

namespace logger {

// 用户接口类，用于格式化用户数据，只有一个用法：LogInterface(...).stream()<<...;
class LogInterface {
	bool format_done_;
	const LogLevel level_;
	std::string prefix_;
	std::string suffix_;

	// 将格式化数据提交到处理程序
	void commit();
public:
	explicit LogInterface(const char *file, const char *func, const int line, const LogLevel level);
	~LogInterface();

	inline LogFormatter stream() {
		return LogFormatter(prefix_, level_);
	}

	static void close();

	LogInterface(const LogInterface &) = delete;
	LogInterface &operator=(const LogInterface &) = delete;
}; // class LogInterface

} // namespace log

#endif // _LOG_INTERFACE_HPP__