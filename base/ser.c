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

#include <base/types.h>
#include <base/ser.h>
#include <string.h>
#include <stdio.h>

int serialize(Serializable *s, Writer *writer) {
	return s->serialize(s->ptr, writer);
}

int deserialize(Serializable *s, Reader *reader) {
	return s->deserialize(s->ptr, reader);
}

Reader build_reader(int (*read_fixed_bytes)(struct Reader *reader, unsigned char *buffer, u64 len), void *ptr)
{
        Reader ret;
        ret.ptr = ptr;
        ret.read_fixed_bytes = read_fixed_bytes;
        return ret;
}

Serializable build_serializable(
	void *ptr,
	int (*serialize)(void *obj, Writer *writer),
	int (*deserialize)(void *obj, Reader *reader)
) {
	Serializable ret;
	ret.ptr = ptr;
	ret.serialize = serialize;
	ret.deserialize = deserialize;
	return ret;
}

Writer build_writer(int (*write_fixed_bytes)(struct Writer *writer, unsigned char *buffer, u64 len), void *ptr) {
	Writer ret;
	ret.ptr = ptr;
	ret.write_fixed_bytes = write_fixed_bytes;
	return ret;
}

int bin_reader_read_fixed_bytes(Reader *reader, unsigned char *buffer, u64 len) {
	memcpy(buffer, reader->ptr, len);
	reader->ptr += len;
	return 0;
}

int bin_writer_write_fixed_bytes(Writer *writer, unsigned char *buffer, u64 len) {
	memcpy(writer->ptr, buffer, len);
	writer->ptr += len;
	return 0;
}


int deserialize_u64(void *obj, Reader *reader) {
	unsigned char buffer[8];
	int ret = reader->read_fixed_bytes(reader, buffer, 8);

	if(ret == 0) {
		*((u64 *)obj) = 0;
		*((u64 *)obj) += (u64)(buffer[7] & 0xFF);
		*((u64 *)obj) += (u64)(buffer[6] & 0xFF) << 8;
		*((u64 *)obj) += (u64)(buffer[5] & 0xFF) << 16;
		*((u64 *)obj) += (u64)(buffer[4] & 0xFF) << 24;
		*((u64 *)obj) += (u64)(buffer[3] & 0xFF) << 32;
		*((u64 *)obj) += (u64)(buffer[2] & 0xFF) << 40;
		*((u64 *)obj) += (u64)(buffer[1] & 0xFF) << 48;
		*((u64 *)obj) += (u64)(buffer[0] & 0xFF) << 56;
	}

	return ret;
}

int serialize_u64(void *obj, Writer *writer) {
	unsigned char buffer[8];

	u64 value = *((u64 *)obj);
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
