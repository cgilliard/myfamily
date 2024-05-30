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

#include <execinfo.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <log/log.h>

int logger(Log *log, int num, ...) {
	// set defaults
	log->fp = NULL;
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

	// iterate through arg list for overrides
	va_list valist;
	va_start(valist, num);
	for(int i = 0; i < num; i++) {
      		LogConfigOption next = va_arg(valist, LogConfigOption);
		if(next.type == ShowColors) {
			log->show_colors = *((bool*)next.value);
		} else if(next.type == ShowStdout) {
			log->show_stdout = *((bool*)next.value);
		} else if(next.type == ShowTimestamp) {
			log->show_timestamp = *((bool*)next.value);
		} else if(next.type == ShowMillis) {
                        log->show_millis = *((bool*)next.value);
                } else if(next.type == ShowLogLevel) {
                        log->show_log_level = *((bool*)next.value);
                } else if(next.type == AutoRotate) {
                        log->auto_rotate = *((bool*)next.value);
                } else if(next.type == DeleteRotation) {
                        log->delete_rotation = *((bool*)next.value);
                } else if(next.type == MaxSizeBytes) {
			log->max_size_bytes = *((u64*)next.value);
		} else if(next.type == MaxAgeMillis) {
                        log->max_age_millis = *((u64*)next.value);
                } else if(next.type == LogFilePath) {
			char *buf = ((char*)next.value);
			int len = strlen(buf);
			log->path = malloc(sizeof(char) * (len + 1));
			if(log->path == NULL) {
				if(log->file_header != NULL) {
					free(log->file_header);
				}
                		return -1;
        		}
			strcpy(log->path, buf);
		} else if(next.type == FileHeader) {
			char *buf = ((char*)next.value);
                        int len = strlen(buf);
                        log->file_header = malloc(sizeof(char) * (len + 1));
			if(log->file_header == NULL) {
                                if(log->path != NULL) {
                                        free(log->path);
                                }
                                return -1;
                        }
                        strcpy(log->file_header, buf);
		}

   	}
	va_end(valist);

	return 0;
}

int get_format(Log *log, LogLevel level, char *buf) {
	char milli_buf[10];
	char log_level_buf[10];
	char dt_buf[30];
	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);

	if(log->show_millis) {
		struct timeval time;
  		gettimeofday(&time, NULL);
		int millis = time.tv_usec / 1000;
		sprintf(milli_buf, ".%03d", millis);
	} else {
		strcpy(milli_buf, "");
	}

	if(log->show_log_level) {
		if(level == Trace) {
			strcpy(log_level_buf, " (TRACE)");
		} else if(level == Debug) {
			strcpy(log_level_buf, " (DEBUG)");
		} else if(level == Info) {
                        strcpy(log_level_buf, " (INFO)");
                } else if(level == Warn) {
                        strcpy(log_level_buf, " (WARN)");
                } else if(level == Error) {
                        strcpy(log_level_buf, " (ERROR)");
                } else if(level == Fatal) {
                        strcpy(log_level_buf, " (FATAL)");
                } else {
			strcpy(log_level_buf, "");
		}

	} else {
		strcpy(log_level_buf, "");
	}

	if(log->show_timestamp) {
		sprintf(
			dt_buf,
			"[%d-%02d-%02d %02d:%02d:%02d%s]",
			tm.tm_year + 1900,
                	tm.tm_mon + 1,
                	tm.tm_mday,
                	tm.tm_hour,
                	tm.tm_min,
                	tm.tm_sec,
                	milli_buf
		);
	} else {
		strcpy(dt_buf, "");
	}

	sprintf(
		buf,
		"%s%s: ",
		dt_buf,
		log_level_buf
	);
	return 0;
}

int log_line(Log *log, LogLevel level, char *line) {
	int len = strlen(line);
	char fline[len+100];

	get_format(log, level, fline);
	strcat(fline, line);
	strcat(fline, "\n");

	if(log->show_stdout) {
		printf("%s", fline);
	}
	if(log->fp) {
		fprintf(log->fp, "%s", fline);
	}
	return 0;
}

int init(Log *log) {
	if(log->path) {
		bool write_header = false;
		if(log->file_header && access(log->path, F_OK) != 0) {
			write_header = true;
		}
		log->fp = fopen(log->path, "a");
		if(write_header) {
			fprintf(log->fp,"%s\n",log->file_header);
		}
		fseek(log->fp, 0, SEEK_END);
	}
	return 0;
}

int log_close(Log *log) {
	if(log->fp) {
		fclose(log->fp);
	}
	return 0;
}

int log_config_option_show_colors(LogConfigOption *option, bool value) {
	option->type = ShowColors;
	option->value = malloc(sizeof(bool));
	if(option->value == NULL) {
		return -1;
	}
	*((bool *)option->value) = value;
	return 0;
}

int log_config_option_show_stdout(LogConfigOption *option, bool value) {
	option->type = ShowStdout;
	option->value = malloc(sizeof(bool));
	if(option->value == NULL) {
                return -1;
        }
	*((bool *)option->value) = value;
	return 0;
}

int log_config_option_show_timestamp(LogConfigOption *option, bool value) {
        option->type = ShowTimestamp;
        option->value = malloc(sizeof(bool));
        if(option->value == NULL) {
                return -1;
        }
        *((bool *)option->value) = value;
        return 0;
}

int log_config_option_show_millis(LogConfigOption *option, bool value) {
        option->type = ShowMillis;
        option->value = malloc(sizeof(bool));
        if(option->value == NULL) {
                return -1;
        }
        *((bool *)option->value) = value;
        return 0;
}

int log_config_option_show_log_level(LogConfigOption *option, bool value) {
        option->type = ShowLogLevel;
        option->value = malloc(sizeof(bool));
        if(option->value == NULL) {
                return -1;
        }
        *((bool *)option->value) = value;
        return 0;
}

int log_config_option_auto_rotate(LogConfigOption *option, bool value) {
        option->type = AutoRotate;
        option->value = malloc(sizeof(bool));
        if(option->value == NULL) {
                return -1;
        }
        *((bool *)option->value) = value;
        return 0;
}

int log_config_option_show_delete_rotation(LogConfigOption *option, bool value) {
        option->type = DeleteRotation;
        option->value = malloc(sizeof(bool));
        if(option->value == NULL) {
                return -1;
        }
        *((bool *)option->value) = value;
        return 0;
}

int log_config_option_max_size_bytes(LogConfigOption *option, u64 value) {
        option->type = MaxSizeBytes;
        option->value = malloc(sizeof(u64));
        if(option->value == NULL) {
                return -1;
        }
        *((u64 *)option->value) = value;
        return 0;
}

int log_config_option_max_age_millis(LogConfigOption *option, u64 value) {
        option->type = MaxAgeMillis;
        option->value = malloc(sizeof(u64));
        if(option->value == NULL) {
                return -1;
        }
        *((u64 *)option->value) = value;
        return 0;
}

int log_config_option_log_file_path(LogConfigOption *option, char *value) {
	option->type = LogFilePath;
	if(value == NULL) {
		option->value = NULL;
	} else {
		int len = strlen(value);
		option->value = malloc(sizeof(char) * (len + 1));
		if(option->value == NULL) {
                	return -1;
        	}
		strcpy(option->value, value);
	}
	return 0;
}

int log_config_option_file_header(LogConfigOption *option, char *value) {
        option->type = FileHeader;
        if(value == NULL) {
                option->value = NULL;
        } else {
                int len = strlen(value);
                option->value = malloc(sizeof(char) * (len + 1));
		if(option->value == NULL) {
                	return -1;
        	}
                strcpy(option->value, value);
        }
        return 0;
}

void free_log_config_option(LogConfigOption *option) {
	if(option->value != NULL)
		free(option->value);
}

void free_log(Log *log) {
	if(log->path)
		free(log->path);
	if(log->file_header)
		free(log->file_header);
}
