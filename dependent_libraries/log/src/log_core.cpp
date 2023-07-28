#include "log_core.hpp"

#include <functional>

#include "log_config.hpp"
#include "log_file.hpp"


std::atomic<bool> logger::LogCore::running_(false);

logger::LogCore::LogCore() :
	run_(true),
	write_status_(WriteThreadStatus::WAIT_CREATE),
	is_empty(true),
	write_thread_(std::bind(&LogCore::func, this)) {
	::pthread_rwlock_init(&core_mutex_, nullptr);
}

logger::LogCore::~LogCore() {
	::pthread_rwlock_destroy(&core_mutex_);
}

void logger::LogCore::append(const std::string &s) {
	if(s.empty() || !LogConfig::getInstance().check_level(LogLevel::LOG_OFF)) {
		return;
	}
	bool is;
	::pthread_rwlock_rdlock(&core_mutex_);
	is = buffer_.append(s);
	is_empty = false;
	pthread_rwlock_unlock(&core_mutex_);
	if(is) {
		core_cond_.notify_one();
	}
}

void logger::LogCore::switch_buffer(LogBufferList &buf) {
	std::mutex tmp_mutex;
	std::unique_lock<std::mutex> locker(tmp_mutex);
	do {
		write_status_ = WriteThreadStatus::WAIT_SIGNAL;
		write_status_ = (core_cond_.wait_for(locker, LogConfig::getInstance().log_sync_time())
			== std::cv_status::timeout) ?
			WriteThreadStatus::TimeOut :
			WriteThreadStatus::NOTIFY;
	} while(is_empty && run_);
	tmp_mutex.unlock();
	if(run_) {
		write_status_ = WriteThreadStatus::WAIT_DATA;
		::pthread_rwlock_wrlock(&core_mutex_);
		swap(buf, buffer_);
		is_empty = true;
		pthread_rwlock_unlock(&core_mutex_);
		write_status_ = WriteThreadStatus::GET_DATA;
	}
	locker.unlock();
}

void logger::LogCore::commit(const LogBuffer &buffer) {
	LogFile::getInstance().append(buffer);
}

void logger::LogCore::wait_write_thread_done() {
	// 循环退出条件
	// 1. 写线程已关闭(write_status_ == WriteThreadStatus::END)
	// 2. 写线程阻塞在“等待前端提交数据”处但前端不存在提交数据
	//         (write_status_ == WriteThreadStatus::WAIT_SIGNAL && is_empty)
	while(!((write_status_ == WriteThreadStatus::END) ||
		(write_status_ == WriteThreadStatus::WAIT_SIGNAL && is_empty))) {
		std::this_thread::yield();
	}
}

void logger::LogCore::close() {
	if(run_ == false) {
		return;
	}
	LogConfig::getInstance().set_base_level(LogLevel::LOG_OFF);
	wait_write_thread_done();
	run_ = false;
	core_cond_.notify_one();
	write_thread_.join();
	LogFile::getInstance().close();
}

void logger::LogCore::func() {
	running_ = true;
	write_status_ = WriteThreadStatus::START;
	while(run_) {
		write_status_ = WriteThreadStatus::ONE_LOOP_START;
		LogBufferList link;
		write_status_ = WriteThreadStatus::READY_DATA;
		switch_buffer(link);
		write_status_ = WriteThreadStatus::READY_DATA_DONE;
		while(link.now() != nullptr && run_) {
			write_status_ = WriteThreadStatus::COMMIT_DATA_START;
			commit(*link.now());
			link.next();
			write_status_ = WriteThreadStatus::COMMIT_DATA_END;
		}
		LogFile::getInstance().flush();
		write_status_ = WriteThreadStatus::ONE_LOOP_END;
	}
	write_status_ = WriteThreadStatus::END;
	running_ = false;
}