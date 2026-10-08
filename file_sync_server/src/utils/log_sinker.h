//
// Created by wx on 2026/10/7.
//

#ifndef FILE_SYNC_SERVER_LOG_SINKER_H
#define FILE_SYNC_SERVER_LOG_SINKER_H

#endif //FILE_SYNC_SERVER_LOG_SINKER_H

#include "logger/logger.h"
#include <stdio.h>
#include <stdbool.h>

void file_sink(const LogEvent *event, const char *fmt, va_list args, void *user_data);

void console_sink(const LogEvent *event, const char *fmt, va_list args, void *user_data);


// 控制台配置项（可选，通过 user_data 传入，传 NULL 使用默认行为）
typedef struct {
    bool force_color;   // 强制开启颜色（即使重定向到文件）
    bool disable_color; // 彻底禁用颜色
} ConsoleSinkConfig;
