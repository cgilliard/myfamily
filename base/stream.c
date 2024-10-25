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

#include <base/fam_err.h>
#include <base/os.h>
#include <base/print_util.h>
#include <base/stream.h>
#include <base/types.h>

u8 *sgets(u8 *buf, u64 limit, Stream *strm) {
	if (!buf || limit == 0 || strm == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}
	i64 len = strm_read(strm, buf, limit - 1);
	if (len <= 0) {
		return NULL;
	}

	i64 i;
	for (i = 0; i < len; i++) {
		if (buf[len] == '\n') {
			i++;
			break;
		}
	}
	buf[i] = '\0';
	return buf;
}
