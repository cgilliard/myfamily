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

#include <core/file.h>
#include <core/unit.h>
#include <errno.h>

GETTER(File, fp)
SETTER(File, fp)

void File_cleanup(File *ptr) {
	FILE *fp = GET(File, ptr, fp);
	if (fp) {
		printf("fclose %i\n", fp);
		myfclose(fp);
		File_set_fp(ptr, NULL);
	}
}
Result File_open(char *path, OpenOptions opt) {
	FilePtr ret = BUILD(File);
	FILE *fp;
	if (opt == OpenRead)
		fp = myfopen(path, "r");
	else if (opt == OpenWrite)
		fp = myfopen(path, "w");
	else if (opt == OpenAppend)
		fp = myfopen(path, "a");
	else if (opt == OpenReadExtended)
		fp = myfopen(path, "r+");
	else if (opt == OpenWriteExtended)
		fp = myfopen(path, "w+");
	else if (opt == OpenAppendExtended)
		fp = myfopen(path, "a+");
	else {
		Error err = ERR(FILE_OPEN_ERROR, "Unexpected open options");
		return Err(err);
	}
	File_set_fp(&ret, fp);

	if (fp == NULL) {
		char *err_text = strerror(errno);
		Error err = ERR(FILE_OPEN_ERROR,
				"Attempt to open file '%s' generated error: %s",
				path, err_text);
		return Err(err);
	}
	printf("fp open %i\n", fp);

	return Ok(ret);
}

Result File_read(File *ptr, char *buf, u64 len) {
	FILE *fp = GET(File, ptr, fp);
	u64 val = fread(buf, 1, len, fp);
	return Ok(val);
}
Result File_seek(File *ptr, u64 pos) {
	FILE *fp = GET(File, ptr, fp);
	if (fseek(fp, pos, SEEK_SET) != 0) {
		char *err_text = strerror(errno);
		Error err =
		    ERR(FILE_SEEK_ERROR, "Attempt to seek generated error: %s",
			err_text);
		return Err(err);
	}
	return Ok(_());
}

Result File_write(File *ptr, char *buf, u64 len) {
	FILE *fp = GET(File, ptr, fp);
	errno = 0;
	u64 wlen = fwrite(buf, sizeof(char), len, fp);
	if (wlen == 0) {
		char *s = strerror(errno);
		Error e = ERR(IO_ERROR, "write error: %s", s);
		return Err(e);
	}

	return Ok(wlen);
}
Result File_flush(File *ptr) {
	FILE *fp = GET(File, ptr, fp);
	if (fflush(fp)) {
		char *s = strerror(errno);
		Error e = ERR(IO_ERROR, "flush error: %s", s);
		return Err(e);
	}
	return Ok(_());
}
