#include "accepter.hpp"

#include "event_loop.hpp"
#include "channel.hpp"
#include <iostream>

#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "log.hpp"

static int safe_close(int fd) {
	logger::log_trace << "start";
	int out = fd;
	if(fd >= 0) {
		out = ::close(fd);
		if(out) {
			logger::log_error << "file close fail: " << "errno = " << errno
				<< ' ' << strerror(errno);
		}
		fd = -1;
	} else {
		logger::log_warn << "fd = " << fd << " had closed";
	}
	logger::log_trace << "end";
	return out;
}


void web::Accepter::listen_error() {
	logger::log_error << "Listen Socket fd: " << listen_fd_ << " Error " << strerror(errno);
	assert(false);
}

void web::Accepter::listen_accept() {
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	int new_sock = accept4(listen_fd_, (struct sockaddr *)&client, &len,
		SOCK_NONBLOCK | SOCK_CLOEXEC);
	if(new_sock < 0) {
		logger::log_fatal << "Accept error " << strerror(errno);
		return;
	}
	logger::log_info << "new socket " << new_sock;
	if(accept_callback_) {
		accept_callback_(new_sock, client);
	}
}

web::Accepter::Accepter(uint16_t port) :
	listen_fd_(-1),
	listen_event_loop_(nullptr),
	listen_channel_(nullptr) {
	listen_fd_ = ::socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
		IPPROTO_TCP);
	assert(listen_fd_ >= 0);
	logger::log_info << "Listen Socket fd: " << listen_fd_ << " Create";
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	int res = bind(listen_fd_, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	assert(res != -1);
	logger::log_info << "Listen Socket fd: " << listen_fd_ << " bind 127.0.0.1:" << port;
	listen_event_loop_.reset(new ev::EventLoop);
	listen_channel_.reset(new ev::Channel(listen_event_loop_.get(), listen_fd_));
}

web::Accepter::~Accepter() {
	listen_event_loop_.reset();
	listen_channel_.reset();
	safe_close(listen_fd_);
}

void web::Accepter::free_callback() {
	logger::log_trace << "Accepter: " << this << " free "
		<< timeout << "ms event_loop is " << listen_event_loop_.get();
}

int web::Accepter::run(AcceptCallback callback) {
	int res = listen(listen_fd_, SOMAXCONN);
	assert(res != -1);
	logger::log_info << "run";
	accept_callback_ = callback;
	listen_channel_->setReadCallback(std::bind(&Accepter::listen_accept, this));
	listen_channel_->setErrorCallback(std::bind(&Accepter::listen_error, this));
	listen_event_loop_->setChannel(listen_channel_.get());
	listen_event_loop_->setTimeOut(timeout);
	listen_event_loop_->setFreeCallback(std::bind(&Accepter::free_callback, this));
	return listen_event_loop_->run();
}