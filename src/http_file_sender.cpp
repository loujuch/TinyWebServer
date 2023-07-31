#include "http_file_sender.hpp"

#include <unordered_map>
#include <iostream>

#include "log.hpp"

#include <assert.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

std::string web::HttpFileSender::web_root_;

web::HttpFileSender::HttpFileSender(const std::string &file_name) :
	fd_(-1),
	off_(0),
	size_(0),
	can_exec_(false),
	exist_(false) {
	if(file_name.empty()) {
		logger::log_error << "file_name is empty";
		return;
	}
	auto tmp(std::move(file_path(file_name)));
	can_exec_ = (access(tmp.c_str(), F_OK | X_OK) == 0);
	if(can_exec_) {
		exist_ = true;
		return;
	}
	exist_ = (access(tmp.c_str(), F_OK && R_OK) == 0);
	if(exist_) {
		struct stat s;
		int n = stat(tmp.c_str(), &s);
		if(n != -1 && S_ISREG(s.st_mode)) {
			char buffer[32];
			size_ = s.st_size;
			auto p = ::localtime(&s.st_mtim.tv_sec);
			strftime(buffer, sizeof(buffer), "%a, %d %b %Y %T GMT", p);
			last_modified_time_ = buffer;
		} else {
			exist_ = false;
		}
	}
	if(exist_) {
		fd_ = ::open(tmp.c_str(), O_RDONLY);
		assert(fd_ >= 0);
	}
}

web::HttpFileSender::~HttpFileSender() {
	if(fd_ != -1) {
		::close(fd_);
	}
}

std::string web::HttpFileSender::file_path(const std::string &file_name) {
	if(file_name.empty()) {
		logger::log_error << "file_name is empty";
		return web_root_ + "/index.html";
	}
	auto tmp = web_root_;
	if(file_name[0] != '/') {
		tmp.push_back('/');
	}
	tmp.append(file_name);
	return std::move(tmp);
}

int web::HttpFileSender::send_file_to_network(int sock) {
	if(sock < 0 || fd_ < 0 || off_ >= size_) {
		logger::log_warn << "socket " << sock << " fd "
			<< fd_ << " off " << off_ << " size " << size_;
		return -2;
	}
	int n = ::sendfile(sock, fd_, &off_, size_ - off_);
	if(n == -1) {
		if(errno != EINTR && errno != EAGAIN) {
			return 0;
		}
	}
	return n;
}

std::string web::HttpFileSender::file_type(const std::string &file_name) {
	size_t dot = file_name.rfind('.');
	if(dot == std::string::npos) {
		return "Content-Type: application/octet-stream";
	}
	static const std::unordered_map<std::string, std::string> type_prex{
		{"html", "text"},
		{"jpeg", "image"},
		{"png", "image"},
		{"pdf", "application"},
		{"mpeg4", "video"},
		{"mp3", "audio"},
		{"pdf", "application"}
	};
	auto tmp = file_name.substr(dot + 1);
	if(tmp == "jpg") {
		tmp = "jpeg";
	} else if(tmp == "mp4") {
		tmp = "mpeg4";
	}
	auto p = type_prex.find(tmp);
	if(p == type_prex.end()) {
		return "Content-Type: application/octet-stream";
	}
	return p->second + '/' + tmp;
}