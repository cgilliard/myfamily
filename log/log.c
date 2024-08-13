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

#include <core/std.h>
#include <errno.h>
#include <log/log.h>
#include <sys/time.h>
#include <time.h>

const char PathSeparator =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif

GETTER(Log, formatter)
GETTER(Log, config)
GETTER(Log, fp)
SETTER(Log, fp)
GETTER(Log, last_rotation)
SETTER(Log, last_rotation)
GETTER(Log, cur_size)
SETTER(Log, cur_size)
SETTER(Log, lock)

u64 log_now() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	u64 ret = (u64)(tv.tv_sec) * 1000 + (u64)(tv.tv_usec) / 1000;
	return ret;
}

void Log_cleanup(Log *log) {
	FormatterPtr f = GET(Log, log, formatter);
	cleanup(&f);
	LogConfig config = GET(Log, log, config);
	cleanup(&config.log_file_path);
	cleanup(&config.file_header);
	FILE *fp = GET(Log, log, fp);
	if (fp != NULL)
		myfclose(fp);

	if (config.is_sync) {
		Slab lock_slab = GET(Log, log, lock);
		pthread_mutex_destroy(lock_slab.data);
		myfree(&lock_slab);
	}
}

bool Log_need_rotate(Log *log) {
	LogConfig config = GET(Log, log, config);
	u64 max_size_bytes = config.max_size_bytes;
	u64 max_age_millis = config.max_age_millis;

	u64 last_rot = GET(Log, log, last_rotation);
	u64 cur_sz = GET(Log, log, cur_size);
	u64 now = log_now();

	if (now - last_rot > max_age_millis)
		return true;
	if (cur_sz > max_size_bytes)
		return true;
	return false;
}

Result Log_log(Log *log, LogLevel level, String line) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char buf[100];
	char milli_buf[20];
	String full_line = STRING("");
	LogConfig conf = GET(Log, log, config);

	if (conf.show_millis) {
		struct timeval time;
		gettimeofday(&time, NULL);
		int millis = time.tv_usec / 1000;
		snprintf(milli_buf, 20, ".%03d", millis);
	} else
		strcpy(milli_buf, "");

	if (conf.show_timestamp) {
		if (conf.show_colors) {
			snprintf(
			    buf, 100,
			    "[%s%d-%02d-%02d %02d:%02d:%02d%s%s]: ", DIMMED,
			    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			    tm.tm_hour, tm.tm_min, tm.tm_sec, milli_buf, RESET);
		} else
			snprintf(buf, 100, "[%d-%02d-%02d %02d:%02d:%02d%s]: ",
				 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
				 tm.tm_hour, tm.tm_min, tm.tm_sec, milli_buf);
		String ts_string = STRING(buf);
		Result r1 = append(&full_line, unwrap(&ts_string));
		TRYU(r1);
	}
	if (conf.show_log_level) {
		String level_str;
		if (level == TRACE) {
			if (conf.show_colors)
				snprintf(buf, 100, "(%sTRACE%s): ", BLUE,
					 RESET);
			else
				snprintf(buf, 100, "(TRACE): ");
			level_str = STRING(buf);
		} else if (level == DEBUG) {
			if (conf.show_colors)
				snprintf(buf, 100, "(%sDEBUG%s): ", CYAN,
					 RESET);
			else
				snprintf(buf, 100, "(DEBUG): ");
			level_str = STRING(buf);
		} else if (level == INFO) {
			if (conf.show_colors)
				snprintf(buf, 100, "(%sINFO%s) : ", GREEN,
					 RESET);
			else
				snprintf(buf, 100, "(INFO) : ");
			level_str = STRING(buf);
		} else if (level == WARN) {
			if (conf.show_colors)
				snprintf(buf, 100, "(%sWARN%s) : ", YELLOW,
					 RESET);
			else
				snprintf(buf, 100, "(WARN) : ");
			level_str = STRING(buf);
		} else if (level == ERROR) {
			if (conf.show_colors)
				snprintf(buf, 100, "(%sERROR%s): ", MAGENTA,
					 RESET);
			else
				snprintf(buf, 100, "(ERROR): ");
			level_str = STRING(buf);
		} else if (level == FATAL) {
			if (conf.show_colors)
				snprintf(buf, 100, "(%sFATAL%s): ", BRIGHT_RED,
					 RESET);
			else
				snprintf(buf, 100, "(FATAL): ");
			level_str = STRING(buf);
		} else {
			Error err = ERR(ILLEGAL_STATE, "unexpected log level");
			return Err(err);
		}
		Result r1 = append(&full_line, unwrap(&level_str));
		TRYU(r1);
	}
	if (conf.show_linenum) {
		char bt_buf[2000];
		Backtrace bt = BUILD(Backtrace, NULL, 0);
		BACKTRACE(&bt);

		u64 count = GET(Backtrace, &bt, count);
		bool found_log_c = false;
		bool found_path = false;
		for (u64 i = 0; i < count; i++) {
			Backtrace_file_path(&bt, bt_buf, 2000, i);
			if (strstr(bt_buf, "log/log.c:"))
				found_log_c = true;
			else if (found_log_c) {
				found_path = true;
				break;
			}
		}

		if (!found_path)
			strcpy(bt_buf, "UNKNOWN");
		// take out space (linux)
		for (u64 i = 0; i < strlen(bt_buf); i++)
			if (bt_buf[i] == ' ')
				bt_buf[i] = 0;

		if (found_path) {
			int strlen_bt_buf = strlen(bt_buf);
			if (strlen_bt_buf > conf.line_num_max_len) {
				char nbuf[conf.line_num_max_len + 3];
				nbuf[0] = '.';
				nbuf[1] = '.';
				strcpy(nbuf + 2,
				       bt_buf + (strlen_bt_buf -
						 conf.line_num_max_len));
				nbuf[conf.line_num_max_len + 2] = 0;
				strcpy(bt_buf, nbuf);
			}
		}

		String line_num_str;
		if (conf.show_colors) {
			snprintf(buf, 100, "[%s%s%s]: ", YELLOW, bt_buf, RESET);
			line_num_str = STRING(buf);
		} else {
			snprintf(buf, 100, "[%s]: ", bt_buf);
			line_num_str = STRING(buf);
		}
		Result r1 = append(&full_line, unwrap(&line_num_str));
	}
	Result r2 = append(&full_line, unwrap(&line));
	TRYU(r2);
	char *raw_line = unwrap(&full_line);

	if (conf.show_terminal) {
		if (level >= WARN)
			fprintf(stderr, "%s\n", raw_line);
		else
			fprintf(stdout, "%s\n", raw_line);
	}

	FILE *fp = GET(Log, log, fp);
	if (fp) {
		u64 last_rot = GET(Log, log, last_rotation);
		u64 cur_sz = GET(Log, log, cur_size);
		cur_sz += strlen(raw_line) + 1;
		SET(Log, log, cur_size, cur_sz);
		fprintf(fp, "%s\n", raw_line);
		fflush(fp);
		if (conf.auto_rotate) {
			if (Log_need_rotate(log)) {
				Result r = Log_rotate(log);
				TRYU(r);
			}
		}
	}

	return Ok(UNIT);
}

Formatter Log_formatter(Log *log) { return GET(Log, log, formatter); }

Result Log_rotate(Log *log) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	LogConfig conf = GET(Log, log, config);
	char *path = NULL;
	StringPtr log_file_path;
	MATCH(conf.log_file_path, VARIANT(SOME, {
		      String lfp = BORROW(conf.log_file_path, lfp);
		      printf("lfp=%s\n", (char *)unwrap(&lfp));
		      Option opath;
		      myclone(&opath, &conf.log_file_path);
		      log_file_path = ENUM_VALUE(log_file_path, String, opath);
		      path = unwrap(&log_file_path);
	      }));

	if (path == NULL) {
		Error e =
		    ERR(ILLEGAL_STATE,
			"rotation called on a log that does not have a file");
		return Err(e);
	}

	char rotation_name[strlen(path) + 100];
	char *fname = strrchr(path, PathSeparator);
	char *ext;
	if (fname == NULL) {
		fname = path;
	} else {
		if (strlen(fname) > 0) {
			fname = fname + 1;
		} else {
			Error e = ERR(ILLEGAL_STATE, "invalid file path");
			return Err(e);
		}
	}
	strncpy(rotation_name, path, fname - path);
	rotation_name[fname - path] = 0;

	ext = strrchr(fname, '.');
	if (ext == NULL) {
		ext = fname + strlen(fname);
	}

	char date_format[100];
	strncpy(rotation_name + (fname - path), fname, ext - fname);
	rotation_name[ext - path] = 0;
	strcat(rotation_name, ".r_");
	u64 r;
	if (rand_u64(&r)) {
		Error e = ERR(RAND_ERROR, "could not generate a random");
		return Err(e);
	}
	snprintf(date_format, 100, "%d_%02d_%02d_%02d_%02d_%02d_%" PRIu64,
		 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
		 tm.tm_min, tm.tm_sec, r);
	strcat(rotation_name, date_format);
	strcat(rotation_name, ext);

	FILE *lfp = GET(Log, log, fp);
	myfclose(lfp);
	bool delete_rotation = conf.delete_rotation;
	if (delete_rotation) {
		remove(path);
	} else {
		rename(path, rotation_name);
	}

	lfp = myfopen(path, "w");
	if (lfp == NULL) {
		int err = errno;
		char *error_message = strerror(err);
		char buf[strlen(error_message) + strlen(path) + 5];
		strcpy(buf, path);
		strcat(buf, ": ");
		strcat(buf, error_message);
		Error e = ERR(IO_ERROR, buf);
		return Err(e);
	}
	MATCH(conf.file_header, VARIANT(SOME, {
		      StringPtr header =
			  ENUM_VALUE(header, String, conf.file_header);
		      char *hdr = unwrap(&header);
		      fprintf(lfp, "%s\n", hdr);
		      fflush(lfp);
	      }));
	fseek(lfp, 0, SEEK_END);
	SET(Log, log, cur_size, ftello(lfp));
	SET(Log, log, fp, lfp);
	SET(Log, log, last_rotation, log_now());

	return Ok(UNIT);
}

Result Log_update_init_state(FILE *fp, LogConfig *lc) {
	u64 sz = 0;
	MATCH(lc->file_header, VARIANT(SOME, {
		      // check if this is a new file and needs a file header
		      fseek(fp, 0L, SEEK_END);
		      sz = ftell(fp);

		      if (sz == 0) {
			      // new file add header
			      StringPtr header =
				  BORROW(lc->file_header, header);
			      char *hdr = unwrap(&header);
			      fprintf(fp, "%s\n", hdr);
			      fflush(fp);
			      sz = strlen(hdr);
		      }
	      }));
	return Ok(sz);
}

Result Log_build_fp(LogConfig *lc) {
	u64 ret_fp = 0;
	u64 cur_size = 0;
	MATCH(lc->log_file_path, VARIANT(SOME, {
		      StringPtr value = BORROW(lc->log_file_path, value);
		      char *lfp = unwrap(&value);
		      FILE *fp = myfopen(lfp, "a");
		      if (!fp) {
			      int err = errno;
			      char *error_message = strerror(err);
			      char buf[strlen(error_message) + strlen(lfp) + 5];
			      strcpy(buf, lfp);
			      strcat(buf, ": ");
			      strcat(buf, error_message);
			      Error e = ERR(IO_ERROR, buf);
			      return Err(e);
		      }

		      Result r = Log_update_init_state(fp, lc);
		      if (IS_ERR(r)) {
			      myfclose(fp);
			      Error e = UNWRAP_ERR(r);
			      return Err(e);
		      }
		      cur_size = TRY(r, cur_size);
		      ret_fp = (u64)fp;
	      }));

	TuplePtr ret = TUPLE(ret_fp, cur_size);
	return Ok(ret);
}

Result Log_build_impl(int n, va_list ptr, bool is_rc) {
	LogConfig lc;

	lc.show_log_level = true;
	lc.show_timestamp = true;
	lc.show_terminal = true;
	lc.formatter_size = 10000;
	lc.max_age_millis = 1000 * 60 * 60; // 1 hr
	lc.max_size_bytes = 1024 * 1024;    // 1 mb
	lc.delete_rotation = false;
	lc.show_linenum = false;
	lc.show_colors = true;
	lc.show_millis = true;
	lc.auto_rotate = false;
	lc.is_sync = false;
	lc.line_num_max_len = 30;
	lc.log_file_path = None;
	lc.file_header = None;

	for (int i = 0; i < n; i++) {
		LogConfigOption next;
		Rc rc = BUILD(Rc);
		if (!is_rc) {
			next = va_arg(ptr, LogConfigOption);
			NO_CLEANUP(rc);
		} else {
			rc = va_arg(ptr, Rc);
			void *vptr = unwrap(&rc);
			memcpy(&next, vptr, mysize(vptr));
		}

		// clang-format off
		MATCH(next,
			VARIANT(SHOW_TIMESTAMP, {
				lc.show_timestamp = ENUM_VALUE(lc.show_timestamp, bool, next);
			}) VARIANT(SHOW_LOG_LEVEL, {
				lc.show_log_level = ENUM_VALUE(lc.show_log_level, bool, next);
			}) VARIANT(FORMATTER_SIZE, {
				lc.formatter_size = ENUM_VALUE(lc.formatter_size, u64, next);
			}) VARIANT(SHOW_MILLIS, {
				lc.show_millis = ENUM_VALUE(lc.show_millis, bool, next);
			}) VARIANT(LOG_FILE_PATH, {
				String s = ENUM_VALUE(s, String, next);
				String smyclone;
				myclone(&smyclone, &s);
				lc.log_file_path = Some(smyclone);
			}) VARIANT(FILE_HEADER, {
				String s = ENUM_VALUE(s, String, next);
				String smyclone;
				myclone(&smyclone, &s);
				lc.file_header = Some(smyclone);
			}) VARIANT(SHOW_COLORS, {
				lc.show_colors = ENUM_VALUE(lc.show_colors, bool, next);
			}) VARIANT(LOG_SYNC, {
                                lc.is_sync = ENUM_VALUE(lc.is_sync, bool, next);
			}) VARIANT(SHOW_TERMINAL, {
				lc.show_terminal = ENUM_VALUE(lc.show_terminal, bool, next);
			}) VARIANT(AUTO_ROTATE, {
				lc.auto_rotate = ENUM_VALUE(lc.auto_rotate, bool, next);
			}) VARIANT(DELETE_ROTATION, {
				lc.delete_rotation = ENUM_VALUE(lc.delete_rotation, bool, next);
			}) VARIANT(MAX_AGE_MILLIS, {
                                lc.max_age_millis = ENUM_VALUE(lc.max_age_millis, u64, next);
			}) VARIANT(MAX_SIZE_BYTES, {
                                lc.max_size_bytes = ENUM_VALUE(lc.max_size_bytes, u64, next);
			}) VARIANT(LINENUM_MAX_LEN, {
                                lc.line_num_max_len = ENUM_VALUE(lc.line_num_max_len, u64, next);
                        }) VARIANT(SHOW_LINENUM, {
				lc.show_linenum = ENUM_VALUE(lc.show_linenum, bool, next);
			})
		);
		// clang-format on
	}

	Result r1 = Log_build_fp(&lc);
	FormatterPtr f = FORMATTER(lc.formatter_size);

	// if there's an error cleanup to free allocated resources
	Slab lock;
	if (IS_ERR(r1)) {
		lock.data = NULL;
		LogPtr ret = BUILD(Log, lc, NULL, f, 0, 0, lock);
		cleanup(&ret);
		return r1;
	}
	Tuple t = TRY(r1, t);
	u64 fp64 = ELEMENT_AT(t, 0, fp64);
	u64 cur_size = ELEMENT_AT(t, 1, cur_size);
	FILE *fp = (FILE *)fp64;

	if (lc.is_sync) {
		if (mymalloc(&lock, sizeof(pthread_mutex_t))) {
			lock.data = NULL;
			LogPtr ret = BUILD(Log, lc, NULL, f, 0, 0, lock);
			cleanup(&ret);
			return STATIC_ALLOC_ERROR_RESULT;
		}
		if (pthread_mutex_init((pthread_mutex_t *)lock.data, NULL)) {
			lock.data = NULL;
			LogPtr ret = BUILD(Log, lc, NULL, f, 0, 0, lock);
			cleanup(&ret);
			Error e = ERR(PTHREAD_ERROR, "could not init pthread");
			return Err(e);
		}
	} else {
		lock.data = NULL;
	}
	LogPtr ret = BUILD(Log, lc, fp, f, cur_size, log_now(), lock);

	return Ok(ret);
}

Result Log_build_rc(int n, ...) {
	va_list ptr;
	va_start(ptr, n);
	ResultPtr ret = Log_build_impl(n, ptr, true);
	va_end(ptr);
	return ret;
}

Result Log_build(int n, ...) {
	va_list ptr;
	va_start(ptr, n);
	ResultPtr ret = Log_build_impl(n, ptr, false);
	va_end(ptr);
	return ret;
}
