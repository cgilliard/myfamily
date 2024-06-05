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

#ifndef _SER_BASE__
#define _SER_BASE__

struct Reader {
	int (*read_fixed_bytes)(struct Reader *reader, unsigned char *buffer, u64 len);
	void *ptr;
};
typedef struct Reader Reader;

struct Writer {
	int (*write_fixed_bytes)(struct Writer *writer, unsigned char *buffer, u64 len);
	void *ptr;
};
typedef struct Writer Writer;

struct Serializable {
	void *ptr;
	int (*serialize)(void *obj, Writer *writer);
	int (*deserialize)(void *obj, Reader *reader);
};
typedef struct Serializable Serializable;

int serialize_u64(void *obj, Writer *writer);
int deserialize_u64(void *obj, Reader *reader);

int bin_reader_read_fixed_bytes(Reader *reader, unsigned char *buffer, u64 len);
int bin_writer_write_fixed_bytes(Writer *writer, unsigned char *buffer, u64 len);

Reader build_reader(int (*read_fixed_bytes)(struct Reader *reader, unsigned char *buffer, u64 len), void *ptr);
#define READER(read_fixed_bytes, ptr) build_reader(read_fixed_bytes, ptr)

Writer build_writer(int (*write_fixed_bytes)(struct Writer *writer, unsigned char *buffer, u64 len), void *ptr);
#define WRITER(write_fixed_bytes, ptr) build_writer(write_fixed_bytes, ptr)

int serialize(Serializable *s, Writer *writer);
int deserialize(Serializable *s, Reader *reader);

Serializable build_serializable(void *ptr, int (*serialize)(void *obj, Writer *writer), int (*deserialize)(void *obj, Reader *reader));
#define SER(ptr, serialize, deserialize) build_serializable(ptr, serialize, deserialize)

/*
#ifndef typeof
#ifdef __clang__
#define typeof(x) __typeof__(x)
#endif
#endif
*/

#define TYPE_NAME(x) _Generic((x), \
    int:     "int", \
    float:   "float", \
    u64: "u64", \
    default: "not_implemented")

#endif // _SER_BASE__
