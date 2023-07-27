#include "log_interface.hpp"

#include "log_timestamp.hpp"
#include "log_core.hpp"
#include "log_config.hpp"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

logger::LogInterface::LogInterface(const char *file, const char *func,
	const int line, const LogLevel level) :format_done_(false), level_(level) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return;
	}
	if(file[0] == '.' && file[1] == '/') {
		file += 2;
	}
	logger::LogFormatter(prefix_, level_) << '['
		<< LogTimeStamp::getInstance().get_time_stamp_str()
		<< "] {" << ::syscall(SYS_gettid) << "->" << func << "} ["
		<< level << "] ";
	logger::LogFormatter(suffix_, level_) << " [" << file << ':' << line << "]\n";
	format_done_ = true;
}

logger::LogInterface::~LogInterface() {
	// 防止因日志等级调整导致未格式化日志被提交给后端
	if(!(LogConfig::getInstance().check_level(level_) && format_done_)) {
		return;
	}
	commit();
}

void logger::LogInterface::commit() {
	LogCore::getInstance().append(prefix_ + suffix_);
}

void logger::LogInterface::close() {
	LogCore::getInstance().close();
}