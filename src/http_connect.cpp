#include "http_connect.hpp"

#include <vector>
#include <iostream>

#include "event_loop.hpp"
#include "channel.hpp"
#include "http_file_sender.hpp"
#include "http_cgi.hpp"
#include "http_connect_pool.hpp"
#include "log.hpp"
#include "web_server.hpp"

#include <unistd.h>

#include <stdio.h>
#include <string.h>

static const std::string &get_response_head(int number) {
	static const std::vector<std::string> head_str({
		"HTTP/1.1 500 Internal Server Error\r\n",
		"HTTP/1.1 200 OK\r\n",
		"HTTP/1.1 404 Not Found\r\n",
		"HTTP/1.1 400 Bad Request\r\n",
		"HTTP/1.1 501 Not Implemented\r\n",
		});
	switch(number) {
		case 200:return head_str[1];
		case 404:return head_str[2];
		case 400:return head_str[3];
		case 501:return head_str[4];
		case 500:return head_str[0];
		default:return head_str[0];
	}
	return head_str[0];
}

web::HttpConnect::HttpConnect(ev::EventLoop *loop, SocketConnect &&sock,
	const sockaddr_in new_addr, HttpConnectPool *pool, int group) :
	group_(group),
	pool_(pool),
	close_(false),
	conn_sockfd_(std::move(sock)),
	conn_channel_(nullptr),
	conn_sender_(nullptr),
	conn_cgi_(nullptr),
	own_event_loop_(loop) {
	if(conn_sockfd_.socket() < 0 || conn_sockfd_.is_close()) {
		logger::log_error << "http_connent " << this << " sock " << conn_sockfd_.socket();
		return;
	}
	if(own_event_loop_ == nullptr) {
		logger::log_error << "http_connent " << this << " event_loop == nullptr";
		return;
	}
}

web::HttpConnect::~HttpConnect() {
	logger::log_info << "http_connect " << this << " delete";
	logger::log_info << "http_connect " << this << " close timer from loop " << own_event_loop_;
	own_event_loop_->closeTimer(timer_id_);
	logger::log_info << "http_connect " << this << " close channel "
		<< conn_channel_.get() << " from loop " << own_event_loop_;
	conn_channel_->close();
	logger::log_info << "http_connect " << this << " close sock " << conn_sockfd_.socket();
	conn_sockfd_.close();
}


void web::HttpConnect::exec() {
	logger::log_info << "new http_connect " << this << " create";
	conn_channel_.reset(new ev::Channel(own_event_loop_, conn_sockfd_.socket()));
	conn_channel_->setReadCallback(std::bind(&HttpConnect::conn_read, this));
	conn_channel_->setCloseCallback(std::bind(&HttpConnect::conn_close, this));
	conn_channel_->setErrorCallback(std::bind(&HttpConnect::conn_error, this));
	logger::log_info << "http_connect " << this << " timer into loop " << own_event_loop_;
	timer_id_ = own_event_loop_->addAfterTimer(timeout, std::bind(&HttpConnect::conn_timer, this));
	logger::log_info << "http_connect " << this << " channel " << conn_channel_.get()
		<< " into loop " << own_event_loop_;
	own_event_loop_->setChannel(conn_channel_.get());
}


bool web::HttpConnect::send_error_short_file(int error) {
	logger::log_warn << "http error errno " << error;
	auto tmp = get_response_head(error) +
		"Content-Type: text/html\r\n"
		"Connection: close\r\n"
		"\r\n";
	conn_sockfd_.send(tmp.c_str(), tmp.size());
	conn_sender_.reset(new HttpFileSender("/" + std::to_string(error) + ".html"));
	if(conn_sender_->exist() && ::strcasecmp(head_parser_.method().c_str(), "HEAD") != 0) {
		conn_sender_->send_file_to_network(conn_sockfd_.socket());
	}
	conn_sender_.reset();
	close();
	return false;
}

int web::HttpConnect::set_cgi(const std::string &file_name) {
	std::vector<std::pair<std::string, std::string>> env_bar;
	auto &tmp = HttpCGI::env();
	for(auto &e : tmp) {
		if(head_parser_.hasField(e.first)) {
			env_bar.emplace_back(e.second, head_parser_.field(e.first));
		}
	}
	env_bar.emplace_back("QUERY_STRING", head_parser_.query());
	conn_cgi_.reset(new HttpCGI(file_name, head_parser_.method(), env_bar, own_event_loop_));
	if(conn_cgi_->error()) {
		logger::log_error << " cgi can't exec";
		conn_cgi_.reset();
		return -1;
	} else {
		auto tmp = get_response_head(200) +
			"Connection: close\r\n";
		conn_sockfd_.send(tmp.c_str(), tmp.size());
		conn_cgi_->cgi_channel()->setReadCallback(std::bind(&HttpConnect::cgi_read, this));
		conn_cgi_->cgi_channel()->setCloseCallback(std::bind(&HttpConnect::cgi_close, this));
		conn_cgi_->cgi_channel()->setErrorCallback(std::bind(&HttpConnect::cgi_error, this));
		conn_cgi_->exec();
		if(!head_parser_.leave_data().empty()) {
			conn_cgi_->send(head_parser_.leave_data().c_str(), head_parser_.leave_data().size());
		}
	}
	return 0;
}

bool web::HttpConnect::cgi_read() {
	if(close_) {
		return false;
	}
	if(conn_cgi_ == nullptr) {
		return false;
	}
	own_event_loop_->closeTimer(timer_id_);
	char buffer[1024];
	int size = conn_cgi_->recv(buffer, sizeof(buffer));
	if(size < 0) {
		close();
		return false;
	} else if(size == 0) {
		return true;
	}
	conn_sockfd_.send(buffer, size);
	timer_id_ = own_event_loop_->addAfterTimer(timeout,
		std::bind(&HttpConnect::conn_timer, this));
	return true;
}

bool web::HttpConnect::cgi_close() {
	if(close_) {
		return false;
	}
	return close();
}

bool web::HttpConnect::cgi_error() {
	if(close_) {
		return false;
	}
	return close();
}

bool web::HttpConnect::conn_read() {
	if(close_) {
		return false;
	}
	own_event_loop_->closeTimer(timer_id_);
	static constexpr int buffer_size = 1024 * 8;
	char buffer[buffer_size];
	int n = conn_sockfd_.recv(buffer, sizeof(buffer));
	if(n < 0) {
		return close();
	} else if(n == 0) {
		return true;
	}
	if(conn_cgi_ != nullptr) {
		conn_cgi_->send(buffer, n);
		return true;
	}
	if(head_parser_.error()) {
		logger::log_warn << "html head error";
		return send_error_short_file(500);
	} else if(!head_parser_.finish()) {
		head_parser_.parser(buffer, n);
	}
	if(head_parser_.error()) {
		logger::log_warn << "html head error";
		return send_error_short_file(500);
	} else if(head_parser_.finish()) {
		auto f = head_parser_.file_path();
		if(f.back() == '/') {
			f += "index.html";
		}
		conn_sender_.reset(new HttpFileSender(f));
		if(!conn_sender_->exist()) {
			return send_error_short_file(404);
		} else {
			if(::strcasecmp(head_parser_.method().c_str(), "GET") == 0) {
				if(conn_sender_->can_exec()) {
					if(set_cgi(conn_sender_->file_path(f)) == -1) {
						return send_error_short_file(500);
					}
				} else {
					auto tmp = get_response_head(200) +
						"Content-Type: " + HttpFileSender::file_type(f) + "\r\n"
						"Connection: close\r\n"
						"Last-Modified: " + conn_sender_->last_modified_time() + "\r\n"
						"Content-Length: " + std::to_string(conn_sender_->size()) + "\r\n"
						"\r\n";
					conn_sockfd_.send(tmp.c_str(), tmp.size());
					conn_sender_->send_file_to_network(conn_sockfd_.socket());
					if(conn_sender_->finish()) {
						return close();
					} else {
						conn_channel_->setWriteCallback(
							std::bind(&HttpConnect::conn_write, this));
						own_event_loop_->setChannel(conn_channel_.get());
					}
				}
			} else if(::strcasecmp(head_parser_.method().c_str(), "POST") == 0) {
				if(set_cgi(conn_sender_->file_path(f)) == -1) {
					return send_error_short_file(500);
				}
			} else if(::strcasecmp(head_parser_.method().c_str(), "HEAD") == 0) {
				auto tmp = get_response_head(20) +
					"Content-Type: " + HttpFileSender::file_type(f) + "\r\n"
					"Connection: close\r\n"
					"Content-Length: " + std::to_string(conn_sender_->size()) + "\r\n"
					"Last-Modified: " + conn_sender_->last_modified_time() + "\r\n"
					"\r\n";
				conn_sockfd_.send(tmp.c_str(), tmp.size());
				conn_sender_.reset();
				return close();
			} else {
				return send_error_short_file(501);
			}
		}
	}
	timer_id_ = own_event_loop_->addAfterTimer(timeout, std::bind(&HttpConnect::conn_timer, this));
	return true;
}

bool web::HttpConnect::conn_write() {
	if(close_) {
		return false;
	}
	if(conn_sender_ == nullptr || conn_sockfd_.is_close() || conn_sockfd_.socket() < 0) {
		logger::log_error << "sender " << conn_sender_.get()
			<< " sock " << conn_sockfd_.socket();
		return false;
	}
	own_event_loop_->closeTimer(timer_id_);
	int n = conn_sender_->send_file_to_network(conn_sockfd_.socket());
	if(conn_sender_->finish()) {
		conn_sender_.reset();
		return close();
	} else {
		timer_id_ = own_event_loop_->addAfterTimer(timeout,
			std::bind(&HttpConnect::conn_timer, this));
	}
	return true;
}

bool web::HttpConnect::conn_close() {
	if(close_) {
		return false;
	}
	logger::log_info << "Socket: " << conn_sockfd_.socket() << " close";
	return close();
}

bool web::HttpConnect::conn_error() {
	if(close_) {
		return false;
	}
	logger::log_error << "Socket: " << conn_sockfd_.socket() << " error";
	return close();
}

void web::HttpConnect::conn_timer() {
	if(close_) {
		return;
	}
	logger::log_warn << "Socket: " << conn_sockfd_.socket() << " timeout";
	close();
}

bool web::HttpConnect::close() {
	if(!close_) {
		close_ = true;
		logger::log_info << "Socket: " << conn_sockfd_.socket() << " close";
		pool_->del_connect(this);
	}
	return false;
}