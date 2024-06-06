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

#include <util/slabio.h>
#include <string.h>
#include <log/log.h>
#include <limits.h>

#define LOG_LEVEL Info

int slab_reader_next_slab(SlabReader *reader, Slab *slab) {
	int ret = 0;
	u64 next = ULONG_MAX;

	void *ptr = slab->data + (slab->len - SLABIO_NEXT_POINTER_SIZE);
	for(int i=slab->len - SLABIO_NEXT_POINTER_SIZE; i<slab->len; i++) {
		debug("slab->data[%d]=%i", i, ((char*)slab->data)[i]);
	}
        Reader bin_reader = READER(bin_reader_read_fixed_bytes, ptr);
        ret = deserialize_u64(&next, &bin_reader);
	debug("next=%llu", next);
        reader->cur_slab_id = next;
        reader->cur_slab_offset = 0;

	return ret;
}

int slab_reader_read_fixed_bytes(Reader *reader, unsigned char *buffer, u64 len) {
	int ret = 0;
	SlabReader *sr = reader->ptr;
	Slab slab;
	u64 slab_len = ULONG_MAX;
	u64 len_sum = 0;

	debug(
		"read cur_slab=%llu,offset=%llu,len=%llu",
		sr->cur_slab_id, sr->cur_slab_offset, len
	);

	while(true) {
		if(sr->cur_slab_offset >= slab_len - SLABIO_NEXT_POINTER_SIZE) {
			slab_reader_next_slab(sr, &slab);
		}
		debug("cur_slab=%llu", sr->cur_slab_id);

		slab_read(sr->sa, sr->cur_slab_id, &slab);

		slab_len = slab.len;
		u64 slab_rem = (slab_len - SLABIO_NEXT_POINTER_SIZE) - sr->cur_slab_offset;
		u64 wlen;

		if(slab_len <= SLABIO_NEXT_POINTER_SIZE) {
                        error(
                                "illegal slab size. slab_size must be greater than %llu",
                                SLABIO_NEXT_POINTER_SIZE
                        );
                        ret = -1;
                        break;
                }

		if(slab_rem > len) {
                        wlen = len;
                } else {
                        wlen = slab_rem;
                }

		if(wlen > 0) {
			memcpy(buffer + len_sum, slab.data + sr->cur_slab_offset, wlen);
			sr->cur_slab_offset += wlen;
                	len -= wlen;
			len_sum += wlen;
		}

		if(len == 0)
			break;
	}

	debug("read complete. Read %llu bytes.", len_sum);
	return ret;
}

int slab_writer_allocate_slab(SlabWriter *writer, Slab *slab) {
	int ret = 0;

	u64 next = slab_allocate(writer->sa, slab->len);
	if(next == ULONG_MAX)
		ret = -1;

	debug("allocate %llu", next);

	if(!ret) {
		void *ptr = slab->data + (slab->len - SLABIO_NEXT_POINTER_SIZE);
		Writer bin_writer = WRITER(bin_writer_write_fixed_bytes, ptr);
		ret = serialize_u64(&next, &bin_writer);
		writer->cur_slab_id = next;
		writer->cur_slab_offset = 0;
	}

	return ret;
}

int slab_writer_write_fixed_bytes(Writer *writer, unsigned char *buffer, u64 len) {
	int ret = 0;
	SlabWriter *sw = writer->ptr;
	Slab slab;
	u64 len_sum = 0;
	u64 slab_len = ULONG_MAX;

	debug("write cur_slab=%llu,len=%llu", sw->cur_slab_id, len);

	while(true) {
		if(sw->cur_slab_offset >= slab_len - SLABIO_NEXT_POINTER_SIZE) {
			// allocate
			slab_writer_allocate_slab(sw, &slab);
		}
		slab_read(sw->sa, sw->cur_slab_id, &slab);

		slab_len = slab.len;
		u64 slab_rem = (slab_len - SLABIO_NEXT_POINTER_SIZE) - sw->cur_slab_offset;
		u64 wlen;

		if(slab_len <= SLABIO_NEXT_POINTER_SIZE) {
			error(
				"illegal slab size. slab_size must be greater than %llu",
				SLABIO_NEXT_POINTER_SIZE
			);
			ret = -1;
			break;
		}

		if(slab_rem > len) {
			wlen = len;
		} else {
			wlen = slab_rem;
		}

		debug("slab_len=%llu,wlen=%llu", slab_len, wlen);

		if(wlen > 0) {
			memcpy(slab.data + sw->cur_slab_offset, buffer + len_sum, wlen);
			sw->cur_slab_offset += wlen;
			len -= wlen;
			len_sum += wlen;
		}

		if(len == 0)
			break;
	}

	debug("slab write complete len = %llu", len_sum);

	return ret;
}

int slab_reader_init(SlabReader *sr, SlabAllocator *sa) {
	sr->sa = sa;
	sr->cur_slab_id = ULONG_MAX;
        sr->cur_slab_offset = 0;
	return 0;
}

int slab_writer_init(SlabWriter *sw, SlabAllocator *sa) {
	sw->sa = sa;
	sw->cur_slab_id = ULONG_MAX;
	sw->cur_slab_offset = 0;

	return 0;
}

void slab_reader_seek(Reader *reader, u64 slab_id, u64 offset) {
	SlabReader *sr = reader->ptr;
	sr->cur_slab_id = slab_id;
	sr->cur_slab_offset = offset;
}

void slab_writer_seek(Writer *writer, u64 slab_id, u64 offset) {
	SlabWriter *sw = writer->ptr;
	sw->cur_slab_id = slab_id;
        sw->cur_slab_offset = offset;
}

int slab_reader_skip(Reader *sr, u64 bytes) { return 0; }
int slab_writer_skiip(Writer *sw, u64 bytes) { return 0; }

u64 slab_reader_cur_id(Reader *reader) { return 0; }
u64 slab_writer_cur_id(Reader *writer) { return 0; }

u64 slab_reader_cur_offset(Reader *reader) { return 0; }
u64 slab_writer_cur_offset(Writer *writer) { return 0; }

