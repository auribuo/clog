/*
clog.h v.1.0.0

Simple logging library.
Supports structured logging, logging contexts and filters on that context.
No configuration needed!
LICENSE: See either LICENSE in the github repo or the end of the file
Repo: github.com/auribuo/clog

Example
```
#define CLOG_IMPLEMENTATION // Needed: pull in implementation
#define CLOG_ENABLE_CTX // Needed to use contexts
#define CLOG_STRIP_PREFIX // Optional: Used to strip prefixes
#include "clog.h"

typedef struct {
    const char* name;
    int age;
    int fav_num;
    const char* address;
} Person;

int main(void) {
    set_log_level(debug); // Set the log level (default, info)
    set_color_enabled(true); // Enable colors. In unix you can use set_color_enabled_auto() to auto enable colors for ttys

    Person p = {
        .name = "Max Mustermann",
        .age = 22,
        .fav_num = 42,
        .address = "Via Roma 1"
    };

    // Filters can disable certain contexts. The filter syntax looks like this: [log|nolog]:[<context>|all].
    // Multiple filters can be separated by ';'
    parse_log_filter("nolog:main");
    parse_log_filter_env(); // Filters can be supplied using the CLOG_FILTER env variable. The syntax is the same

    LOG_INFO("This is a test log");
    LOG_INFO("This is a test log with some info added", LOG_ARG_S("name", p.name));
    LOG_WARNING("You can use shorthand args", LOG_ARG_SS(p.address));
    LOG_ERROR("Oh, no!", LOG_ARG_UB("fav_bin_num", p.fav_num)); // Numbers can be formatted differently
    LOG_DEBUG("Debug includes the file and line");
    LOG_INFO_CTX("main", "You can also include contexts to logs"); // Hidden because of filter
}
```
*/

#ifndef __CLOG_H__
#define __CLOG_H__
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef CLOG_MALLOC
#define CLOG_MALLOC malloc
#endif // !CLOG_MALLOC

#define CLOG_CHECK_ALLOC(ptr) \
    if (!ptr) {               \
        panic("Out of RAM."); \
    }

// Color wrappers
#define CLOG_RED(s) "\x1B[31m" s "\x1B[0m"
#define CLOG_GREEN(s) "\x1B[32m" s "\x1B[0m"
#define CLOG_BLUE(s) "\x1B[34m" s "\x1B[0m"
#define CLOG_YELLOW(s) "\x1B[33m" s "\x1B[0m"
#define CLOG_MAGENTA(s) "\x1B[35m" s "\x1B[0m"

// Log level headers
#define CLOG_DBG_HEADER(c) c ? "\033[34m[DBG]\033[0m" : "[DBG]"
#define CLOG_INF_HEADER(c) c ? "\033[32m[INF]\033[0m" : "[INF]"
#define CLOG_WRN_HEADER(c) c ? "\033[33m[WRN]\033[0m" : "[WRN]"
#define CLOG_ERR_HEADER(c) c ? "\033[31m[ERR]\033[0m" : "[ERR]"
#define CLOG_FAT_HEADER(c) c ? "\033[31m[FAT]\033[0m" : "[FAT]"

typedef enum {
    debug,
    info,
    warning,
    error,
    fatal,
    nolog,
} Clog_Log_Level;

typedef enum {
    TERM,
    INT,
    UINT,
    FLOAT,
    STRING,
    CHAR,
    ANY,
} Clog_Arg_Type;

typedef enum {
    DEC,
    BIN,
    HEX
} Clog_Integer_Format;

typedef struct {
    Clog_Arg_Type ty;
    union {
        int64_t i;
        uint64_t u;
        float f;
        const char *s;
        void *a;
    } val;
    Clog_Integer_Format opt;
    char *(*conv_fun)(void *);
} Clog_Arg;

// C++ has no inline struct initialization apparently
static inline Clog_Arg clog_make_log_arg_t();
static inline Clog_Arg clog_make_log_arg_i(int v, Clog_Integer_Format fmt);
static inline Clog_Arg clog_make_log_arg_u(unsigned int v, Clog_Integer_Format fmt);
static inline Clog_Arg clog_make_log_arg_f(float v);
static inline Clog_Arg clog_make_log_arg_s(const char *v);
static inline Clog_Arg clog_make_log_arg_c(char v);
static inline Clog_Arg clog_make_log_arg_a(void *v, char *(*fn)(void *));

void clog_set_log_level(Clog_Log_Level log_level);
#define clog_disable_log() clog_set_log_level(nolog)

void clog_set_color_enabled(bool enabled);
#ifndef _Win32
#include <unistd.h>
#define clog_set_color_auto() clog_set_color_enabled(isatty(STDOUT_FILENO))
#endif // !_Win32
void clog_set_timestamp_enabled(bool enabled);

#define CLOG_LOG_ARG_T \
    NULL, clog_make_log_arg_t()

#define CLOG_LOG_ARG_I(k, v) \
    k, clog_make_log_arg_i(v, NONE)

#define CLOG_LOG_ARG_IS(k) \
    #k, clog_make_log_arg_i(k, NONE)

#define CLOG_LOG_ARG_IB(k, v) \
    k, clog_make_log_arg_i(v, BIN)

#define CLOG_LOG_ARG_ISB(k) \
    #k, clog_make_log_arg_i(k, BIN)

#define CLOG_LOG_ARG_IH(k, v) \
    k, clog_make_log_arg_i(v, HEX)

#define CLOG_LOG_ARG_ISH(k) \
    #k, clog_make_log_arg_i(k, HEX)

#define CLOG_LOG_ARG_U(k, v) \
    k, clog_make_log_arg_u(v, NONE)

#define CLOG_LOG_ARG_US(k) \
    #k, clog_make_log_arg_u(k, NONE)

#define CLOG_LOG_ARG_UB(k, v) \
    k, clog_make_log_arg_u(v, BIN)

#define CLOG_LOG_ARG_USB(k) \
    #k, clog_make_log_arg_u(k, BIN)

#define CLOG_LOG_ARG_UH(k, v) \
    k, clog_make_log_arg_u(v, HEX)

#define CLOG_LOG_ARG_USH(k) \
    #k, clog_make_log_arg_u(k, HEX)

#define CLOG_LOG_ARG_F(k, v) \
    k, clog_make_log_arg_f(v)

#define CLOG_LOG_ARG_FS(k) \
    #k, clog_make_log_arg_f(k)

#define CLOG_LOG_ARG_S(k, v) \
    k, clog_make_log_arg_s((const char *)v)

#define CLOG_LOG_ARG_SS(k) \
    #k, clog_make_log_arg_s((const char *)k)

#define CLOG_LOG_ARG_C(k, v) \
    k, clog_make_log_arg_c(v)

#define CLOG_LOG_ARG_CS(k) \
    #k, clog_make_log_arg_c(k)

#define CLOG_LOG_ARG_A(k, v, f) \
    k, clog_make_log_arg_a((void *)v, f)

#define CLOG_LOG_ARG_AS(k, f) \
    #k, clog_make_log_arg_a((void *)k, f)

// Logging macros
#define CLOG_LOG_DEBUG(msg, ...) clog_log_msg(debug, NULL, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_DEBUGF(msg, ...) clog_log_msgf(debug, NULL, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_INFO(msg, ...) clog_log_msg(info, NULL, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_INFOF(msg, ...) clog_log_msgf(info, NULL, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_WARNING(msg, ...) clog_log_msg(warning, NULL, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_WARNINGF(msg, ...) clog_log_msgf(warning, NULL, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_ERROR(msg, ...) clog_log_msg(error, NULL, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_ERRORF(msg, ...) clog_log_msgf(error, NULL, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_FATAL(msg, ...) clog_log_msg(fatal, NULL, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_FATALF(msg, ...) clog_log_msgf(fatal, NULL, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#ifdef CLOG_ENABLE_CTX
#define CLOG_LOG_DEBUG_CTX(ctx, msg, ...) clog_log_msg(debug, ctx, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_DEBUGF_CTX(ctx, msg, ...) clog_log_msgf(debug, ctx, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_INFO_CTX(ctx, msg, ...) clog_log_msg(info, ctx, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_INFOF_CTX(ctx, msg, ...) clog_log_msgf(info, ctx, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_WARNING_CTX(ctx, msg, ...) clog_log_msg(warning, ctx, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_WARNINGF_CTX(ctx, msg, ...) lclog_og_msgf(warning, ctx, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_ERROR_CTX(ctx, msg, ...) clog_log_msg(error, ctx, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_ERRORF_CTX(ctx, msg, ...) clog_log_msgf(error, ctx, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_FATAL_CTX(ctx, msg, ...) clog_log_msg(fatal, ctx, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#define CLOG_LOG_FATALF_CTX(ctx, msg, ...) clog_log_msgf(fatal, ctx, __FILE__, __LINE__, msg, ##__VA_ARGS__, CLOG_LOG_ARG_T)
#endif // CLOG_ENABLE_CTX

// Shorthand debug macros for quick debug value print
#define CLOG_DBGI(v) CLOG_LOG_DEBUG("dbg", "", CLOG_LOG_ARG_IS(v))
#define CLOG_DBGU(v) CLOG_LOG_DEBUG("dbg", "", CLOG_LOG_ARG_US(v))
#define CLOG_DBGF(v) CLOG_LOG_DEBUG("dbg", "", CLOG_LOG_ARG_FS(v))
#define CLOG_DBGS(v) CLOG_LOG_DEBUG("dbg", "", CLOG_LOG_ARG_SS(v))

// Log filter stuff
typedef struct {
    const char *start;
    size_t len;
} Clog_String_View;

typedef struct {
    const char *filter;
    Clog_String_View *items;
    size_t count;
    size_t capacity;
} Clog_Filters;

_Noreturn void panic(const char *msg);
_Noreturn void panicf(const char *msg, ...);
const char *clog_header(Clog_Log_Level level);
void clog_set_log_level(Clog_Log_Level level);
void clog_set_color_enabled(bool enabled);
void clog_set_timestamp_enabled(bool enabled);
const char *clog_current_time();
const char *clog_fmt_context(const char *ctx);
const char *clog_format_src(const char *file, int line, Clog_Log_Level level);
bool clog_do_log(const char *ctx, Clog_Log_Level level);
const char *clog_stringify_log_arg(Clog_Arg arg);
void clog_print_log(Clog_Log_Level level, const char *ctx, const char *file, int line, bool isf, const char *msg, va_list args);
void clog_log_msg(Clog_Log_Level level, const char *ctx, const char *file, int line, const char *msg, ...);
void clog_log_msgf(Clog_Log_Level level, const char *ctx, const char *file, int line, const char *format, ...);
#ifdef CLOG_ENABLE_CTX
void clog_print_view(Clog_String_View view, FILE *stream);
void clog_expect_rule_char(Clog_String_View view, const char c, const char *filter);
void clog_expect_advance(Clog_String_View *view, const char c, const char *filter);
void clog_parse_filter_elem(Clog_String_View view, const char *filter);
void clog_parse_log_filter(const char *filter);
void clog_parse_log_filter_env();
void clog_reset_filter();
#endif // CLOG_ENABLE_CTX

#ifdef CLOG_IMPLEMENTATION
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- START NOB CODE ---
// This code is taken from https://github.com/tsoding/nob.h/blob/112911eba033af91e3d6e7100f8dde69127b3525/nob.h
#define NOB_REALLOC realloc
#define NOB_FREE free

// Initial capacity of a dynamic array
#ifndef NOB_DA_INIT_CAP
#define NOB_DA_INIT_CAP 256
#endif

// Slightly modified to not having to bring in assert
#define nob_da_reserve(da, expected_capacity)                                              \
    do {                                                                                   \
        if ((expected_capacity) > (da)->capacity) {                                        \
            if ((da)->capacity == 0) {                                                     \
                (da)->capacity = NOB_DA_INIT_CAP;                                          \
            }                                                                              \
            while ((expected_capacity) > (da)->capacity) {                                 \
                (da)->capacity *= 2;                                                       \
            }                                                                              \
            (da)->items = NOB_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            CLOG_CHECK_ALLOC((da)->items)                                                  \
        }                                                                                  \
    } while (0)

// Append an item to a dynamic array
#define nob_da_append(da, item)                \
    do {                                       \
        nob_da_reserve((da), (da)->count + 1); \
        (da)->items[(da)->count++] = (item);   \
    } while (0)

#define nob_da_free(da) NOB_FREE((da).items)
// --- END NOB CODE ---

_Noreturn void panic(const char *msg) {
    if (msg == NULL || *msg == '\0') {
        fprintf(stderr, "Program panic.\n");
    } else {
        fprintf(stderr, "%s\n", msg);
    }
    exit(1);
}

_Noreturn void panicf(const char *msg, ...) {
    if (msg == NULL || *msg == '\0') {
        fprintf(stderr, "Program panic.\n");
    } else {
        va_list argptr;
        va_start(argptr, msg);
        fprintf(stderr, "Panic: ");
        vfprintf(stderr, msg, argptr);
        fputc('\0', stderr);
        va_end(argptr);
    }
    exit(1);
}

static inline Clog_Arg clog_make_log_arg_t() {
    Clog_Arg arg;
    arg.ty = TERM;
    arg.val.a = NULL;
    arg.conv_fun = NULL;
    return arg;
}

static inline Clog_Arg clog_make_log_arg_i(int v, Clog_Integer_Format fmt) {
    Clog_Arg arg;
    arg.ty = INT;
    arg.val.i = v;
    arg.conv_fun = NULL;
    arg.opt = fmt;
    return arg;
}

static inline Clog_Arg clog_make_log_arg_u(unsigned int v, Clog_Integer_Format fmt) {
    Clog_Arg arg;
    arg.ty = UINT;
    arg.val.u = v;
    arg.conv_fun = NULL;
    arg.opt = fmt;
    return arg;
}

static inline Clog_Arg clog_make_log_arg_f(float v) {
    Clog_Arg arg;
    arg.ty = FLOAT;
    arg.val.f = v;
    arg.conv_fun = NULL;
    return arg;
}

static inline Clog_Arg clog_make_log_arg_s(const char *v) {
    Clog_Arg arg;
    arg.ty = STRING;
    arg.val.s = v;
    arg.conv_fun = NULL;
    return arg;
}

static inline Clog_Arg clog_make_log_arg_c(char v) {
    Clog_Arg arg;
    arg.ty = CHAR;
    arg.val.i = v;
    arg.conv_fun = NULL;
    return arg;
}

static inline Clog_Arg clog_make_log_arg_a(void *v, char *(*fn)(void *)) {
    Clog_Arg arg;
    arg.ty = ANY;
    arg.val.a = v;
    arg.conv_fun = fn;
    return arg;
}

static Clog_Log_Level lvl = info;
static bool use_color = false;
static Clog_Filters disabled_logs = {0};
static Clog_Filters enabled_logs = {0};
static bool print_time = true;

const char *clog_header(Clog_Log_Level level) {
    switch (level) {
    case debug:
        return CLOG_DBG_HEADER(use_color);
    case info:
        return CLOG_INF_HEADER(use_color);
    case warning:
        return CLOG_WRN_HEADER(use_color);
    case error:
        return CLOG_ERR_HEADER(use_color);
    case fatal:
        return CLOG_FAT_HEADER(use_color);
    case nolog:
        return "";
    default:
        panicf("Invalid level: %d\n", level);
    }
}

void clog_set_log_level(Clog_Log_Level level) {
    lvl = level;
}

void clog_set_color_enabled(bool enabled) {
    use_color = enabled;
}

void clog_set_timestamp_enabled(bool enabled) {
    print_time = enabled;
}

const char *clog_current_time() {
    if (!print_time) {
        return "";
    }
    // 8  = Ansi escape sequence for gray
    // 7  = Ansi escape sequence for clear color
    // 21 = Time format string
    // 1  = Whitespace
    size_t buflen = use_color ? 8 + 21 + 7 : 21 + 1;
    char *buf = (char *)CLOG_MALLOC(buflen);
    CLOG_CHECK_ALLOC(buf);

    time_t now = time(NULL);
    struct tm *gmt = gmtime(&now);
    strftime(buf, buflen, use_color ? "\033[35m%Y-%m-%dT%H:%M:%SZ\033[0m " : "%Y-%m-%dT%H:%M:%SZ ", gmt);

    return buf;
}

const char *clog_fmt_context(const char *ctx) {
    if (ctx == NULL || strlen(ctx) == 0) {
        return "";
    }
    size_t buflen = strlen(ctx) + 4 + 1;
    if (use_color) {
        buflen += 8 + 7;
    }
    char *res_buf = (char *)CLOG_MALLOC(buflen + 1);
    CLOG_CHECK_ALLOC(res_buf);
    sprintf(res_buf, use_color ? "\033[35m{ %s }\033[0m " : "{ %s } ", ctx);
    return res_buf;
}

const char *clog_format_src(const char *file, int line, Clog_Log_Level level) {
    if (level > debug) {
        return "";
    }

    size_t fn_len = strlen(file);
    // Max long is 18446744073709551615 (20 chars) + null terminator
    char nbuf[21];
    sprintf(nbuf, "%d", line);
    size_t ln_len = strlen(nbuf);
    // 2 = <>
    // 1 = :
    // 1 = ' '
    size_t res_len = fn_len + ln_len + 2 + 1 + 1;
    if (use_color) {
        // Add lenghts of escape s
        res_len += 8 + 7;
    }
    char *res_buf = (char *)CLOG_MALLOC(res_len);
    sprintf(res_buf, use_color ? "\033[35m<%s:%s>\033[0m " : "<%s:%s> ", file, nbuf);
    return res_buf;
}

bool clog_do_log(const char *ctx, Clog_Log_Level level) {
    if (level == fatal) {
        return true;
    }
    if (level == nolog) {
        return false;
    }
    if (level < lvl) {
        return false;
    }
    if (ctx == NULL || strlen(ctx) == 0) {
        return level == nolog || level >= lvl;
    }
    for (size_t i = 0; i < disabled_logs.count; ++i) {
        Clog_String_View dis = disabled_logs.items[i];
        if (memcmp(dis.start, ctx, dis.len) == 0) {
            return false;
        }
        if (memcmp(dis.start, "all", dis.len) == 0) {
            for (size_t j = 0; j < enabled_logs.count; ++j) {
                Clog_String_View en = enabled_logs.items[j];
                if (memcmp(en.start, ctx, en.len) == 0 || memcmp(en.start, "all", en.len) == 0) {
                    return level == nolog || level >= lvl;
                }
            }
            return false;
        }
    }
    return level == nolog || level >= lvl;
}

const char *clog_stringify_log_arg(Clog_Arg arg) {
    switch (arg.ty) {
    case INT: {
        switch (arg.opt) {
        case BIN: {
            char *ib_buf = (char *)CLOG_MALLOC(64);
            CLOG_CHECK_ALLOC(ib_buf);
            sprintf(ib_buf, "%#lb", arg.val.i);
            return ib_buf;
        } break;
        case HEX: {
            char *ih_buf = (char *)CLOG_MALLOC(16);
            CLOG_CHECK_ALLOC(ih_buf);
            sprintf(ih_buf, "%#lx", arg.val.i);
            return ih_buf;
        } break;
        default: {
            char *i_buf = (char *)CLOG_MALLOC(21);
            CLOG_CHECK_ALLOC(i_buf);
            sprintf(i_buf, "%ld", arg.val.i);
            return i_buf;
        } break;
        }
    } break;
    case UINT: {
        switch (arg.opt) {
        case BIN: {
            char *ib_buf = (char *)CLOG_MALLOC(64);
            CLOG_CHECK_ALLOC(ib_buf);
            sprintf(ib_buf, "%#lb", arg.val.u);
            return ib_buf;
        } break;
        case HEX: {
            char *ih_buf = (char *)CLOG_MALLOC(16);
            CLOG_CHECK_ALLOC(ih_buf);
            sprintf(ih_buf, "%#lx", arg.val.u);
            return ih_buf;
        } break;
        default: {
            char *i_buf = (char *)CLOG_MALLOC(21);
            CLOG_CHECK_ALLOC(i_buf);
            sprintf(i_buf, "%lu", arg.val.u);
            return i_buf;
        } break;
        }
    } break;
    case FLOAT: {
        char *f_buf = (char *)CLOG_MALLOC(32);
        CLOG_CHECK_ALLOC(f_buf);
        sprintf(f_buf, "%.3f", arg.val.f);
        return f_buf;
    } break;
    case STRING: {
        const char *str_val = arg.val.s;
        if (str_val == NULL) {
            panicf("Log arg has type string but value is null");
        }
        char *s_buf = (char *)CLOG_MALLOC(strlen(str_val) + 3);
        CLOG_CHECK_ALLOC(s_buf);
        sprintf(s_buf, "\"%s\"", str_val);
        return s_buf;
    } break;
    case CHAR: {
        char *c_buf = (char *)CLOG_MALLOC(4);
        CLOG_CHECK_ALLOC(c_buf);
        sprintf(c_buf, "'%c'", (int)arg.val.i);
        return c_buf;
    } break;
    case ANY: {
        if (arg.val.a == NULL || arg.conv_fun == NULL) {
            panicf("Log arg has type any but value or conv function is null");
        }
        return arg.conv_fun(arg.val.a);
    } break;
    default: {
        panicf("Unrecognized arg type: %d", arg.ty);
    } break;
    }
}

void clog_print_log(Clog_Log_Level level, const char *ctx, const char *file, int line, bool isf, const char *msg, va_list args) {
    if (!clog_do_log(ctx, level)) {
        return;
    }

    const char *time = clog_current_time();
    const char *src = clog_format_src(file, line, level);
#ifdef CLOG_ENABLE_CTX
    const char *ctxs = clog_fmt_context(ctx);
#else
    const char *ctxs = "";
#endif // CLOG_ENABLE_CTX

    if (msg != NULL) {
        if (isf) {
            fprintf(stderr, "%s %s%s%s", clog_header(level), ctxs, time, src);
            vfprintf(stderr, msg, args);
        } else {
            fprintf(stderr, "%s %s%s%s%s", clog_header(level), ctxs, time, src, msg);
        }

        char *key;
        while ((key = va_arg(args, char *)) != NULL) {
            Clog_Arg val = va_arg(args, Clog_Arg);
            if (val.ty == 0) {
                panicf("Malformed log message! Missing value to key");
            }
            const char *val_str = clog_stringify_log_arg(val);
            fprintf(stderr, " ");
            fprintf(stderr, use_color ? "\033[35m%s=%s\033[0m" : "%s=%s", key, val_str);
            free((void *)val_str);
        }
        fprintf(stderr, "\n");
    }

    if (print_time) {
        free((void *)time);
    }
    if (level == debug) {
        free((void *)src);
    }
#ifdef CLOG_ENABLE_CTX
    if (ctx != NULL && strlen(ctx) > 0) {
        free((void *)ctxs);
    }
#endif // CLOG_ENABLE_CTX

    if (level == fatal) {
        exit(1);
    }
}

void clog_log_msg(Clog_Log_Level level, const char *ctx, const char *file, int line, const char *msg, ...) {
    va_list argptr;
    va_start(argptr, msg);
    clog_print_log(level, ctx, file, line, false, msg, argptr);
    va_end(argptr);
}

void clog_log_msgf(Clog_Log_Level level, const char *ctx, const char *file, int line, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    clog_print_log(level, ctx, file, line, true, format, argptr);
    va_end(argptr);
}

#ifdef CLOG_ENABLE_CTX
void clog_print_view(Clog_String_View view, FILE *stream) {
    for (size_t i = 0; i < view.len; i++) {
        fputc(view.start[i], stream);
    }
    fputc('\n', stream);
}

void clog_expect_rule_char(Clog_String_View view, const char c, const char *filter) {
    if (view.start == NULL) {
        panicf("NULL start in view");
    }
    if (*(view.start) != c) {
        panicf("Filter rule must be [log|nolog]:<context>, found illegal char '%c' at pos %zu\n", *(view.start), (size_t)(view.start - filter));
    }
}

void clog_expect_advance(Clog_String_View *view, const char c, const char *filter) {
    clog_expect_rule_char(*view, c, filter);
    view->start++;
    view->len--;
}

void clog_parse_filter_elem(Clog_String_View view, const char *filter) {
    if (view.len < 5) {
        panicf("Filter rule must be at least 5 characters long, got %d\n", view.len);
    }
    if (view.start[0] == 'n') {
        view.start++;
        view.len--;
        clog_expect_advance(&view, 'o', filter);
        clog_expect_advance(&view, 'l', filter);
        clog_expect_advance(&view, 'o', filter);
        clog_expect_advance(&view, 'g', filter);
        clog_expect_advance(&view, ':', filter);
        nob_da_append(&disabled_logs, view);
    } else if (view.start[0] == 'l') {
        view.start++;
        view.len--;
        clog_expect_advance(&view, 'o', filter);
        clog_expect_advance(&view, 'g', filter);
        clog_expect_advance(&view, ':', filter);
        nob_da_append(&enabled_logs, view);
    } else {
        panicf("Filter rule must be [log|nolog]:<context>, found illegal char '%c' at pos %zu\n", view.start[0], (size_t)(view.start - filter));
    }
}

void clog_parse_log_filter(const char *filter) {
    if (filter == NULL || strlen(filter) == 0) {
        return;
    }
    const char *start = filter;
    size_t filter_len = strlen(filter);
    Clog_String_View view = {0};
    size_t cursor = 0;
    for (; cursor < filter_len; cursor++) {
        if (filter[cursor] == ';') {
            view.start = start;
            view.len = (const char *)((size_t)filter + cursor) - start;
            clog_parse_filter_elem(view, filter);
            start = filter + cursor + 1;
        }
    }
    if (start < (filter + cursor)) {
        view.start = start;
        view.len = (const char *)((size_t)filter + cursor) - start;
        clog_parse_filter_elem(view, filter);
    }
    if (clog_do_log("filter", debug)) {
        for (size_t i = 0; i < enabled_logs.count; i++) {
            Clog_String_View en = enabled_logs.items[i];
            char *ctx = CLOG_MALLOC(sizeof(char) * (en.len + 1));
            CLOG_CHECK_ALLOC(ctx);
            memcpy(ctx, en.start, en.len);
            ctx[en.len] = '\0';
            CLOG_LOG_DEBUG_CTX("filter", "Enabled logging context", CLOG_LOG_ARG_SS(ctx));
            free(ctx);
        }
        for (size_t i = 0; i < disabled_logs.count; i++) {
            Clog_String_View di = disabled_logs.items[i];
            char *ctx = CLOG_MALLOC(sizeof(char) * (di.len + 1));
            CLOG_CHECK_ALLOC(ctx);
            memcpy(ctx, di.start, di.len);
            ctx[di.len] = '\0';
            CLOG_LOG_DEBUG_CTX("filter", "Disabled logging context", CLOG_LOG_ARG_SS(ctx));
            free(ctx);
        }
    }
}

void clog_parse_log_filter_env() {
    const char *env = (const char *)getenv("CLOG_FILTER");
    clog_parse_log_filter(env);
}

void clog_reset_filter() {
    enabled_logs.count = 0;
    disabled_logs.count = 0;
}
#endif // CLOG_ENABLE_CTX
#endif // CLOG_IMPLEMENTATION

#ifndef _CLOG_STRIP_GUARD
#define _CLOG_STRIP_GUARD
#ifdef CLOG_STRIP_PREFIX
#define RED CLOG_RED
#define GREEN CLOG_GREEN
#define BLUE CLOG_BLUE
#define YELLOW CLOG_YELLOW
#define MAGENTA CLOG_MAGENTA

#define set_log_level clog_set_log_level
#define set_color_enabled clog_set_color_enabled
#ifndef _Win32
#define set_color_auto clog_set_color_auto
#endif // !_Win32
#define set_timestamp_enabled clog_set_timestamp_enabled

#define LOG_ARG_T CLOG_LOG_ARG_T
#define LOG_ARG_I CLOG_LOG_ARG_I
#define LOG_ARG_IS CLOG_LOG_ARG_IS
#define LOG_ARG_IB CLOG_LOG_ARG_IB
#define LOG_ARG_ISB CLOG_LOG_ARG_ISB
#define LOG_ARG_IH CLOG_LOG_ARG_IH
#define LOG_ARG_ISH CLOG_LOG_ARG_ISH
#define LOG_ARG_U CLOG_LOG_ARG_U
#define LOG_ARG_US CLOG_LOG_ARG_US
#define LOG_ARG_UB CLOG_LOG_ARG_UB
#define LOG_ARG_USB CLOG_LOG_ARG_USB
#define LOG_ARG_UH CLOG_LOG_ARG_UH
#define LOG_ARG_USH CLOG_LOG_ARG_USH
#define LOG_ARG_F CLOG_LOG_ARG_F
#define LOG_ARG_FS CLOG_LOG_ARG_FS
#define LOG_ARG_S CLOG_LOG_ARG_S
#define LOG_ARG_SS CLOG_LOG_ARG_SS
#define LOG_ARG_C CLOG_LOG_ARG_C
#define LOG_ARG_CS CLOG_LOG_ARG_CS
#define LOG_ARG_A CLOG_LOG_ARG_A
#define LOG_ARG_AS CLOG_LOG_ARG_AS

// Logging macros
#define LOG_DEBUG CLOG_LOG_DEBUG
#define LOG_DEBUGF CLOG_LOG_DEBUGF
#define LOG_INFO CLOG_LOG_INFO
#define LOG_INFOF CLOG_LOG_INFOF
#define LOG_WARNING CLOG_LOG_WARNING
#define LOG_WARNINGF CLOG_LOG_WARNINGF
#define LOG_ERROR CLOG_LOG_ERROR
#define LOG_ERRORF CLOG_LOG_ERRORF
#define LOG_FATAL CLOG_LOG_FATAL
#define LOG_FATALF CLOG_LOG_FATALF
#ifdef CLOG_ENABLE_CTX
#define LOG_DEBUG_CTX CLOG_LOG_DEBUG_CTX
#define LOG_DEBUGF_CTX CLOG_LOG_DEBUGF_CTX
#define LOG_INFO_CTX CLOG_LOG_INFO_CTX
#define LOG_INFOF_CTX CLOG_LOG_INFOF_CTX
#define LOG_WARNING_CTX CLOG_LOG_WARNING_CTX
#define LOG_WARNINGF_CTX CLOG_LOG_WARNINGF_CTX
#define LOG_ERROR_CTX CLOG_LOG_ERROR_CTX
#define LOG_ERRORF_CTX CLOG_LOG_ERRORF_CTX
#define LOG_FATAL_CTX CLOG_LOG_FATAL_CTX
#define LOG_FATALF_CTX CLOG_LOG_FATALF_CTX
#endif // CLOG_ENABLE_CTX

// Shorthand debug macros for quick debug value print
#define DBGI CLOG_DBGI
#define DBGU CLOG_DBGU
#define DBGF CLOG_DBGF
#define DBGS CLOG_DBGS

#define parse_log_filter clog_parse_log_filter
#define parse_log_filter_env clog_parse_log_filter_env
#define reset_filter clog_reset_filter
#endif // CLOG_STRIP_PREFIX
#endif // !_CLOG_STRIP_GUARD

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __CLOG_H__

/*
MIT License

Copyright (c) 2025 Aurelio Buonomo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
