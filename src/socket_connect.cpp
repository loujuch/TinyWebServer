#include "socket_connect.hpp"

#include "web_server.hpp"

#include <unistd.h>
#include <sys/socket.h>

web::SocketConnect::SocketConnect(int sock, const sockaddr_in &remote_addr, WebServer *web) :
	web_(web),
	close_(false),
	close_write_(false),
	close_read_(false),
	remote_addr_(remote_addr),
	sock_(sock) {
	if(sock_ < 0) {
		close_ = true;
		close_read_ = true;
		close_write_ = true;
	}
}

web::SocketConnect::SocketConnect(SocketConnect &&sock) :
	web_(sock.web_),
	close_(sock.close_),
	close_write_(sock.close_write_),
	close_read_(sock.close_read_),
	remote_addr_(sock.remote_addr_),
	sock_(sock.sock_) {
	sock.web_ = nullptr;
	sock.close_ = true;
	sock.close_write_ = true;
	sock.close_read_ = true;
	sock.sock_ = -1;
}

web::SocketConnect::~SocketConnect() {
	close();
}

int web::SocketConnect::recv(char *buffer, int size) {
	if(close_read_) {
		return -2;
	}
	int n = ::recv(sock_, buffer, size, 0);
	if(n == 0) {
		return -1;
	} else if(n < 0) {
		if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		} else {
			return -1;
		}
	}
	return n;
}

int web::SocketConnect::send(const char *buffer, int size) {
	if(close_write_) {
		return -2;
	}
	int n = ::send(sock_, buffer, size, 0);
	if(n == 0) {
		return -1;
	} else if(n < 0) {
		if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		} else {
			return -1;
		}
	}
	return n;
}

int web::SocketConnect::shut_write() {
	if(close_write_) {
		return -2;
	}
	close_write_ = true;
	return ::shutdown(sock_, SHUT_WR);
}

int web::SocketConnect::shut_read() {
	if(close_read_) {
		return -2;
	}
	close_read_ = true;
	return ::shutdown(sock_, SHUT_RD);
}

int web::SocketConnect::shut_both() {
	if(close_read_ && close_write_) {
		return -2;
	} else if(close_read_) {
		return shut_write();
	} else if(close_write_) {
		return shut_read();
	} else {
		close_read_ = true;
		close_write_ = true;
		return ::shutdown(sock_, SHUT_RDWR);
	}
	return 0;
}

int web::SocketConnect::close() {
	int out = -1;
	if(!close_) {
		close_ = true;
		close_read_ = true;
		close_write_ = true;
		int out = ::close(sock_);
		sock_ = -1;
		web_->close_conn();
	}
	return out;
}