#ifndef _EPOLLER_HPP__
#define _EPOLLER_HPP__

#include "poller.hpp"

namespace ev {

class Epoller :public Poller {
	int epoll_fd_;
public:
	Epoller(EventLoop *event_loop);
	~Epoller();

	int wait(ChannelLists channel_list, int timeout);

	void setChannel(Channel *channel);
	void delChannel(Channel *channel);

	Epoller(const Epoller &) = delete;
	Epoller &operator=(const Epoller &) = delete;
}; // class Epoller

} // namespace ev

#endif // _EPOLLER_HPP__