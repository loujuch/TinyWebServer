#ifndef _HTTP_FILE_SENDER_HPP__
#define _HTTP_FILE_SENDER_HPP__

#include <string>

#include <stdint.h>

namespace web {

class HttpFileSender {
	static std::string web_root_;

	int fd_;
	off_t off_;
	off_t size_;
	bool can_exec_;
	bool exist_;
public:
	explicit HttpFileSender(const std::string &file_name);
	~HttpFileSender();

	int send_file_to_network(int sock);

	inline bool exist() {
		return exist_;
	}

	inline bool can_exec() {
		return can_exec_;
	}

	inline bool finish() {
		return off_ == size_;
	}

	static std::string file_type(const std::string &file_name);

	static inline void set_web_root(const std::string &web_root) {
		web_root_ = web_root;
		while(web_root_.back() == '/') {
			web_root_.pop_back();
		}
	}

	static inline const std::string &web_root() {
		return web_root_;
	}

	HttpFileSender(const HttpFileSender &) = delete;
	HttpFileSender &operator=(const HttpFileSender &) = delete;
};

} //namespace web

#endif // _HTTP_FILE_SENDER_HPP__