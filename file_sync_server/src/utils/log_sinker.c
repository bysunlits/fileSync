//
// Created by wx on 2026/10/7.
//

#include "log_sinker.h"
#include <stdio.h>
#include <stdbool.h>

// 平台兼容头文件（用于 isatty 和 线程锁）
#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <windows.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#include <pthread.h>
#endif

// ANSI 终端颜色代码
#define COLOR_RESET   "\033[0m"
#define COLOR_GRAY    "\033[90m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RED     "\033[31m"
#define COLOR_MAGENTA "\033[35m"


void file_sink(const LogEvent *event, const char *fmt, va_list args, void *user_data) {
    FILE *fp = (FILE *)user_data;
    if (!fp) return;

    fprintf(fp, "[%02d:%02d:%02d] [%s] ",
            event->timestamp.tm_hour, event->timestamp.tm_min, event->timestamp.tm_sec,
            event->module);
    vfprintf(fp, fmt, args);
    fprintf(fp, "\n");
    fflush(fp);
}

typedef struct {
    const char *name;
    const char *color;
} LevelStyle;

// 各级别的名称与配色映射
static const LevelStyle level_styles[] = {
    [LOG_LEVEL_TRACE] = { "TRACE", COLOR_GRAY    },
    [LOG_LEVEL_DEBUG] = { "DEBUG", COLOR_CYAN    },
    [LOG_LEVEL_INFO]  = { "INFO ", COLOR_GREEN   },
    [LOG_LEVEL_WARN]  = { "WARN ", COLOR_YELLOW  },
    [LOG_LEVEL_ERROR] = { "ERROR", COLOR_RED     },
    [LOG_LEVEL_FATAL] = { "FATAL", COLOR_MAGENTA },
};

#if !(defined(_WIN32) || defined(_WIN64))
static pthread_mutex_t g_console_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void console_sink(const LogEvent *event, const char *fmt, va_list args, void *user_data) {


    ConsoleSinkConfig *cfg = (ConsoleSinkConfig *)user_data;

    // 1. 错误级别分流：WARN 及以上走 stderr，其余走 stdout
    FILE *stream = (event->level >= LOG_LEVEL_WARN) ? stderr : stdout;

    bool use_color = (isatty(fileno(stream)) != 0);

    // 2. 如果调用方传了配置，按需覆盖默认行为
    if (cfg) {
        if (cfg->disable_color) {
            use_color = false;
        } else if (cfg->force_color) {
            use_color = true;
        }
    }

    // 3. 格式化时间戳
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &event->timestamp);

    LevelStyle style = level_styles[event->level];

    // 4. 线程安全保护：避免多线程并发输出时字符交叉混杂
#if defined(_WIN32) || defined(_WIN64)
    // Windows 下如需严格线程安全可使用 CRITICAL_SECTION，此处省略
#else
    pthread_mutex_lock(&g_console_mutex);
#endif

    // 5. 格式化打印前缀
    if (use_color) {
        // 带 ANSI 彩色输出
        fprintf(stream, "%s[%s]%s %s[%s]%s %s[%-6s]%s \033[90m(%s:%d %s)\033[0m: ",
                COLOR_GRAY, time_buf, COLOR_RESET,
                style.color, style.name, COLOR_RESET,
                COLOR_CYAN, event->module, COLOR_RESET,
                event->file, event->line, event->func);
    } else {
        // 无颜色纯文本输出
        fprintf(stream, "[%s] [%s] [%-6s] (%s:%d %s): ",
                time_buf,
                style.name,
                event->module,
                event->file,
                event->line,
                event->func);
    }

    // 6. 打印用户传入的具体内容
    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
    fflush(stream);

#if !(defined(_WIN32) || defined(_WIN64))
    pthread_mutex_unlock(&g_console_mutex);
#endif
}