#include "http_head_parser.hpp"

#include "log.hpp"

#include <algorithm>

static int get_line(const char *buffer, int len, int start) {
	if(len <= 0 || start < 0 || buffer == nullptr) {
		logger::log_warn << "buffer " << buffer << " length " << len << " start " << start;
		return -1;
	}
	int s, status = 0;
	for(s = start;s < len;++s) {
		char c = buffer[s];
		if(c == '\r') {
			status = 1;
		} else if(c == '\n' && status == 1) {
			return s + 1;
		} else {
			status = 0;
		}
	}
	return -1;
}

web::HttpHeadParser::HttpHeadParser() :
	line_(0),
	finish_(false),
	error_(false) {
}

int web::HttpHeadParser::parser_range(std::vector<std::pair<off_t, off_t>> &range) {
	auto p = field_.find("Range");
	if(p == field_.end()) {
		return -1;
	}
	auto &s = p->second;
	int n = -1;
	for(int i = 0;i < s.size();++i) {
		if(s[i] == '=') {
			n = i;
			break;
		}
	}
	if(n == -1) {
		return 0;
	}
	int ks = 0, ke = n - 1, vs = n + 1, ve = s.size() - 1;
	while(isspace(s[ks]) && ks <= ke) {
		++ks;
	}
	while(isspace(s[ke]) && ks <= ke) {
		--ke;
	}
	while(isspace(s[vs]) && vs <= ve) {
		++vs;
	}
	while(isspace(s[ve]) && vs <= ve) {
		--ve;
	}
	if(ks > ke || vs > ve || s.substr(ks, ke - ks + 1) != "bytes") {
		error_ = true;
		return 0;
	}
	bool all = false;
	int status = 0;
	off_t left = 0, right = 0;
	range.clear();
	for(int i = vs;i <= ve;++i) {
		char c = s[i];
		if(c == ' ') {
			continue;
		}
		switch(status) {
			case 0:
				if(c >= '0' && c <= '9') {
					all = false;
					left *= 10;
					left += c - '0';
				} else if(c == '-') {
					all = true;
					status = 1;
				} else {
					error_ = 1;
					return 0;
				}
				break;
			case 1:
				if(c >= '0' && c <= '9') {
					all = false;
					right *= 10;
					right += c - '0';
				} else if(c == ',') {
					if(left > right) {
						error_ = -1;
						return 0;
					}
					all = false;
					range.emplace_back(left, right);
					left = 0;
					right = 0;
					status = 0;
				} else {
					error_ = 1;
					return 0;
				}
				break;
			default:
				error_ = true;
				return 0;
		}
	}
	if(all) {
		range.emplace_back(left, -1);
	} else if(status == 1) {
		if(left > right) {
			error_ = -1;
			return 0;
		}
		range.emplace_back(left, right);
	} else {
		error_ = true;
		return 0;
	}
	std::sort(range.begin(), range.end(), [](
		const std::pair<off_t, off_t> &a,
		const std::pair<off_t, off_t> &b) {
			if(a.second == b.second) {
				return a.second < b.second;
			}
			return a.first < b.first;
		}
	);
	return range.size();
}

void web::HttpHeadParser::commit(const char *buffer, int len) {
	if(len < 2 || buffer == nullptr) {
		logger::log_warn << "buffer " << buffer << " length " << len;
		return;
	}
	if(buffer[len - 2] != '\r' || buffer[len - 1] != '\n') {
		return;
	}
	if(len == 2) {
		finish_ = true;
		return;
	}
	len -= 2;
	if(line_ == 0) {
		bool isback = true;
		int now = 0, s = 0;
		for(int i = 0;i < len;++i) {
			if(!isspace(buffer[i])) {
				if(isback) {
					s = i;
				}
				isback = false;
			} else {
				if(!isback) {
					if(now == 0) {
						method_.append(buffer + s, i - s);
					} else if(now == 1) {
						file_path_.append(buffer + s, i - s);
					} else if(now == 2) {
						version_.append(buffer + s, i - s);
					} else {
						error_ = true;
						return;
					}
					++now;
				}
				isback = true;
			}
		}
		if(!isback) {
			if(now == 0) {
				method_.append(buffer + s, len - s);
			} else if(now == 1) {
				file_path_.append(buffer + s, len - s);
			} else if(now == 2) {
				version_.append(buffer + s, len - s);
			} else {
				error_ = true;
				return;
			}
		}
		++now;
		if(now != 3) {
			error_ = true;
			return;
		}
		int w = -1;
		for(int i = 0;i < file_path_.size();++i) {
			if(file_path_[i] == '?') {
				w = i;
				break;
			}
		}
		if(w != -1) {
			if(w + 1 == file_path_.size()) {
				file_path_.pop_back();
			} else {
				query_ = file_path_.substr(w + 1);
				file_path_ = file_path_.substr(0, w);
			}
		}
		++line_;
	} else {
		int n = -1;
		for(int i = 0;i < len;++i) {
			if(buffer[i] == ':') {
				n = i;
				break;
			}
		}
		if(n == -1) {
			error_ = true;
			return;
		}
		int ks = 0, ke = n - 1, vs = n + 1, ve = len - 1;
		while(isspace(buffer[ks]) && ks <= ke) {
			++ks;
		}
		while(isspace(buffer[ke]) && ks <= ke) {
			--ke;
		}
		while(isspace(buffer[vs]) && vs <= ve) {
			++vs;
		}
		while(isspace(buffer[ve]) && vs <= ve) {
			--ve;
		}
		if(ks > ke || vs > ve) {
			error_ = true;
			return;
		}
		field_.emplace(
			std::string(buffer + ks, buffer + ke + 1),
			std::string(buffer + vs, buffer + ve + 1));
	}
}

bool web::HttpHeadParser::parser(const char *buffer, int size) {
	if(size <= 0 || buffer == nullptr) {
		logger::log_warn << "buffer " << buffer << " size " << size;
		return error_;
	}
	if(error_) {
		return true;
	}
	if(finish_) {
		leave_data_.append(buffer, size);
		return error_;
	}
	int start = 0, e;
	if(!leave_data_.empty()) {
		if(buffer[start] == '\n' && leave_data_.back() == '\r') {
			leave_data_.push_back('\n');
			commit(leave_data_.c_str(), leave_data_.size());
			leave_data_.clear();
			++start;
		} else {
			e = get_line(buffer + start, size, start);
			if(e == -1) {
				leave_data_.append(buffer + start, size);
				return error_;
			}
			leave_data_.append(buffer + start, e - start);
			commit(leave_data_.c_str(), leave_data_.size());
			leave_data_.clear();
			start = e;
		}
	}
	if(error_) {
		return error_;
	}
	while((e = get_line(buffer, size, start)) > start) {
		commit(buffer + start, e - start);
		if(error_) {
			return error_;
		}
		start = e;
		if(finish_) {
			break;
		}
	}
	if(start < size) {
		leave_data_.append(buffer + start, size - start);
	}
	return error_;
}