#ifndef _CALLBACK_HPP__
#define _CALLBACK_HPP__

#include <functional>
#include <vector>
#include <memory>

namespace ev {

class EventLoop;
class Channel;

using ReadCallback = std::function<void()>;
using WriteCallback = std::function<void()>;
using ErrorCallback = std::function<void()>;
using CloseCallback = std::function<void()>;
using DeleteCallback = std::function<void()>;

using TimerCallback = std::function<void()>;
using SignalCallback = std::function<void()>;

using FreeCallback = std::function<void()>;

using ChannelArgs = std::vector<std::unique_ptr<Channel>>;
using ThreadInitCallback = std::function<void(EventLoop &,
	ChannelArgs &)>;

} // namespace ev

#endif // _CALLBACK_HPP__