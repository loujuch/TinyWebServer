#ifndef _LOG_CORE_HPP__
#define _LOG_CORE_HPP__

#include <thread>
#include <condition_variable>
#include <string>
#include <atomic>

#include "log_buffer_list.hpp"

#include <pthread.h>

namespace logger {

enum class WriteThreadStatus :uint8_t {
	WAIT_CREATE = 0,
	START = 1,
	ONE_LOOP_START = 2,
	READY_DATA = 3,
	WAIT_SIGNAL = 4,
	TimeOut = 5,
	NOTIFY = 6,
	WAIT_DATA = 7,
	GET_DATA = 8,
	READY_DATA_DONE = 9,
	COMMIT_DATA_START = 10,
	COMMIT_DATA_END = 11,
	ONE_LOOP_END = 12,
	END = 13
};

class LogCore {
	static std::atomic<bool> running_;

	std::atomic<bool> run_;
	std::atomic<WriteThreadStatus> write_status_;
	std::atomic<bool> is_empty;
	std::thread write_thread_;
	::pthread_rwlock_t core_mutex_;
	std::condition_variable core_cond_;
	LogBufferList buffer_;

	LogCore();
	~LogCore();

	void wait_write_thread_done();
	void func();
	void switch_buffer(LogBufferList &n);
	void commit(const LogBuffer &buffer);
public:
	inline static bool running() {
		return running_;
	}

	inline static LogCore &getInstance() {
		static LogCore core;
		return core;
	}

	void append(const std::string &s);
	void close();

	LogCore(const LogCore &) = delete;
	LogCore &operator=(const LogCore &) = delete;
}; // class LogCore

} // namespace log

#endif // _LOG_CORE_HPP__