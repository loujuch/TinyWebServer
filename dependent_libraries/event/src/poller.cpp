#include "poller.hpp"

#include "channel.hpp"
#include "epoller.hpp"
#include "event_loop.hpp"

#include <assert.h>

ev::Poller::Poller(ev::EventLoop *event_loop) :
	owner_event_loop_(event_loop) {
	assert(owner_event_loop_ != nullptr);
}

ev::Poller::~Poller() {
	assert(owner_event_loop_ != nullptr);
}

ev::Poller *ev::Poller::new_default_poller() {
	assert(EventLoop::this_thread_event_loop() != nullptr);
	return new Epoller(EventLoop::this_thread_event_loop());
}

bool ev::Poller::hasChannel(Channel *channel) const {
	return channel_map_.count(channel->fd());
}