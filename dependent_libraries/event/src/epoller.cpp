#include "epoller.hpp"

#include <iostream>

#include "channel.hpp"
#include "event.hpp"
#include "log.hpp"

#include <sys/epoll.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

static int safe_close(int fd) {
	int out = fd;
	if(fd >= 0) {
		out = ::close(fd);
		if(out) {
			logger::log_error << "epoll file " << fd << " close fail: " << "errno = " << errno
				<< ' ' << strerror(errno);
		} else {
			logger::log_info << "epoll file " << fd << " close";
		}
		fd = -1;
	} else {
		logger::log_warn << "epoll file fd = " << fd << " had closed";
	}
	return out;
}

uint32_t poller_to_epoll(ev::pooler_event_t event) {
	uint32_t out = 0;
	out |= (event & ev::EV_POOLER_READ) ? EPOLLIN : 0;
	out |= (event & ev::EV_POOLER_WRITE) ? EPOLLOUT : 0;
	out |= (event & ev::EV_POOLER_ERROR) ? EPOLLERR : 0;
	out |= (event & ev::EV_POOLER_HUP) ? EPOLLHUP : 0;
	out |= (event & ev::EV_POOLER_RDHUP) ? EPOLLRDHUP : 0;
	out |= (event & ev::EV_POOLER_PRI) ? EPOLLPRI : 0;
	return out;
}

ev::pooler_event_t epoll_to_poller(uint32_t event) {
	ev::pooler_event_t out = ev::EV_POOLER_NONE;
	out |= (event & EPOLLIN) ? ev::EV_POOLER_READ : 0;
	out |= (event & EPOLLOUT) ? ev::EV_POOLER_WRITE : 0;
	out |= (event & EPOLLERR) ? ev::EV_POOLER_ERROR : 0;
	out |= (event & EPOLLHUP) ? ev::EV_POOLER_HUP : 0;
	out |= (event & EPOLLRDHUP) ? ev::EV_POOLER_RDHUP : 0;
	out |= (event & EPOLLPRI) ? ev::EV_POOLER_PRI : 0;
	return out;
}

ev::Epoller::Epoller(EventLoop *event_loop) :
	Poller(event_loop),
	epoll_fd_(epoll_create1(EPOLL_CLOEXEC)) {
	if(epoll_fd_ < 0) {
		logger::log_fatal << "epoll_fd create fail";
		return;
	}
	assert(epoll_fd_ >= 0);
	logger::log_info << "epoll fd = " << epoll_fd_ << " create";
}

ev::Epoller::~Epoller() {
	safe_close(epoll_fd_);
}

int ev::Epoller::wait(ChannelLists channel_list, int timeout) {
	struct epoll_event events[pooler_events_size];
	int nfds = epoll_wait(epoll_fd_, events, pooler_events_size, timeout);
	for(int i = 0;i < nfds;++i) {
		Channel *channel = static_cast<Channel *>(events[i].data.ptr);
		logger::log_trace << "epoller " << this << " get channel "
			<< channel << " event " << events[i].events;
		channel->set_revents(epoll_to_poller(events[i].events));
		channel_list[i] = channel;
	}
	if(nfds > 0) {
		logger::log_info << "epoll fd = " << epoll_fd_ << " nfds = " << nfds;
	} else if(nfds == 0) {
		logger::log_info << "epoll fd = " << epoll_fd_ << " timeout : " << timeout;
	} else {
		logger::log_error << "epoll fd = " << epoll_fd_
			<< " errno = " << errno << ' ' << strerror(errno);
	}
	return nfds;
}

void ev::Epoller::setChannel(Channel *channel) {
	if(channel->event_loop() != owner_event_loop_) {
		logger::log_error << "epoller " << epoll_fd_ << " in event_loop " << owner_event_loop_
			<< " set other event_loop " << channel->event_loop() << " channel " << channel;
		return;
	}
	if(channel->event() == EV_POOLER_NONE) {
		logger::log_warn << "epoller " << epoll_fd_ << " in event_loop " << owner_event_loop_
			<< " set channel " << channel << " not event";
	}
	assert(channel->event() != EV_POOLER_NONE);
	struct epoll_event event;
	event.data.ptr = channel;
	event.events = poller_to_epoll(channel->event());
	logger::log_trace << "epoller " << this << " set channel "
		<< channel << " event " << event.events;
	if(channel_map_.count(channel->fd()) == 0) {
		// 添加
		logger::log_info << "epoll fd = " << epoll_fd_ << " add channel fd = " << channel->fd();
		epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, channel->fd(), &event);
		channel_map_.emplace(channel->fd(), channel);
	} else {
		// 修改
		logger::log_info << "epoll fd = " << epoll_fd_ << " mod channel fd = " << channel->fd();
		epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, channel->fd(), &event);
		channel_map_[channel->fd()] = channel;
	}
}

void ev::Epoller::delChannel(Channel *channel) {
	logger::log_info << "epoll fd = " << epoll_fd_ << " try del channel fd = " << channel->fd();
	if(channel->event_loop() != owner_event_loop_) {
		logger::log_error << "epoller " << epoll_fd_ << " in event_loop " << owner_event_loop_
			<< " set other event_loop " << channel->event_loop() << " channel " << channel;
		return;
	}
	if(!hasChannel(channel)) {
		logger::log_error << "epoller " << epoll_fd_ << " in event_loop " << owner_event_loop_
			<< " del channel " << channel << " is not in epoller";
		return;
	}
	channel_map_.erase(channel->fd());
	epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, channel->fd(), nullptr);
	logger::log_info << "epoll fd = " << epoll_fd_ << " del channel fd = " << channel->fd();
}