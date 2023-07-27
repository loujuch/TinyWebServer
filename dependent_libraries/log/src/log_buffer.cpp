#include "log_buffer.hpp"

#include <string.h>

logger::LogBuffer::LogBuffer() :size_(0) {
}

bool logger::LogBuffer::append(const std::string &s) {
	if(this->free_size() < s.size()) {
		return false;
	}
	::memcpy(buffer_ + size_, s.c_str(), s.size());
	size_ += s.size();
	return true;
}