#ifndef _CHANNEL_HPP__
#define _CHANNEL_HPP__

#include "callback.hpp"
#include "event.hpp"

namespace ev {

class EventLoop;

class Channel {
	bool close_;
	int fd_;
	pooler_event_t events_;
	pooler_event_t revents_;
	EventLoop *const owner_event_loop_;
	ReadCallback read_callback_;
	WriteCallback write_callback_;
	ErrorCallback error_callback_;
	CloseCallback close_callback_;
	DeleteCallback delete_callback_;
public:
	Channel(EventLoop *event_loop, int fd);
	Channel(Channel &&channel);
	~Channel();

	void exec();
	void close();

	inline int fd() {
		return fd_;
	}

	inline pooler_event_t event() {
		return events_;
	}

	inline const EventLoop *event_loop() {
		return owner_event_loop_;
	}

	inline void set_revents(pooler_event_t revents) {
		revents_ = revents;
	}

	inline void setReadCallback(ReadCallback callback) {
		events_ |= EV_POOLER_READ;
		read_callback_ = callback;
	}

	inline void setWriteCallback(WriteCallback callback) {
		events_ |= EV_POOLER_WRITE;
		write_callback_ = callback;
	}

	inline void setErrorCallback(ErrorCallback callback) {
		events_ |= EV_POOLER_ERROR;
		error_callback_ = callback;
	}

	inline void setCloseCallback(CloseCallback callback) {
		events_ |= EV_POOLER_RDHUP;
		close_callback_ = callback;
	}

	inline void setDeleteCallback(DeleteCallback callback) {
		delete_callback_ = callback;
	}

	Channel(const Channel &) = delete;
	Channel &operator=(const Channel &) = delete;
}; // class Channel

} // namespace ev

#endif // _CHANNEL_HPP__