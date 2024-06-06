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

int serialize_f64(void *obj, Writer *writer);
int deserialize_f64(void *obj, Reader *reader);

int serialize_f32(void *obj, Writer *writer);
int deserialize_f32(void *obj, Reader *reader);

int serialize_u128(void *obj, Writer *writer);
int deserialize_u128(void *obj, Reader *reader);

int serialize_u64(void *obj, Writer *writer);
int deserialize_u64(void *obj, Reader *reader);

int serialize_u32(void *obj, Writer *writer);
int deserialize_u32(void *obj, Reader *reader);

int serialize_u16(void *obj, Writer *writer);
int deserialize_u16(void *obj, Reader *reader);

int serialize_u8(void *obj, Writer *writer);
int deserialize_u8(void *obj, Reader *reader);

int serialize_i128(void *obj, Writer *writer);
int deserialize_i128(void *obj, Reader *reader);

int serialize_i64(void *obj, Writer *writer);
int deserialize_i64(void *obj, Reader *reader);

int serialize_i32(void *obj, Writer *writer);
int deserialize_i32(void *obj, Reader *reader);

int serialize_i16(void *obj, Writer *writer);
int deserialize_i16(void *obj, Reader *reader);

int serialize_i8(void *obj, Writer *writer);
int deserialize_i8(void *obj, Reader *reader);

int serialize_string(void *obj, Writer *writer);
int deserialize_string(void *obj, Reader *reader);

int serialize_bool(void *obj, Writer *writer);
int deserialize_bool(void *obj, Reader *reader);

int __serialize_not_implemented_(void *obj, Reader *reader);
int __deserialize_not_implemented_(void *obj, Writer *writer);

int bin_reader_read_fixed_bytes(Reader *reader, unsigned char *buffer, u64 len);
int bin_writer_write_fixed_bytes(Writer *writer, unsigned char *buffer, u64 len);

Reader build_reader(int (*read_fixed_bytes)(struct Reader *reader, unsigned char *buffer, u64 len), void *ptr);
#define READER(read_fixed_bytes, ptr) build_reader(read_fixed_bytes, ptr)

Writer build_writer(int (*write_fixed_bytes)(struct Writer *writer, unsigned char *buffer, u64 len), void *ptr);
#define WRITER(write_fixed_bytes, ptr) build_writer(write_fixed_bytes, ptr)

int serialize(Serializable *s, Writer *writer);
int deserialize(Serializable *s, Reader *reader);

#define SERIALIZE_NAME(x) _Generic((x), \
    f32: serialize_f32, \
    f64: serialize_f64, \
    u128: serialize_u128, \
    u64: serialize_u64, \
    u32: serialize_u32, \
    u16: serialize_u16, \
    u8: serialize_u8, \
    i128: serialize_i128, \
    i64: serialize_i64, \
    i32: serialize_i32, \
    i16: serialize_i16, \
    i8: serialize_i8, \
    String: serialize_string, \
    bool: serialize_bool, \
    default: __serialize_not_implemented_ \
) \

#define DESERIALIZE_NAME(x) _Generic((x), \
    f32: deserialize_f32, \
    f64: deserialize_f64, \
    u128: deserialize_u128, \
    u64: deserialize_u64, \
    u32: deserialize_u32, \
    u16: deserialize_u16, \
    u8: deserialize_u8, \
    i128: deserialize_i128, \
    i64: deserialize_i64, \
    i32: deserialize_i32, \
    i16: deserialize_i16, \
    i8: deserialize_i8, \
    String: deserialize_string, \
    bool: deserialize_bool, \
    default: __deserialize_not_implemented_ \
) \

#define SER(x) build_serializable(&x, SERIALIZE_NAME(x), DESERIALIZE_NAME(x))
Serializable build_serializable(void *ptr, int (*serialize)(void *obj, Writer *writer), int (*deserialize)(void *obj, Reader *reader));



#endif // _SER_BASE__
