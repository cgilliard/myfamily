// Copyright (c) 2024, The MyFamily Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <base/rand.h>
#include <execinfo.h>
#include <inttypes.h>
#include <limits.h>
#include <log/log.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

const char PathSeparator =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif

#define _FILE_OFFSET_BITS 64

int set_option(Log* log, int type, void* value)
{
    if (type == ShowColors) {
        log->show_colors = *((bool*)value);
    } else if (type == ShowStdout) {
        log->show_stdout = *((bool*)value);
    } else if (type == ShowTimestamp) {
        log->show_timestamp = *((bool*)value);
    } else if (type == ShowMillis) {
        log->show_millis = *((bool*)value);
    } else if (type == ShowLogLevel) {
        log->show_log_level = *((bool*)value);
    } else if (type == AutoRotate) {
        log->auto_rotate = *((bool*)value);
    } else if (type == DeleteRotation) {
        log->delete_rotation = *((bool*)value);
    } else if (type == MaxSizeBytes) {
        log->max_size_bytes = *((u64*)value);
    } else if (type == MaxAgeMillis) {
        log->max_age_millis = *((u64*)value);
    } else if (type == LogFilePath) {
        char* buf = ((char*)value);
        int len = strlen(buf);
        log->path = malloc(sizeof(char) * (len + 1));
        if (log->path == NULL) {
            return -1;
        }
        strcpy(log->path, buf);
    } else if (type == FileHeader) {
        char* buf = ((char*)value);
        int len = strlen(buf);
        char* tmp = malloc(sizeof(char) * (len + 1));
        if (tmp == NULL) {
            return -1;
        }
        strcpy(tmp, buf);
        if (log->file_header != NULL)
            free(log->file_header);
        log->file_header = tmp;
    }

    return 0;
}

int logger(Log* log, int num, ...)
{
    int ret = 0;
    // set defaults
    log->fp = NULL;
    log->level = Info;
    log->show_colors = true;
    log->show_stdout = true;
    log->show_timestamp = true;
    log->show_millis = true;
    log->show_log_level = true;
    log->auto_rotate = false;
    log->delete_rotation = false;
    log->max_size_bytes = ULONG_MAX;
    log->max_age_millis = ULONG_MAX;
    log->path = NULL;
    log->file_header = NULL;
    log->off = 0;
    log->last_rotation = clock();

    // iterate through arg list for overrides
    va_list valist;
    va_start(valist, num);
    for (int i = 0; i < num; i++) {
        LogConfigOption next = va_arg(valist, LogConfigOption);
        if (set_option(log, next.type, next.value)) {
            if (log->file_header != NULL)
                free(log->file_header);
            if (log->path != NULL)
                free(log->path);
            ret = -1;
            break;
        }
    }

    va_end(valist);
    return ret;
}

int get_format(Log* log, LogLevel level, char* buf)
{
    char milli_buf[14];
    char log_level_buf[20];
    char dt_buf[40];
    char spacing[2];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if (log->show_millis) {
        struct timeval time;
        gettimeofday(&time, NULL);
        int millis = time.tv_usec / 1000;
        sprintf(milli_buf, ".%03d", millis);
    } else {
        strcpy(milli_buf, "");
    }

    if (log->show_timestamp) {
        strcpy(spacing, " ");
    } else {
        strcpy(spacing, "");
    }

    if (log->show_log_level) {
        if (level == Trace) {
            if (log->show_colors) {
                sprintf(log_level_buf, "%s(" ANSI_COLOR_YELLOW "TRACE" ANSI_COLOR_RESET ")", spacing);
            } else {
                sprintf(log_level_buf, "%s(TRACE)", spacing);
            }
        } else if (level == Debug) {
            if (log->show_colors) {
                sprintf(log_level_buf, "%s(" ANSI_COLOR_CYAN "DEBUG" ANSI_COLOR_RESET ")", spacing);
            } else {
                sprintf(log_level_buf, "%s(DEBUG)", spacing);
            }
        } else if (level == Info) {
            if (log->show_colors) {
                sprintf(log_level_buf, "%s(" ANSI_COLOR_GREEN "INFO" ANSI_COLOR_RESET ")", spacing);
            } else {
                sprintf(log_level_buf, "%s(INFO)", spacing);
            }
        } else if (level == Warn) {
            if (log->show_colors) {
                sprintf(log_level_buf, "%s(" ANSI_COLOR_MAGENTA "WARN" ANSI_COLOR_RESET ")", spacing);
            } else {
                sprintf(log_level_buf, "%s(WARN)", spacing);
            }
        } else if (level == Error) {
            if (log->show_colors) {
                sprintf(log_level_buf, "%s(" ANSI_COLOR_RED "ERROR" ANSI_COLOR_RESET ")", spacing);
            } else {
                sprintf(log_level_buf, "%s(ERROR)", spacing);
            }
        } else if (level == Fatal) {
            if (log->show_colors) {
                sprintf(log_level_buf, "%s(" ANSI_COLOR_BRIGHT_RED "FATAL" ANSI_COLOR_RESET ")", spacing);
            } else {
                sprintf(log_level_buf, "%s(FATAL)", spacing);
            }
        } else {
            strcpy(log_level_buf, "");
        }
    } else {
        strcpy(log_level_buf, "");
    }

    if (log->show_timestamp) {
        if (log->show_colors) {
            sprintf(
                dt_buf,
                "[" ANSI_COLOR_DIMMED
                "%d-%02d-%02d %02d:%02d:%02d%s" ANSI_COLOR_RESET
                "]",
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec,
                milli_buf);
        } else {
            sprintf(
                dt_buf,
                "["
                "%d-%02d-%02d %02d:%02d:%02d%s"
                "]",
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec,
                milli_buf);
        }
    } else {
        strcpy(dt_buf, "");
    }

    if ((level == Info || level == Warn) && log->show_log_level) {
        strcpy(spacing, ":");
    } else {
        strcpy(spacing, "");
    }

    sprintf(
        buf,
        "%s%s%s: ",
        dt_buf,
        log_level_buf,
        spacing);
    return 0;
}

int do_log(Log* log, LogLevel level, char* line, bool is_plain, bool is_all, va_list args)
{
    if (level >= log->level) {
        va_list args_copy;
        va_copy(args_copy, args);

        int len = strlen(line);
        char fline[len + 100];

        if (is_plain) {
            strcpy(fline, "");
        } else {
            get_format(log, level, fline);
        }
        strcat(fline, line);
        strcat(fline, "\n");

        if (log->show_stdout || is_all) {
            vprintf(fline, args);
        }
        if (log->fp) {
            vfprintf(log->fp, fline, args_copy);
            log->off = ftello(log->fp);
        }
    }
    return 0;
}

int log_all(Log* log, LogLevel level, char* line, ...)
{
    va_list args;
    va_start(args, line);
    int ret = do_log(log, level, line, false, true, args);
    va_end(args);
    return ret;
}

int log_plain(Log* log, LogLevel level, char* line, ...)
{
    va_list args;
    va_start(args, line);
    int ret = do_log(log, level, line, true, false, args);
    va_end(args);
    return ret;
}

int log_line(Log* log, LogLevel level, char* line, ...)
{
    va_list args;
    va_start(args, line);
    int ret = do_log(log, level, line, false, false, args);
    va_end(args);
    return ret;
}

void log_set_level(Log* log, LogLevel level)
{
    log->level = level;
}

int log_init(Log* log)
{
    if (log->path) {
        bool write_header = false;
        if (log->file_header && access(log->path, F_OK) != 0) {
            write_header = true;
        }
        log->fp = fopen(log->path, "a");
        if (write_header) {
            fprintf(log->fp, "%s\n", log->file_header);
        }
        fseek(log->fp, 0, SEEK_END);
        log->off = ftello(log->fp);
    }
    return 0;
}

int log_config_option(Log* log, LogConfigOption option)
{
    if (option.type == LogFilePath) {
        return -1;
    }

    return set_option(log, option.type, option.value);
}

int log_rotate(Log* log)
{
    int ret = 0;
    char rotation_name[strlen(log->path) + 100];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    printf("do rotate\n");

    char* fname = strrchr(log->path, PathSeparator);
    if (fname == NULL) {
        fname = log->path;
    } else {
        if (strlen(fname) > 0)
            fname = fname + 1;
        else
            ret = -1;
        printf("fname=%s,orig=%s\n", fname + 1, log->path);
        // test.log -> test.r_<mon>_<day>_<year>_<time>_<rand>.log
        // sprintf(rotation_name, "%d-%02d-%02d %02d:%02d:%02d%s");
    }

    char* ext;
    if (ret == 0) {
        strncpy(rotation_name, log->path, fname - log->path);
        rotation_name[fname - log->path] = 0;

        ext = strrchr(fname, '.');
        if (ext == NULL) {
            ext = fname + strlen(fname);
        }
    }

    if (ret == 0) {
        char date_format[100];
        strncpy(rotation_name + (fname - log->path), fname, ext - fname);
        rotation_name[ext - log->path] = 0;
        strcat(rotation_name, ".r_");
        u64 r;
        ret = rand_u64(&r);
        if (ret == 0) {
            sprintf(date_format,
                "%d_%02d_%02d_%02d_%02d_%02d_%llu",
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec,
                r);
            strcat(rotation_name, date_format);
            strcat(rotation_name, ext);
            printf("fname=%s,ext=%s,rotation_name=%s\n", fname, ext, rotation_name);
        }
    }

    if (ret == 0) {
        log->off = 0;
        log->last_rotation = clock();
    }
    return ret;
}

u64 timediff(clock_t t1, clock_t t2)
{
    u64 elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}

bool log_need_rotate(Log* log)
{
    u64 diff = timediff(clock(), log->last_rotation);
    return log->off > log->max_size_bytes || diff > log->max_age_millis;
}

int log_close(Log* log)
{
    if (log->fp) {
        fclose(log->fp);
    }
    return 0;
}

int log_config_option_show_colors(LogConfigOption* option, bool value)
{
    option->type = ShowColors;
    option->value = malloc(sizeof(bool));
    if (option->value == NULL) {
        return -1;
    }
    *((bool*)option->value) = value;
    return 0;
}

int log_config_option_show_stdout(LogConfigOption* option, bool value)
{
    option->type = ShowStdout;
    option->value = malloc(sizeof(bool));
    if (option->value == NULL) {
        return -1;
    }
    *((bool*)option->value) = value;
    return 0;
}

int log_config_option_show_timestamp(LogConfigOption* option, bool value)
{
    option->type = ShowTimestamp;
    option->value = malloc(sizeof(bool));
    if (option->value == NULL) {
        return -1;
    }
    *((bool*)option->value) = value;
    return 0;
}

int log_config_option_show_millis(LogConfigOption* option, bool value)
{
    option->type = ShowMillis;
    option->value = malloc(sizeof(bool));
    if (option->value == NULL) {
        return -1;
    }
    *((bool*)option->value) = value;
    return 0;
}

int log_config_option_show_log_level(LogConfigOption* option, bool value)
{
    option->type = ShowLogLevel;
    option->value = malloc(sizeof(bool));
    if (option->value == NULL) {
        return -1;
    }
    *((bool*)option->value) = value;
    return 0;
}

int log_config_option_auto_rotate(LogConfigOption* option, bool value)
{
    option->type = AutoRotate;
    option->value = malloc(sizeof(bool));
    if (option->value == NULL) {
        return -1;
    }
    *((bool*)option->value) = value;
    return 0;
}

int log_config_option_show_delete_rotation(LogConfigOption* option, bool value)
{
    option->type = DeleteRotation;
    option->value = malloc(sizeof(bool));
    if (option->value == NULL) {
        return -1;
    }
    *((bool*)option->value) = value;
    return 0;
}

int log_config_option_max_size_bytes(LogConfigOption* option, u64 value)
{
    option->type = MaxSizeBytes;
    option->value = malloc(sizeof(u64));
    if (option->value == NULL) {
        return -1;
    }
    *((u64*)option->value) = value;
    return 0;
}

int log_config_option_max_age_millis(LogConfigOption* option, u64 value)
{
    option->type = MaxAgeMillis;
    option->value = malloc(sizeof(u64));
    if (option->value == NULL) {
        return -1;
    }
    *((u64*)option->value) = value;
    return 0;
}

int log_config_option_log_file_path(LogConfigOption* option, char* value)
{
    option->type = LogFilePath;
    if (value == NULL) {
        option->value = NULL;
    } else {
        int len = strlen(value);
        option->value = malloc(sizeof(char) * (len + 1));
        if (option->value == NULL) {
            return -1;
        }
        strcpy(option->value, value);
    }
    return 0;
}

int log_config_option_file_header(LogConfigOption* option, char* value)
{
    option->type = FileHeader;
    if (value == NULL) {
        option->value = NULL;
    } else {
        int len = strlen(value);
        option->value = malloc(sizeof(char) * (len + 1));
        if (option->value == NULL) {
            return -1;
        }
        strcpy(option->value, value);
    }
    return 0;
}

void log_config_option_free(LogConfigOption* option)
{
    if (option->value != NULL)
        free(option->value);
}

void log_free(Log* log)
{
    if (log->path)
        free(log->path);
    if (log->file_header)
        free(log->file_header);
}
