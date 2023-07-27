#include "log_formatter.hpp"

#include <algorithm>
#include <vector>

#include "log_config.hpp"

#include <cinttypes>

inline std::string get_log_level_str(logger::LogLevel repeat) {
	static std::string log_level_str[8] = {
		"OFF", "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE", "ALL"
	};
	return log_level_str[static_cast<unsigned>(repeat)];
}

template <typename T>
void formatInteger(std::string &s, T integer) {
	if(!integer) {
		s.push_back('0');
		return;
	}
	static const std::string digit("0123456789");
	bool is_min = false;
	if(integer < 0) {
		s.push_back('-');
		if(0 == (integer << 1)) {
			// 全为1的二进制正数转化为十进制数时末尾不会为9，
			// 故将`X_MIN`转化为`X_MAX`，之后再将其末尾加上1即可
			is_min = true;
			integer = ~integer;
		} else {
			integer = -integer;
		}
	}
	uint64_t size = s.size();
	while(integer) {
		s.push_back(digit[integer % 10]);
		integer /= 10;
	}
	std::reverse(s.begin() + size, s.end());
	s.back() += is_min;
}

template <typename T>
void formatFloat(std::string &s, T f) {
	char buffer[64];
	static constexpr T type_limit_min = 1e-4;
	static constexpr T type_limit_max = 1e7;
	if(f == 0.0) {
		buffer[0] = '0';
		buffer[1] = '\0';
	} else if((f >= type_limit_min && f < type_limit_max) ||
		(f > -type_limit_max && f <= -type_limit_min)) {
		int n = snprintf(buffer, sizeof(buffer), "%f", f) - 1;
		while(buffer[n] == '0') {
			--n;
		}
		buffer[n + (buffer[n] != '.')] = '\0';
	} else {
		int e = snprintf(buffer, sizeof(buffer), "%e", f) - 1;
		while(buffer[e] != 'e') {
			--e;
		}
		int n = e - 1;
		while(buffer[n] == '0') {
			--n;
		}
		n += buffer[n] != '.';
		if(e != n + 1) {
			while(buffer[e] != '\0') {
				buffer[n++] = buffer[e++];
			}
			buffer[n] = '\0';
		}
	}
	s.append(buffer);
}

template <typename T>
void formatPointer(std::string &s, T f) {
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%p", f);
	s.append(buffer);
}

logger::LogFormatter::LogFormatter(std::string &s, LogLevel level) :format_str_(s), level_(level) {
}

logger::LogFormatter &logger::LogFormatter::operator<<(const bool t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	static std::string bool_str[2] = { "false", "true" };
	format_str_.append(bool_str[t]);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const logger::LogLevel level) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	format_str_.append(get_log_level_str(level));
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const char t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	format_str_.push_back(t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const unsigned char t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	format_str_.push_back(t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const int16_t t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	formatInteger(format_str_, t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const uint16_t t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	formatInteger(format_str_, t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const int32_t t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	formatInteger(format_str_, t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const uint32_t t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	formatInteger(format_str_, t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const int64_t t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	formatInteger(format_str_, t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const uint64_t t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	formatInteger(format_str_, t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const float t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	formatFloat(format_str_, t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const double t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	formatFloat(format_str_, t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const char *t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	format_str_.append(t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const void *t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	formatPointer(format_str_, t);
	return *this;
}

logger::LogFormatter &logger::LogFormatter::operator<<(const std::string &t) {
	if(!LogConfig::getInstance().check_level(level_)) {
		return *this;
	}
	format_str_.append(t);
	return *this;
}