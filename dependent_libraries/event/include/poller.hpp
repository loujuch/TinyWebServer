#ifndef _EPOLL_HPP__
#define _EPOLL_HPP__

#include <unordered_map>

#include "event.hpp"

namespace ev {

class EventLoop;
class Channel;

class Poller {
protected:
	using ChannelMap = std::unordered_map<int, Channel *>;

	ChannelMap channel_map_;
	EventLoop *owner_event_loop_;
public:
	Poller(EventLoop *event_loop);
	virtual ~Poller();

	virtual int wait(ChannelLists channel_list, int timeout) = 0;

	virtual void setChannel(Channel *channel) = 0;
	virtual void delChannel(Channel *channel) = 0;
	bool hasChannel(Channel *channel) const;

	inline bool empty() const {
		return channel_map_.empty();
	}

	static Poller *new_default_poller();

	Poller(const Poller &) = delete;
	Poller &operator=(const Poller &) = delete;
}; // class Poller

} // namespace ev

#endif // _EPOLL_HPP__