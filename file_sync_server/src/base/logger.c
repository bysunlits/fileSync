//
// Created by wx on 2026/10/7.
//

#include "../../libs/logger/logger.h"
#include <stdio.h>

// 全局日志级别控制
LogLevel g_current_log_level = LOG_LEVEL_INFO;

static LogSinkFn g_sink = NULL;
static void *g_sink_user_data = NULL;

static const char *level_names[] = {"TRACE", "DEBUG", "INFO",
				    "WARN",  "ERROR", "FATAL"};

// 默认 Sink：输出到终端
static void default_stdout_sink(const LogEvent *event, const char *fmt,
				va_list args, void *user_data)
{
	(void)user_data;

	char time_buf[20];
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S",
		 &event->timestamp);

	fprintf(stdout, "[%s] [%-5s] [%-8s] (%s:%d %s): ", time_buf,
		level_names[event->level], event->module, event->file,
		event->line, event->func);

	vfprintf(stdout, fmt, args);
	fprintf(stdout, "\n");
	fflush(stdout);
}

void log_set_level(LogLevel level)
{
	g_current_log_level = level;
}

void log_set_sink(LogSinkFn sink, void *user_data)
{
	g_sink = sink;
	g_sink_user_data = user_data;
}

void log_dispatch(const LogEvent *event, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	// 如果未设置自定义 sink，走默认输出
	if (g_sink) {
		g_sink(event, fmt, args, g_sink_user_data);
	} else {
		default_stdout_sink(event, fmt, args, NULL);
	}

	va_end(args);
}