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

#include <base/ser.h>
#include <base/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int serialize(Serializable* s, Writer* writer)
{
    return s->serialize(s->ptr, writer);
}

int deserialize(Serializable* s, Reader* reader)
{
    return s->deserialize(s->ptr, reader);
}

Reader build_reader(int (*read_fixed_bytes)(struct Reader* reader,
                        unsigned char* buffer, u64 len),
    void* ptr)
{
    Reader ret;
    ret.ptr = ptr;
    ret.read_fixed_bytes = read_fixed_bytes;
    return ret;
}

Serializable build_serializable(void* ptr,
    int (*serialize)(void* obj, Writer* writer),
    int (*deserialize)(void* obj, Reader* reader))
{
    Serializable ret;
    ret.ptr = ptr;
    ret.serialize = serialize;
    ret.deserialize = deserialize;
    return ret;
}

Writer build_writer(int (*write_fixed_bytes)(struct Writer* writer,
                        unsigned char* buffer, u64 len),
    void* ptr)
{
    Writer ret;
    ret.ptr = ptr;
    ret.write_fixed_bytes = write_fixed_bytes;
    return ret;
}

int bin_reader_read_fixed_bytes(Reader* reader, unsigned char* buffer,
    u64 len)
{
    memcpy(buffer, reader->ptr, len);
    reader->ptr += len;
    return 0;
}

int bin_writer_write_fixed_bytes(Writer* writer, unsigned char* buffer,
    u64 len)
{
    memcpy(writer->ptr, buffer, len);
    writer->ptr += len;
    return 0;
}

int serialize_i128(void* obj, Writer* writer)
{
    return serialize_u128(obj, writer);
}

int deserialize_i128(void* obj, Reader* reader)
{
    return deserialize_u128(obj, reader);
}

int serialize_f64(void* obj, Writer* writer)
{
    return serialize_u64(obj, writer);
}

int deserialize_f64(void* obj, Reader* reader)
{
    return deserialize_u64(obj, reader);
}

int serialize_f32(void* obj, Writer* writer)
{
    return serialize_u32(obj, writer);
}

int deserialize_f32(void* obj, Reader* reader)
{
    return deserialize_u32(obj, reader);
}

int serialize_i64(void* obj, Writer* writer)
{
    return serialize_u64(obj, writer);
}

int deserialize_i64(void* obj, Reader* reader)
{
    return deserialize_u64(obj, reader);
}

int serialize_i32(void* obj, Writer* writer)
{
    return serialize_u32(obj, writer);
}

int deserialize_i32(void* obj, Reader* reader)
{
    return deserialize_u32(obj, reader);
}

int serialize_i16(void* obj, Writer* writer)
{
    return serialize_u16(obj, writer);
}

int deserialize_i16(void* obj, Reader* reader)
{
    return deserialize_u16(obj, reader);
}

int serialize_i8(void* obj, Writer* writer)
{
    return serialize_u8(obj, writer);
}

int deserialize_i8(void* obj, Reader* reader)
{
    return deserialize_u8(obj, reader);
}

int deserialize_bool(void* obj, Reader* reader)
{
    unsigned char buffer[1];
    int ret = reader->read_fixed_bytes(reader, buffer, 1);

    if (ret == 0) {
        *((bool*)obj) = false;
        if (buffer[0])
            *((bool*)obj) = true;
    }

    return ret;
}

int serialize_bool(void* obj, Writer* writer)
{
    unsigned char buffer[1];

    if (*((bool*)obj)) {
        buffer[0] = 1;
    } else {
        buffer[0] = 0;
    }

    return writer->write_fixed_bytes(writer, buffer, 1);
}

int deserialize_u8(void* obj, Reader* reader)
{
    unsigned char buffer[1];
    int ret = reader->read_fixed_bytes(reader, buffer, 1);

    if (ret == 0) {
        *((u8*)obj) = 0;
        *((u8*)obj) += (u8)(buffer[0] & 0xFF);
    }

    return ret;
}

int serialize_u8(void* obj, Writer* writer)
{
    unsigned char buffer[1];

    buffer[0] = *((u8*)obj) & 0xFF;

    return writer->write_fixed_bytes(writer, buffer, 1);
}

int deserialize_u16(void* obj, Reader* reader)
{
    unsigned char buffer[2];
    int ret = reader->read_fixed_bytes(reader, buffer, 2);

    if (ret == 0) {
        *((u16*)obj) = 0;
        *((u16*)obj) += (u16)(buffer[1] & 0xFF);
        *((u16*)obj) += (u16)(buffer[0] & 0xFF) << 8;
    }

    return ret;
}

int serialize_u16(void* obj, Writer* writer)
{
    unsigned char buffer[2];

    u16 value = *((u16*)obj);
    buffer[1] = value & 0xFF;
    value >>= 8;
    buffer[0] = value & 0xFF;

    return writer->write_fixed_bytes(writer, buffer, 2);
}

int deserialize_u32(void* obj, Reader* reader)
{
    unsigned char buffer[4];
    int ret = reader->read_fixed_bytes(reader, buffer, 4);

    if (ret == 0) {
        *((u32*)obj) = 0;
        *((u32*)obj) += (u32)(buffer[3] & 0xFF);
        *((u32*)obj) += (u32)(buffer[2] & 0xFF) << 8;
        *((u32*)obj) += (u32)(buffer[1] & 0xFF) << 16;
        *((u32*)obj) += (u32)(buffer[0] & 0xFF) << 24;
    }

    return ret;
}

int serialize_u32(void* obj, Writer* writer)
{
    unsigned char buffer[4];

    u32 value = *((u32*)obj);
    buffer[3] = value & 0xFF;
    value >>= 8;
    buffer[2] = value & 0xFF;
    value >>= 8;
    buffer[1] = value & 0xFF;
    value >>= 8;
    buffer[0] = value & 0xFF;

    return writer->write_fixed_bytes(writer, buffer, 4);
}

int deserialize_u64(void* obj, Reader* reader)
{
    unsigned char buffer[8];
    int ret = reader->read_fixed_bytes(reader, buffer, 8);

    if (ret == 0) {
        *((u64*)obj) = 0;
        *((u64*)obj) += (u64)(buffer[7] & 0xFF);
        *((u64*)obj) += (u64)(buffer[6] & 0xFF) << 8;
        *((u64*)obj) += (u64)(buffer[5] & 0xFF) << 16;
        *((u64*)obj) += (u64)(buffer[4] & 0xFF) << 24;
        *((u64*)obj) += (u64)(buffer[3] & 0xFF) << 32;
        *((u64*)obj) += (u64)(buffer[2] & 0xFF) << 40;
        *((u64*)obj) += (u64)(buffer[1] & 0xFF) << 48;
        *((u64*)obj) += (u64)(buffer[0] & 0xFF) << 56;
    }

    return ret;
}

int serialize_u64(void* obj, Writer* writer)
{
    unsigned char buffer[8];

    u64 value = *((u64*)obj);
    buffer[7] = value & 0xFF;
    value >>= 8;
    buffer[6] = value & 0xFF;
    value >>= 8;
    buffer[5] = value & 0xFF;
    value >>= 8;
    buffer[4] = value & 0xFF;
    value >>= 8;
    buffer[3] = value & 0xFF;
    value >>= 8;
    buffer[2] = value & 0xFF;
    value >>= 8;
    buffer[1] = value & 0xFF;
    value >>= 8;
    buffer[0] = value & 0xFF;

    return writer->write_fixed_bytes(writer, buffer, 8);
}

int deserialize_u128(void* obj, Reader* reader)
{
    unsigned char buffer[16];
    int ret = reader->read_fixed_bytes(reader, buffer, 16);

    if (ret == 0) {
        *((u128*)obj) = 0;
        *((u128*)obj) += (u128)(buffer[15] & 0xFF);
        *((u128*)obj) += (u128)(buffer[14] & 0xFF) << 8;
        *((u128*)obj) += (u128)(buffer[13] & 0xFF) << 16;
        *((u128*)obj) += (u128)(buffer[12] & 0xFF) << 24;
        *((u128*)obj) += (u128)(buffer[11] & 0xFF) << 32;
        *((u128*)obj) += (u128)(buffer[10] & 0xFF) << 40;
        *((u128*)obj) += (u128)(buffer[9] & 0xFF) << 48;
        *((u128*)obj) += (u128)(buffer[8] & 0xFF) << 56;
        *((u128*)obj) += (u128)(buffer[7] & 0xFF) << 64;
        *((u128*)obj) += (u128)(buffer[6] & 0xFF) << 72;
        *((u128*)obj) += (u128)(buffer[5] & 0xFF) << 80;
        *((u128*)obj) += (u128)(buffer[4] & 0xFF) << 88;
        *((u128*)obj) += (u128)(buffer[3] & 0xFF) << 96;
        *((u128*)obj) += (u128)(buffer[2] & 0xFF) << 104;
        *((u128*)obj) += (u128)(buffer[1] & 0xFF) << 112;
        *((u128*)obj) += (u128)(buffer[0] & 0xFF) << 120;
    }

    return ret;
}

int serialize_u128(void* obj, Writer* writer)
{
    unsigned char buffer[16];

    u128 value = *((u128*)obj);
    buffer[15] = value & 0xFF;
    value >>= 8;
    buffer[14] = value & 0xFF;
    value >>= 8;
    buffer[13] = value & 0xFF;
    value >>= 8;
    buffer[12] = value & 0xFF;
    value >>= 8;
    buffer[11] = value & 0xFF;
    value >>= 8;
    buffer[10] = value & 0xFF;
    value >>= 8;
    buffer[9] = value & 0xFF;
    value >>= 8;
    buffer[8] = value & 0xFF;
    value >>= 8;
    buffer[7] = value & 0xFF;
    value >>= 8;
    buffer[6] = value & 0xFF;
    value >>= 8;
    buffer[5] = value & 0xFF;
    value >>= 8;
    buffer[4] = value & 0xFF;
    value >>= 8;
    buffer[3] = value & 0xFF;
    value >>= 8;
    buffer[2] = value & 0xFF;
    value >>= 8;
    buffer[1] = value & 0xFF;
    value >>= 8;
    buffer[0] = value & 0xFF;

    return writer->write_fixed_bytes(writer, buffer, 16);
}

int serialize_string(void* obj, Writer* writer)
{
    int ret = 0;
    u64 len = ((String*)obj)->len;
    serialize_u64(&len, writer);
    ret = writer->write_fixed_bytes(writer, (unsigned char*)((String*)obj)->ptr,
        len);

    return ret;
}

int deserialize_string(void* obj, Reader* reader)
{
    int ret = 0;
    ((String*)obj)->ptr = NULL;

    u64 len;
    ret = deserialize_i64(&len, reader);

    if (!ret) {
        unsigned char* ptr = malloc(sizeof(char) * (len + 1));
        if (ptr == NULL)
            ret = -1;
        else {
            ret = reader->read_fixed_bytes(reader, ptr, len);
        }

        if (!ret) {
            ((String*)obj)->ptr = (char*)ptr;
            ((String*)obj)->len = len;
        }
    }

    return ret;
}
