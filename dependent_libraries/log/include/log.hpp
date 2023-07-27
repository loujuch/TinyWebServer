#ifndef _LOG_HPP__
#define _LOG_HPP__

#include "log_interface.hpp"
#include "log_config.hpp"

// 方便设置日志等级

#ifndef LOG_OFF
#define LOG_OFF LogLevel::LOG_OFF
#endif // LOG_OFF

#ifndef LOG_FATAL
#define LOG_FATAL LogLevel::LOG_FATAL
#endif // LOG_FATAL

#ifndef LOG_ERROR
#define LOG_ERROR LogLevel::LOG_ERROR
#endif // LOG_ERROR

#ifndef LOG_WARN
#define LOG_WARN LogLevel::LOG_WARN
#endif // LOG_WARN

#ifndef LOG_INFO
#define LOG_INFO LogLevel::LOG_INFO
#endif // LOG_INFO

#ifndef LOG_DEBUG
#define LOG_DEBUG LogLevel::LOG_DEBUG
#endif // LOG_DEBUG

#ifndef LOG_TRACE
#define LOG_TRACE LogLevel::LOG_TRACE
#endif // LOG_TRACE

#ifndef LOG_ALL
#define LOG_ALL LogLevel::LOG_ALL
#endif // LOG_ALL

// 通用日志写入

#ifndef log_write
#define log_write(level) LogInterface(__FILE__, __func__, __LINE__, level).stream()
#endif // log_write

//专用日志写入

#ifndef log_fatal
#define log_fatal log_write(logger::LOG_FATAL)
#endif // log_fatal

// LOG_TRACE = 6,

#ifndef log_error
#define log_error log_write(logger::LOG_ERROR)
#endif // log_error

#ifndef log_warn
#define log_warn log_write(logger::LOG_WARN)
#endif // log_warn

#ifndef log_info
#define log_info log_write(logger::LOG_INFO)
#endif // log_info

#ifndef log_debug
#define log_debug log_write(logger::LOG_DEBUG)
#endif // log_debug

#ifndef log_trace
#define log_trace log_write(logger::LOG_TRACE)
#endif // log_trace

// 设置log基础级别
#ifndef log_set_level
#define log_set_level(level) LogConfig::getInstance().set_base_level(level)
#endif // log_set_level

// 设置log文件目录
#ifndef log_set_dir
#define log_set_dir(dir) LogConfig::getInstance().set_log_dir(dir)
#endif // log_set_dir

// 设置log同步时间
#ifndef log_set_sync_time
#define log_set_sync_time(time) LogConfig::getInstance().set_log_sync_time(time)
#endif // log_set_log_dir

// 设置当前文件名
#ifndef log_set_file_prefix
#define log_set_file_prefix(program) LogConfig::getInstance().set_log_file_prefix(program)
#endif // log_set_file_prefix

// 设置当前文件大小限制
#ifndef log_set_file_size_limit
#define log_set_file_size_limit(size) LogConfig::getInstance().set_log_file_size_limit(size)
#endif // log_set_file_prefix

// 关闭日志库
#ifndef log_close
#define log_close() LogInterface::close()
#endif // log_close

#endif // _LOG_HPP__