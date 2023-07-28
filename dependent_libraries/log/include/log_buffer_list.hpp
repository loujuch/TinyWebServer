#ifndef _LOG_BUFFER_LINK_HPP__
#define _LOG_BUFFER_LINK_HPP__

#include <mutex>

#include "log_buffer.hpp"

namespace logger {

class LogBufferList {
	std::mutex assign_mutex_;
	std::shared_ptr<LogBuffer> front_, cur_, back_;

	inline void add_new() {
		auto *p = new LogBuffer;
		back_->next().reset(p);
		back_ = back_->next();
	}
public:
	explicit LogBufferList();

	bool append(const std::string &s);

	inline void reset() {
		cur_ = front_;
	}

	inline std::shared_ptr<LogBuffer> now() {
		return cur_;
	}

	inline void next() {
		cur_ = cur_ == nullptr ? cur_ : cur_->next();
	}

	friend void swap(LogBufferList &l, LogBufferList &r) {
		l.front_.swap(r.front_);
		l.cur_.swap(r.cur_);
		l.back_.swap(r.back_);
	}

	LogBufferList(const LogBufferList &) = delete;
	LogBufferList &operator=(const LogBufferList &) = delete;
}; // class LogBufferList

} // namespace log

#endif // _LOG_BUFFER_LINK_HPP__