#include "web_server.hpp"

#include <iostream>

#include "accepter.hpp"
#include "http_connect_pool.hpp"
#include "event_loop.hpp"
#include "http_file_sender.hpp"
#include "socket_connect.hpp"

#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>

web::WebServer::WebServer(uint16_t port, uint32_t max_conn,
	const std::string &web_root, const std::string &server_name,
	const logger::LogLevel level) :
	run_(false),
	port_(port),
	max_conn_(max_conn),
	now_conn_(0),
	accepter_(nullptr),
	http_connect_pool_(4) {
	logger::log_set_level(level);
	logger::log_set_file_prefix(server_name);
	if(web_root.empty()) {
		HttpFileSender::set_web_root("./web_root");
	} else {
		HttpFileSender::set_web_root(web_root);
	}
	accepter_.reset(new Accepter(port_));
}

web::WebServer::~WebServer() {
	logger::log_close();
}

void web::WebServer::accept_callback(int sock, const sockaddr_in &addr) {
	SocketConnect socket_conn(sock, this);
	++now_conn_;
	if(now_conn_ + 1 >= max_conn_) {
		socket_conn.shut_read();
		static const char server_busy[128] =
			"HTTP/1.0 500 Internal Server Error\r\n"
			"text/html\r\n\r\n"
			"<HTML><TITLE>busy</TITLE><BODY><h1>Server is busy!</h1></BODY></HTML>";
		static const int length = strlen(server_busy);
		socket_conn.send(server_busy, length);
		socket_conn.shut_write();
		socket_conn.close();
	} else {
		http_connect_pool_.add_connect(std::move(socket_conn), addr, this);
	}
}

int web::WebServer::run() {
	if(run_) {
		return 0;
	}
	run_ = true;
	accepter_->event_loop()->setSignal(SIGPIPE, []() {
		logger::log_error << "SIGPIPE: response";
		});
	accepter_->event_loop()->setSignal(SIGHUP, []() {
		logger::log_error << "SIGHUP: response";
		});
	accepter_->event_loop()->setSignal(SIGURG, []() {
		logger::log_error << "SIGURG: response";
		});
	std::cout << "WebServer will run in port " << port_ << std::endl;
	AcceptCallback accept_callback =
		std::bind(&WebServer::accept_callback, this,
			std::placeholders::_1, std::placeholders::_2);
	return accepter_->run(accept_callback);
}