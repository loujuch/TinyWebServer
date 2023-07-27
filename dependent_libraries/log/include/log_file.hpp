#ifndef _LOG_FILE_HPP__
#define _LOG_FILE_HPP__

#include "log_buffer.hpp"

#include <stdio.h>

namespace logger {

class LogFile {
	FILE *fp_;
	uint64_t size_;
	char buffer_[LogConfig::log_buffer_size * 2];

	LogFile();
	~LogFile();

	void use_new_file();
	std::string get_file_name_str();
public:
	inline static LogFile &getInstance() {
		static LogFile file;
		return file;
	}

	void append(const LogBuffer &buffer);
	void flush();
	void close();

	LogFile(const LogFile &) = delete;
	LogFile &operator=(const LogFile &) = delete;
};

} // namespace log

#endif // _LOG_FILE_HPP__