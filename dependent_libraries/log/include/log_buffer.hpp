#ifndef _LOG_BUFFER_HPP__
#define _LOG_BUFFER_HPP__

// #include <iostream>
#include <memory>

#include "log_config.hpp"

namespace logger {

class LogBuffer {
public:
	static constexpr unsigned log_buffer_size = LogConfig::log_buffer_size;
private:
	char buffer_[log_buffer_size];
	unsigned size_;
	std::shared_ptr<LogBuffer> next_;
public:
	LogBuffer();

	// void show() {
	// 	buffer_[size_ - (size_ == log_buffer_size)] = '\0';
	// 	std::cout << buffer_;
	// }

	inline char *assign(const std::string &s) {
		if(free_size() < s.size()) {
			return nullptr;
		}
		char *out = buffer_ + size_;
		size_ += s.size();
		return out;
	}

	inline unsigned size() const {
		return size_;
	}

	inline unsigned free_size() const {
		return log_buffer_size - size_;
	}

	inline void clear() {
		size_ = 0;
	}

	inline const char *str() const {
		return buffer_;
	}

	inline std::shared_ptr<LogBuffer> &next() {
		return next_;
	}

	bool append(const std::string &s);
};

} // namespace log

#endif // _LOG_BUFFER_HPP__