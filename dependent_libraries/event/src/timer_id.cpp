#include "timer_id.hpp"

ev::TimerId::TimerId(Timer *timer) :
	own_timer_(timer) {
}