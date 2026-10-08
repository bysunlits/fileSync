//
// Created by wx on 2026/10/7.
//

#pragma once
#ifndef FILE_SYNC_SERVER_LOGGER_H
#define FILE_SYNC_SERVER_LOGGER_H

#endif //FILE_SYNC_SERVER_LOGGER_H
#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

// 日志级别
typedef enum {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_NONE // 用于关闭所有日志
} LogLevel;

// 传递给 Sink (输出目标) 的元数据结构体
typedef struct {
    LogLevel level;
    const char *module;
    const char *file;
    const char *func;
    int line;
    struct tm timestamp;
} LogEvent;

// 核心解耦点：日志输出目标的函数指针 (Sink 回调)
typedef void (*LogSinkFn)(const LogEvent *event, const char *fmt, va_list args, void *user_data);

// 初始化与配置 API
void log_set_level(LogLevel level);
void log_set_sink(LogSinkFn sink, void *user_data);
void log_dispatch(const LogEvent *event, const char *fmt, ...);

// 默认模块名（如果源文件未定义）
#ifndef LOG_MODULE
#define LOG_MODULE "GLOBAL"
#endif

// 核心宏：捕获元数据并触发派发
// 提示：通过宏可以在编译期消除低于某个级别的日志以提升性能
#define LOG_BASE(level_enum, ...) do { \
    extern LogLevel g_current_log_level; \
    if ((level_enum) >= g_current_log_level) { \
        time_t t = time(NULL); \
        struct tm tm_info; \
        localtime_r(&t, &tm_info); \
        LogEvent event = { \
            .level = level_enum, \
            .module = LOG_MODULE, \
            .file = __FILE__, \
            .func = __func__, \
            .line = __LINE__, \
            .timestamp = tm_info \
        }; \
        log_dispatch(&event, __VA_ARGS__); \
    } \
} while (0)

// 供业务代码调用的正交接口
#define LOG_TRACE(...) LOG_BASE(LOG_LEVEL_TRACE, __VA_ARGS__)
#define LOG_DEBUG(...) LOG_BASE(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...)  LOG_BASE(LOG_LEVEL_INFO,  __VA_ARGS__)
#define LOG_WARN(...)  LOG_BASE(LOG_LEVEL_WARN,  __VA_ARGS__)
#define LOG_ERROR(...) LOG_BASE(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LOG_BASE(LOG_LEVEL_FATAL, __VA_ARGS__)

#endif // LOGGER_H