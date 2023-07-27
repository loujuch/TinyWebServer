#ifndef _LOG_TIME_STAMP__
#define _LOG_TIME_STAMP__

#include <string>

namespace logger {

class LogTimeStamp {
	LogTimeStamp();
public:
	inline static LogTimeStamp &getInstance() {
		static LogTimeStamp timestamp;
		return timestamp;
	}

	std::string get_time_stamp_str();

	std::string get_time_stamp_str_in_file_name();

	LogTimeStamp(const LogTimeStamp &) = delete;
	LogTimeStamp &operator=(const LogTimeStamp &) = delete;
};

} // namespace log

#endif // _LOG_TIME_STAMP__