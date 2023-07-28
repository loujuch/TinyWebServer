#include "timer_heap.hpp"

#include "timer_id.hpp"
#include "log.hpp"

#include <assert.h>

inline int64_t left(std::shared_ptr<ev::Timer> parent_seq) {
	if(parent_seq == nullptr) {
		logger::log_fatal << "parent_seq == nullptr";
		return -1;
	}
	return ((parent_seq->seq() + 1) << 1) - 1;
}

inline int64_t right(std::shared_ptr<ev::Timer> parent_seq) {
	if(parent_seq == nullptr) {
		logger::log_fatal << "parent_seq == nullptr";
		return -1;
	}
	return ((parent_seq->seq() + 1) << 1);
}

inline int64_t parent(std::shared_ptr<ev::Timer> child) {
	if(child == nullptr) {
		logger::log_fatal << "child == nullptr";
		return -1;
	}
	return ((child->seq() + 1) >> 1) - 1;
}

void ev::TimerHeap::from_to(int l, int r) {
	std::swap(timer_heap_[l], timer_heap_[r]);
	std::swap(timer_heap_[l]->seq_, timer_heap_[r]->seq_);
}

bool ev::TimerHeap::push(std::shared_ptr<Timer> &timer) {
	if(timer == nullptr) {
		logger::log_error << "timer ptr nullptr";
		return false;
	}
	if(timer->seq() >= 0) {
		logger::log_warn << "timer ptr = " << timer.get() << " has in heap";
		return false;
	}
	logger::log_info << "timer ptr = " << timer.get() << " push in heap";
	timer->seq_ = timer_heap_.size();
	timer_heap_.emplace_back(timer);
	int site = timer->seq();
	while(site != 0) {
		int next = parent(timer_heap_[site]);
		if(timer_heap_[next]->timer_stamp() <= timer_heap_[site]->timer_stamp()) {
			break;
		}
		from_to(site, next);
		site = next;
	}
	return true;
}

ev::TimerId ev::TimerHeap::emplace(const TimerStamp &when,
	uint64_t interval, TimerCallback timer_callback) {
	std::shared_ptr<Timer> timer(new Timer(when, interval, timer_callback));
	logger::log_info << "timer ptr = " << timer.get() << " create";
	bool is = push(timer);
	assert(is);
	return TimerId(timer.get());
}

const std::shared_ptr<ev::Timer> ev::TimerHeap::top() const {
	return timer_heap_.empty() ? nullptr : timer_heap_.front();
}

void ev::TimerHeap::pop() {
	bool is = erase(timer_heap_.front().get());
	assert(is);
}

bool ev::TimerHeap::erase(Timer *timer) {
	if(timer == nullptr || timer->seq() < 0 || timer->seq() >= timer_heap_.size()) {
		logger::log_warn << "timer ptr = " << timer << " has not in heap";
		return false;
	}
	logger::log_info << "timer ptr = " << timer << " delete";
	int root = timer->seq();
	timer->seq_ = -1;
	while(root < timer_heap_.size()) {
		int lchild = left(timer_heap_[root]);
		int rchild = right(timer_heap_[root]);
		if(lchild >= timer_heap_.size()) {
			break;
		}
		if(rchild >= timer_heap_.size()) {
			timer_heap_[root] = timer_heap_[lchild];
			timer_heap_[root]->seq_ = root;
			root = lchild;
			break;
		}
		if(timer_heap_[lchild]->timer_stamp() < timer_heap_[rchild]->timer_stamp()) {
			timer_heap_[root] = timer_heap_[lchild];
			timer_heap_[root]->seq_ = root;
			root = lchild;
		} else {
			timer_heap_[root] = timer_heap_[rchild];
			timer_heap_[root]->seq_ = root;
			root = rchild;
		}
	}
	if(root + 1 < timer_heap_.size()) {
		timer_heap_[root] = timer_heap_.back();
		timer_heap_[root]->seq_ = root;
	}
	timer_heap_.pop_back();
	return true;
}

bool ev::TimerHeap::updateTimer(Timer *timer, uint64_t delay) {
	if(timer == nullptr || timer->seq() < 0 || timer->seq() >= timer_heap_.size()) {
		return false;
	}
	int root = timer->seq();
	timer_heap_[root]->timer_stamp_.delay_time(delay);
	while(root < timer_heap_.size()) {
		int lchild = left(timer_heap_[root]);
		int rchild = right(timer_heap_[root]);
		if(lchild >= timer_heap_.size()) {
			break;
		}
		if(rchild >= timer_heap_.size()) {
			if(timer_heap_[root]->timer_stamp() > timer_heap_[lchild]->timer_stamp()) {
				timer_heap_[root] = timer_heap_[lchild];
				timer_heap_[root]->seq_ = root;
				root = lchild;
			}
			break;
		}
		if(timer_heap_[lchild]->timer_stamp() < timer_heap_[rchild]->timer_stamp()) {
			if(timer_heap_[root]->timer_stamp() > timer_heap_[lchild]->timer_stamp()) {
				timer_heap_[root] = timer_heap_[lchild];
				timer_heap_[root]->seq_ = root;
				root = lchild;
			} else {
				break;
			}
		} else {
			if(timer_heap_[root]->timer_stamp() > timer_heap_[rchild]->timer_stamp()) {
				timer_heap_[root] = timer_heap_[rchild];
				timer_heap_[root]->seq_ = root;
				root = rchild;
			} else {
				break;
			}
		}
	}
	return true;
}