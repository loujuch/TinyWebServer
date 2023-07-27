#include "timer_id.hpp"

#include <assert.h>

ev::TimerId::TimerId(Timer *timer) :
	own_timer_(timer) {
}