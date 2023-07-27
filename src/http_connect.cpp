#include "http_connect.hpp"

#include <vector>

#include "event_loop.hpp"
#include "channel.hpp"
#include "http_file_sender.hpp"
#include "log.hpp"
#include "web_server.hpp"

#include <unistd.h>
#include <assert.h>

#include <stdio.h>
#include <string.h>

static const std::string &get_response_head(int number) {
	static const std::vector<std::string> head_str({
		"HTTP/1.0 500 Internal Server Error\r\n",
		"HTTP/1.0 200 OK\r\n",
		"HTTP/1.0 404 Not Found\r\n",
		"HTTP/1.0 400 Bad Request\r\n"
		});
	switch(number) {
		case 200:return head_str[1];
		case 404:return head_str[2];
		case 400:return head_str[3];
		case 500:return head_str[0];
		default:return head_str[0];
	}
	return head_str[0];
}

web::HttpConnect::HttpConnect(ev::EventLoop *loop, int sock,
	const sockaddr_in new_addr, WebServer *web) :
	status(0),
	close_(false),
	conn_sockfd_(sock),
	own_web_server_(web),
	conn_channel_(nullptr),
	conn_sender_(nullptr),
	own_event_loop_(loop) {
	assert(conn_sockfd_ >= 0);
	assert(own_event_loop_ != nullptr);
	logger::log_info << "new http_connect " << this << " create";
	conn_channel_.reset(new ev::Channel(own_event_loop_, conn_sockfd_));
	assert(conn_channel_ != nullptr);
	conn_channel_->setReadCallback(std::bind(&HttpConnect::conn_read, this));
	conn_channel_->setCloseCallback(std::bind(&HttpConnect::conn_close, this));
	conn_channel_->setErrorCallback(std::bind(&HttpConnect::conn_error, this));
	conn_channel_->setDeleteCallback(std::bind(&HttpConnect::conn_delete, this));
	logger::log_info << "http_connect " << this << " timer into loop " << own_event_loop_;
	timer_id_ = own_event_loop_->addAfterTimer(3000, std::bind(&HttpConnect::conn_timer, this));
	logger::log_info << "http_connect " << this << " channel " << conn_channel_.get()
		<< " into loop " << own_event_loop_;
	own_event_loop_->setChannel(conn_channel_.get());
}

web::HttpConnect::~HttpConnect() {
	logger::log_info << "http_connect " << this << " delete";
	logger::log_info << "http_connect " << this << " close timer from loop " << own_event_loop_;
	own_event_loop_->closeTimer(timer_id_);
	logger::log_info << "http_connect " << this << " close channel "
		<< conn_channel_.get() << " from loop " << own_event_loop_;
	conn_channel_->close();
	logger::log_info << "http_connect " << this << " close sock " << conn_sockfd_;
	::close(conn_sockfd_);
	conn_sockfd_ = -1;
	own_web_server_->close_conn();
}

void web::HttpConnect::conn_read() {
	own_event_loop_->closeTimer(timer_id_);
	static constexpr int buffer_size = 1024 * 8;
	char buffer[buffer_size];
	int n = receive(buffer, sizeof(buffer));
	if(n < 0) {
		return close();
	} else if(n == 0) {
		return;
	}
	if(head_parser_.error()) {
		logger::log_warn << "html head error";
		auto tmp = get_response_head(500) + "text/html\r\n\r\n";
		::send(conn_sockfd_, tmp.c_str(), tmp.size(), 0);
		conn_sender_.reset(new HttpFileSender("/500.html"));
		if(conn_sender_->exist()) {
			conn_sender_->send_file_to_network(conn_sockfd_);
		}
		conn_sender_.reset();
		return close();
	} else if(!head_parser_.finish()) {
		head_parser_.parser(buffer, n);
	}
	if(head_parser_.error()) {
		logger::log_warn << "html head error";
		auto tmp = get_response_head(500) + "text/html\r\n\r\n";
		::send(conn_sockfd_, tmp.c_str(), tmp.size(), 0);
		conn_sender_.reset(new HttpFileSender("/500.html"));
		if(conn_sender_->exist()) {
			conn_sender_->send_file_to_network(conn_sockfd_);
		}
		conn_sender_.reset();
		return close();
	} else if(head_parser_.finish()) {
		auto f = head_parser_.file_path();
		if(f.back() == '/') {
			f += "index.html";
		}
		conn_sender_.reset(new HttpFileSender(f));
		if(!conn_sender_->exist()) {
			auto tmp = get_response_head(404) + "text/html\r\n\r\n";
			::send(conn_sockfd_, tmp.c_str(), tmp.size(), 0);
			conn_sender_.reset(new HttpFileSender("/404.html"));
			if(conn_sender_->exist()) {
				conn_sender_->send_file_to_network(conn_sockfd_);
			}
			return close();
		} else {
			if(::strcasecmp(head_parser_.method().c_str(), "GET") == 0) {
				if(conn_sender_->can_exec()) {
				} else {
					auto tmp = get_response_head(200) +
						HttpFileSender::file_type(f) + "\r\n\r\n";
					::send(conn_sockfd_, tmp.c_str(), tmp.size(), 0);
					conn_sender_->send_file_to_network(conn_sockfd_);
					if(conn_sender_->finish()) {
						return close();
					} else {
						conn_channel_->setWriteCallback(std::bind(&HttpConnect::conn_write, this));
						own_event_loop_->setChannel(conn_channel_.get());
					}
				}
			} else if(::strcasecmp(head_parser_.method().c_str(), "POST") == 0) {
			} else {
			}
		}
	}
	timer_id_ = own_event_loop_->addAfterTimer(3000, std::bind(&HttpConnect::conn_timer, this));
}

void web::HttpConnect::conn_write() {
	assert(conn_sender_ != nullptr && conn_sockfd_ >= 0);
	own_event_loop_->closeTimer(timer_id_);
	int n = conn_sender_->send_file_to_network(conn_sockfd_);
	if(conn_sender_->finish()) {
		conn_sender_.reset();
		return close();
	} else {
		timer_id_ = own_event_loop_->addAfterTimer(3000, std::bind(&HttpConnect::conn_timer, this));
	}
}

int web::HttpConnect::receive(char *buffer, int size) {
	int n = ::recv(conn_sockfd_, buffer, size, 0);
	if(n == 0) {
		logger::log_warn << "http_connect " << this << " recv fd " <<
			conn_channel_->fd() << " return 0";
		return -1;
	} else if(n < 0) {
		if(errno == EINTR || errno == EAGAIN) {
			logger::log_info << "http_connect " << this << " recv fd " <<
				conn_channel_->fd() << " get EINTR or EAGAIN";
			return 0;
		} else {
			logger::log_error << "http_connect " << this << " recv fd " <<
				conn_channel_->fd() << " error " << strerror(errno);
			return -1;
		}
	}
	return n;
}


void web::HttpConnect::response() {
	logger::log_info << "Socket: " << conn_sockfd_ << " read";
	static const char html[1024] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"
		"<HTML><TITLE>index</TITLE><BODY>"
		"<h1>this is a test!<h1></BODY></HTML>";
	static const int len = strlen(html);
	::write(conn_sockfd_, html, len);
	close();
}

void web::HttpConnect::conn_close() {
	logger::log_info << "Socket: " << conn_sockfd_ << " close";
	close();
}

void web::HttpConnect::conn_error() {
	logger::log_error << "Socket: " << conn_sockfd_ << " error";
	close();
}

void web::HttpConnect::conn_timer() {
	logger::log_warn << "Socket: " << conn_sockfd_ << " timeout";
	delete this;
}

void web::HttpConnect::conn_delete() {
	if(!close_) {
		close_ = true;
		logger::log_info << "Socket: " << conn_sockfd_ << " close";
		delete this;
	}
}

void web::HttpConnect::close() {
	logger::log_info << "channel: " << conn_channel_.get() << " close";
	conn_channel_->close();
}