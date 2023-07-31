#ifndef _CALLBACK_HPP__
#define _CALLBACK_HPP__

#include <functional>
#include <vector>
#include <memory>

namespace ev {

class EventLoop;
class Channel;

using ReadCallback = std::function<bool()>;
using WriteCallback = std::function<bool()>;
using ErrorCallback = std::function<bool()>;
using CloseCallback = std::function<bool()>;

using TimerCallback = std::function<void()>;
using SignalCallback = std::function<void()>;

using FreeCallback = std::function<void()>;

using ThreadInitCallback = std::function<void(EventLoop &)>;

} // namespace ev

#endif // _CALLBACK_HPP__