#ifndef _LOG_LEVEL_HPP__
#define _LOG_LEVEL_HPP__

#include <string>

#include <cstdint>

namespace logger {

enum class LogLevel : uint8_t {
	LOG_OFF = 0,
	LOG_FATAL = 1,
	LOG_ERROR = 2,
	LOG_WARN = 3,
	LOG_INFO = 4,
	LOG_DEBUG = 5,
	LOG_TRACE = 6,
	LOG_ALL = 7
}; // class LogLevel

} // namespace log

#endif // _LOG_LEVEL_HPP__