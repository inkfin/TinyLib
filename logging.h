#ifndef TL_LOGGING_H
#define TL_LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define todo(cmt) \
    assert(0 && cmt);

typedef enum {
    TL_INFO,
    TL_WARNING,
    TL_ERROR,
} TL_Log_Level;

typedef struct {
    va_list ap;
    const char* fmt;
    const char* file;
    struct tm* time;
    void* udata;
    int line;
    int level;
} TL_Log_Event;

typedef void (*TL_Log_LogFn)(TL_Log_Event* ev);
typedef void (*TL_Log_LockFn)(bool lock, void* udata);

void log_set_lock(TL_Log_LockFn fn, void* udata);
void log_set_level(int level);
void log_set_quiet(bool enable);
int log_add_callback(TL_Log_LogFn fn, void* udata, int level);
int log_add_fp(FILE* fp, int level);

void tl_log(TL_Log_Level level, const char* file, int line, const char* format, ...);

#define TL_LOG(level, format, ...) \
    tl_log(level, __FILE__, __LINE__, format, __VA_ARGS__)

#ifdef TL_LOGGING_IMPL
#undef TL_LOGGING_IMPL

#define MAX_CALLBACKS 32

typedef struct {
    TL_Log_LogFn fn;
    void* udata;
    int level;
} TL_Log_Callback;

static struct {
    void* udata;
    TL_Log_LockFn lock;
    int level;
    bool quiet;
    TL_Log_Callback callbacks[MAX_CALLBACKS];
} TL_Log_L;

static const char* _tl_level_strings[] = {
    "[INFO]", "[WARNING]", "[ERROR]"
};

#ifndef LOG_DISABLE_COLOR
static const char* _tl_level_colors[] = {
    "\x1b[32m", "\x1b[33m", "\x1b[31m"
};
#endif

static void init_log_event(TL_Log_Event* ev, void* udata)
{
    if (!ev->time) {
        time_t t = time(NULL);
        localtime_s(ev->time, &t);
    }
    ev->udata = udata;
}

static void _tl_log_stdout_callback(TL_Log_Event* ev)
{
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
#ifndef LOG_DISABLE_COLOR
    fprintf(
        ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
        buf, _tl_level_colors[ev->level], _tl_level_strings[ev->level],
        ev->file, ev->line);
#else
    fprintf(
        ev->udata, "%s %-5s %s:%d: ",
        buf, level_strings[ev->level], ev->file, ev->line);
#endif
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void _tl_log_lock(void)
{
    if (TL_Log_L.lock) {
        TL_Log_L.lock(true, TL_Log_L.udata);
    }
}

static void _tl_log_unlock(void)
{
    if (TL_Log_L.lock) {
        TL_Log_L.lock(false, TL_Log_L.udata);
    }
}

static void _tl_log_file_callback(TL_Log_Event* ev)
{
    char buf[64];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    fprintf(
        ev->udata, "%s %-5s %s:%d: ",
        buf, _tl_level_strings[ev->level], ev->file, ev->line);
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

void log_set_lock(TL_Log_LockFn fn, void* udata)
{
    TL_Log_L.lock = fn;
    TL_Log_L.udata = udata;
}

void log_set_level(int level)
{
    TL_Log_L.level = level;
}

void log_set_quiet(bool enable)
{
    TL_Log_L.quiet = enable;
}

int log_add_callback(TL_Log_LogFn fn, void* udata, int level)
{
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!TL_Log_L.callbacks[i].fn) {
            TL_Log_L.callbacks[i] = (TL_Log_Callback) { fn, udata, level };
            return 0;
        }
    }
    return -1;
}

int log_add_fp(FILE* fp, int level)
{
    return log_add_callback(_tl_log_file_callback, fp, level);
}

void tl_log(TL_Log_Level level, const char* file, int line, const char* fmt, ...)
{
    struct tm timeinfo = { 0 };
    TL_Log_Event ev = {
        .fmt = fmt,
        .file = file,
        .line = line,
        .level = level,
        .time = &timeinfo,
    };

    _tl_log_lock();

    if (!TL_Log_L.quiet && level >= TL_Log_L.level) {
        init_log_event(&ev, stderr);
        va_start(ev.ap, fmt);
        _tl_log_stdout_callback(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < MAX_CALLBACKS && TL_Log_L.callbacks[i].fn; i++) {
        TL_Log_Callback* cb = &TL_Log_L.callbacks[i];
        if (level >= cb->level) {
            init_log_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    _tl_log_unlock();
}

#undef MAX_CALLBACKS

#endif // TL_LOGGING_IMPL

#ifdef __cplusplus
}
#endif

#endif // TL_LOGGING_H
