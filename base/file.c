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

#include <base/ekinds.h>
#include <base/file.h>
#include <base/unit.h>
#include <errno.h>
#include <stdio.h>

GETTER(File, fp)
SETTER(File, fp)

void File_cleanup(File *ptr) {
	FILE *fp = *File_get_fp(ptr);
	if (fp) {
		fclose(fp);
		File_set_fp(ptr, NULL);
	}
}

usize File_size(File *ptr) { return sizeof(File); }

Result File_read(File *ptr, char *buf, u64 limit) {
	FILE *fp = *File_get_fp(ptr);
	u64 val = fread(buf, 1, limit, fp);
	return Ok(val);
}

Result File_seek(File *ptr, u64 pos) {
	FILE *fp = *File_get_fp(ptr);
	if (fseek(fp, pos, SEEK_SET) != 0) {
		char *err_text = strerror(errno);
		Error err =
		    ERROR(FILE_SEEK_ERROR,
			  "Attempt to seek generated error: %s", err_text);
		return Err(err);
	}
	return Ok(UNIT);
}

Result File_open(char *path, OpenOptions opt) {
	FilePtr *ret = tlmalloc(sizeof(FILE));
	if (ret == NULL) {
		Error err =
		    ERROR(ALLOC_ERROR, "Could not allocate sufficient memory");
		return Err(err);
	}
	BUILDPTR(ret, File);
	FILE *fp;
	if (opt == OpenRead)
		fp = fopen(path, "r");
	else if (opt == OpenWrite)
		fp = fopen(path, "w");
	else if (opt == OpenAppend)
		fp = fopen(path, "a");
	else if (opt == OpenReadExtended)
		fp = fopen(path, "r+");
	else if (opt == OpenWriteExtended)
		fp = fopen(path, "w+");
	else if (opt == OpenAppendExtended)
		fp = fopen(path, "a+");
	else {
		Error err = ERROR(FILE_OPEN_ERROR, "Unexpected open options");
		return Err(err);
	}
	File_set_fp(ret, fp);

	if (fp == NULL) {
		tlfree(ret);
		char *err_text = strerror(errno);
		Error err =
		    ERROR(FILE_OPEN_ERROR,
			  "Attempt to open file '%s' generated error: %s", path,
			  err_text);
		return Err(err);
	}

	Rc rc = RC(ret);
	return Ok(rc);
}

Result File_flush(File *ptr){todo()}

Result File_write(File *ptr, char *buf, u64 len) {
	todo()
}
