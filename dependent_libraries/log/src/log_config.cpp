#include "log_config.hpp"

#include "log_core.hpp"

logger::LogConfig::LogConfig() :
	log_file_prefix_("hello_world"),
	log_dir_("./log/"),
	log_file_size_limit_(log_buffer_size * 1024 * 3),
	log_sync_time_(std::chrono::milliseconds(50)),
	base_level_(LogLevel::LOG_OFF) {
}

bool logger::LogConfig::set_log_file_prefix(const std::string &log_program) {
	if(LogCore::running()) {
		return false;
	}
	std::string::size_type pos = log_program.rfind('/');
	if(pos != std::string::npos) {
		log_file_prefix_ = log_program;
	} else {
		log_file_prefix_ = log_program.substr(pos + 1);
	}
	return true;
}

bool logger::LogConfig::set_log_dir(const std::string &log_dir) {
	if(LogCore::running()) {
		return false;
	}
	log_dir_ = log_dir;
	if(log_dir_.back() != '/') {
		log_dir_.push_back('/');
	}
	return true;
}