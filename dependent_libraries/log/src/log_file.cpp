#include "log_file.hpp"

#include <iostream>

#include "log_timestamp.hpp"

#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

void mkdir(const std::string &path) {
	int s = 0;
	for(int i = 0;i < path.size();++i) {
		if(i == '/') {
			auto sub_dir = path.substr(s, i - s);
			if(::access(sub_dir.c_str(), F_OK)) {
				if(::mkdir(sub_dir.c_str(), 0775)) {
					std::cerr << __FILE__ << ' ' << __func__ << ' ' << __LINE__ << ' '
						<< strerror(errno) << std::endl;
				}
			}
			s = i + 1;
		}
	}
	if(s != path.size()) {
		auto sub_dir = path.substr(s);
		if(::access(sub_dir.c_str(), F_OK)) {
			if(::mkdir(sub_dir.c_str(), 0775)) {
				std::cerr << __FILE__ << ' ' << __func__ << ' ' << __LINE__ << ' '
					<< strerror(errno) << std::endl;
			}
		}
	}
}

logger::LogFile::LogFile() :fp_(nullptr), size_(0) {
	mkdir(LogConfig::getInstance().log_dir());
	use_new_file();
}

logger::LogFile::~LogFile() {
	close();
}

void logger::LogFile::use_new_file() {
	close();
	auto file_name = get_file_name_str();
	fp_ = ::fopen(file_name.c_str(), "ae");
	::setbuffer(fp_, buffer_, sizeof(buffer_));
}

std::string logger::LogFile::get_file_name_str() {
	return LogConfig::getInstance().log_dir() + LogConfig::getInstance().log_file_prefix() +
		"_" + LogTimeStamp::getInstance().get_time_stamp_str_in_file_name() + "_" +
		std::to_string(::getpid()) + ".log";
}

void logger::LogFile::append(const LogBuffer &buffer) {
	if(buffer.size() + size_ >= LogConfig::getInstance().log_file_size_limit()) {
		use_new_file();
	}
	const char *p = buffer.str();
	uint64_t need = buffer.size();
	do {
		uint64_t n = ::fwrite_unlocked(buffer.str(), 1, need, fp_);
		if(n == 0) {
			int err = ferror(fp_);
			std::cerr << __FILE__ << ' ' << __func__ << ' ' << __LINE__ << ' '
				<< strerror(err) << std::endl;
			break;
		}
		p += n;
		size_ += n;
		need -= n;
	} while(need);
}

void logger::LogFile::flush() {
	::fflush(fp_);
}

void logger::LogFile::close() {
	if(fp_ != nullptr) {
		flush();
		::fclose(fp_);
		fp_ = nullptr;
		size_ = 0;
	}
}