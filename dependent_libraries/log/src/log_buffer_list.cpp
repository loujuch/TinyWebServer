#include "log_buffer_list.hpp"

#include <iostream>

#include <cstring>

logger::LogBufferList::LogBufferList() :front_(new LogBuffer), cur_(front_), back_(cur_) {
}

bool logger::LogBufferList::append(const std::string &s) {
	bool out = false;
	char *buffer = nullptr;
	assign_mutex_.lock();
	if((buffer = back_->assign(s)) == nullptr) {
		add_new();
		out = true;
		if((buffer = back_->assign(s)) == nullptr) {
			std::cerr << __FILE__ << ' ' << __func__ << ' ' << __LINE__
				<< ": append line logger fail\n";
		}
	}
	assign_mutex_.unlock();
	::memcpy(buffer, s.c_str(), s.size());
	return out;
}