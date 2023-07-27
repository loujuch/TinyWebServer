#include "log_timestamp.hpp"

#include <sys/timeb.h>

logger::LogTimeStamp::LogTimeStamp() {
}

std::string logger::LogTimeStamp::get_time_stamp_str() {
	static const std::string digit("0123456789");
	char buffer[24];
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S.", gmtime(&(t.tv_sec)));
	t.tv_nsec /= 1000000;
	buffer[22] = digit[t.tv_nsec % 10];
	t.tv_nsec /= 10;
	buffer[21] = digit[t.tv_nsec % 10];
	t.tv_nsec /= 10;
	buffer[20] = digit[t.tv_nsec % 10];
	buffer[23] = '\0';
	return buffer;
}

std::string logger::LogTimeStamp::get_time_stamp_str_in_file_name() {
	char buffer[36];
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S_", gmtime(&(t.tv_sec)));
	snprintf(buffer + 20, sizeof(buffer) - 20, "%lu", t.tv_nsec % 10000000000);
	return buffer;
}