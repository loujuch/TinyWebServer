#ifndef _LOG_CONFIG_HPP__
#define _LOG_CONFIG_HPP__

#include <atomic>
#include <chrono>
#include <string>

#include "log_level.hpp"

namespace logger {

class LogConfig {
	std::string log_file_prefix_;
	std::string log_dir_;
	std::atomic<uint64_t> log_file_size_limit_;
	std::atomic<std::chrono::milliseconds> log_sync_time_;
	std::atomic<LogLevel> base_level_;

	LogConfig();

	static inline bool vaild(LogLevel base, LogLevel repeat) {
		return (repeat <= base) && (base != LogLevel::LOG_OFF);
	}
public:
	static constexpr unsigned log_buffer_size = 1024 * 1024;

	inline static LogConfig &getInstance() {
		static LogConfig config;
		return config;
	}

	bool set_log_file_prefix(const std::string &log_program);
	bool set_log_dir(const std::string &log_dir);

	inline bool set_log_file_size_limit(uint64_t size) {
		if(size <= log_buffer_size) {
			return false;
		}
		log_file_size_limit_ = size;
		return true;
	}

	inline uint64_t log_file_size_limit() {
		return log_file_size_limit_;
	}

	inline std::string &log_dir() {
		return log_dir_;
	}

	inline std::string &log_file_prefix() {
		return log_file_prefix_;
	}

	inline bool set_log_sync_time(unsigned t) {
		if(!t) {
			return false;
		}
		log_sync_time_ = std::chrono::milliseconds(t);
		return true;
	}

	inline std::chrono::milliseconds log_sync_time() {
		return log_sync_time_;
	}

	inline void set_base_level(LogLevel level) {
		base_level_ = level;
	}

	inline bool check_level(LogLevel level) {
		return vaild(base_level_, level);
	}

	LogConfig(const LogConfig &) = delete;
	LogConfig &operator=(const LogConfig &) = delete;
}; // class LogConfig

} // namespace log

#endif // _LOG_CONFIG_HPP__