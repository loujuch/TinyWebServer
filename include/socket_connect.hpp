#ifndef _SOCKET_CONNECT_HPP__
#define _SOCKET_CONNECT_HPP__

namespace web {

class WebServer;

class SocketConnect {
	WebServer *web_;
	bool close_, close_write_, close_read_;
	int sock_;
public:
	explicit SocketConnect(int sock, WebServer *web);
	SocketConnect(SocketConnect &&sock);
	~SocketConnect();

	int recv(char *buffer, int size);
	int send(const char *buffer, int size);

	int shut_write();
	int shut_read();
	int shut_both();

	int close();

	inline int socket() const {
		return sock_;
	}

	inline bool is_close() const {
		return close_;
	}

	SocketConnect(const SocketConnect &) = delete;
	SocketConnect &operator=(const SocketConnect &) = delete;
}; // class SocketConnect

} // namespace web

#endif // _SOCKET_CONNECT_HPP__