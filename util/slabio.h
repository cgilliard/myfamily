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

#ifndef _SLABIO_UTIL__
#define _SLABIO_UTIL__

#include <util/slabs.h>
#include <base/ser.h>

#define SLABIO_NEXT_POINTER_SIZE 8

struct SlabReader {
	u64 cur_slab_id;
	u64 cur_slab_offset;
	SlabAllocator *sa;
};
typedef struct SlabReader SlabReader;

struct SlabWriter {
	u64 cur_slab_id;
	u64 cur_slab_offset;
	SlabAllocator *sa;
};
typedef struct SlabWriter SlabWriter;

int slab_reader_init(SlabReader *sr, SlabAllocator *sa);
int slab_writer_init(SlabWriter *sw, SlabAllocator *sa);

void slab_reader_seek(Reader *sr, u64 slab_id, u64 offset);
void slab_writer_seek(Writer *sw, u64 slab_id, u64 offset);

int slab_reader_skip(Reader *sr, u64 bytes);
int slab_writer_skiip(Writer *sw, u64 bytes);

int slab_reader_read_fixed_bytes(Reader *reader, unsigned char *buffer, u64 len);
int slab_writer_write_fixed_bytes(Writer *writer, unsigned char *buffer, u64 len);

u64 slab_reader_cur_id(Reader *reader);
u64 slab_writer_cur_id(Reader *writer);

u64 slab_reader_cur_offset(Reader *reader);
u64 slab_writer_cur_offset(Writer *writer);

#endif // _SLABIO_UTIL__
