#include "http_cgi.hpp"

#include <iostream>

#include "channel.hpp"
#include "event_loop.hpp"

#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

const std::vector<std::pair<std::string, std::string>> web::HttpCGI::env_var(
	{
		{ "Content-Length","CONTENT_LENGTH" },
		{ "Content-Type", "CONTENT_TYPE" },
		{ "Cookie", "HTTP_COOKIE" },
		{ "User-Agent", "HTTP_USER_AGENT"}
	}
);

web::HttpCGI::HttpCGI(const std::string &exec_file,
	const std::string &method,
	const std::vector<std::pair<std::string, std::string>> &env_bar,
	ev::EventLoop *loop) :
	pid_(-1),
	error_(false),
	close_(false),
	this_send_fork_recv_{ -1, -1 },
	this_recv_fork_send_{ -1, -1 },
	own_event_loop_(loop),
	recv_channel_(nullptr) {
	bool is_get = ::strcasecmp(method.c_str(), "GET") == 0;
	bool is_post = ::strcasecmp(method.c_str(), "POST") == 0;
	if((!(is_get || is_post))) {
		error_ = true;
		return;
	}
	if(::pipe2(this_recv_fork_send_, 0) < 0) {
		error_ = true;
		return;
	}
	if(::pipe2(this_send_fork_recv_, 0) < 0) {
		::close(this_recv_fork_send_[0]);
		::close(this_recv_fork_send_[1]);
		error_ = true;
		return;
	}
	pid_ = fork();
	if(pid_ < 0) {
		::close(this_send_fork_recv_[0]);
		::close(this_send_fork_recv_[1]);
		::close(this_recv_fork_send_[0]);
		::close(this_recv_fork_send_[1]);
		error_ = true;
		return;
	}
	if(pid_ == 0) {
		::close(this_recv_fork_send_[0]);
		::close(this_send_fork_recv_[1]);
		::dup2(this_recv_fork_send_[1], STDOUT_FILENO);
		::dup2(this_send_fork_recv_[0], STDIN_FILENO);
		::setenv("REQUEST_METHOD", method.c_str(), 1);
		for(const auto &e : env_bar) {
			::setenv(e.first.c_str(), e.second.c_str(), 1);
		}
		::execl(exec_file.c_str(), exec_file.c_str(), nullptr);
		::exit(0);
	}
	::close(this_recv_fork_send_[1]);
	::close(this_send_fork_recv_[0]);
	int flag;
	flag = ::fcntl(this_recv_fork_send_[0], F_GETFL, 0);
	::fcntl(this_recv_fork_send_[0], F_SETFL, flag | O_NONBLOCK);
	flag = ::fcntl(this_send_fork_recv_[1], F_GETFL, 0);
	::fcntl(this_send_fork_recv_[1], F_SETFL, flag | O_NONBLOCK);
	recv_channel_.reset(new ev::Channel(own_event_loop_, this_recv_fork_send_[0]));
}

web::HttpCGI::~HttpCGI() {
	close();
	int status = -1;
	if(::waitpid(pid_, &status, WNOHANG) == 0) {
		::kill(pid_, SIGKILL);
	}
}

void web::HttpCGI::exec() {
	close_ = false;
	own_event_loop_->setChannel(recv_channel_.get());
}

void web::HttpCGI::close() {
	if(!close_) {
		close_ = true;
		own_event_loop_->delChannel(recv_channel_.get());
		recv_channel_.reset();
		::close(this_recv_fork_send_[0]);
		::close(this_send_fork_recv_[1]);
	}
}

int web::HttpCGI::send(const char *buffer, int size) {
	if(close_) {
		return 0;
	}
	int n = ::write(this_send_fork_recv_[1], buffer, size);
	if(n == 0) {
		return -1;
	} else if(n < 0) {
		if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}
	}
	return n;
}

int web::HttpCGI::recv(char *buffer, int size) {
	if(close_) {
		return 0;
	}
	int n = ::read(this_recv_fork_send_[0], buffer, size);
	if(n == 0) {
		return -1;
	} else if(n < 0) {
		if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}
	}
	return n;
}