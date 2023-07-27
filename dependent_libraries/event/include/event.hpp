#ifndef _EVENT_HPP__
#define _EVENT_HPP__

namespace ev {

class Channel;

using pooler_event_t = unsigned;

constexpr pooler_event_t EV_POOLER_NONE = 0x0;
constexpr pooler_event_t EV_POOLER_READ = 0x1;
constexpr pooler_event_t EV_POOLER_WRITE = 0x2;
constexpr pooler_event_t EV_POOLER_ERROR = 0x4;
constexpr pooler_event_t EV_POOLER_HUP = 0x8;
constexpr pooler_event_t EV_POOLER_RDHUP = 0x10;
constexpr pooler_event_t EV_POOLER_NVAL = 0x20;
constexpr pooler_event_t EV_POOLER_PRI = 0x40;

constexpr unsigned pooler_events_size = 1024;
using ChannelLists = Channel * [pooler_events_size];
} // namespace ev

#endif // _EVENT_HPP__