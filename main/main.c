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

// #include <core/std.h>
// #include <main/main.h>
#include <assert.h>
#include <core/stack.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

void format_time(long nanoseconds) {
	long seconds = nanoseconds / 1e9;
	nanoseconds %= (long)1e9;

	long minutes = seconds / 60;
	seconds %= 60;

	long hours = minutes / 60;
	minutes %= 60;

	long milliseconds = nanoseconds / 1e6;
	nanoseconds %= (long)1e6;

	long microseconds = nanoseconds / 1e3;
	nanoseconds %= (long)1e3;

	printf("Elapsed time: %02ld:%02ld:%02ld.%03ld:%03ld:%03ld "
	       "(hh:mm:ss.ms:us:ns)\n",
	       hours, minutes, seconds, milliseconds, microseconds,
	       nanoseconds);
}

int main(int argc, char **argv) {
	StackAllocatorConfig config = DEFAULT_STACK_ALLOCATOR_CONFIG;
	StackAllocator sa;
	build_stack(&config, &sa);
	void *sp = sa.begin.data;
	size_t my_offset1;
	size_t my_offset2;

	int r = allocate_stack(sizeof(u32), &sa);
	(*((u32 *)(sp + 0))) = 0;
	printf("value=%i\n", *((u32 *)(sp + 0)));

	struct timespec start, end;

	clock_gettime(CLOCK_MONOTONIC, &start);

	u64 target = 1000;
	u32 x;
	for (u64 k = 0; k < target; k++) {
		//(*((u32 *)(sp + 0))) = 0;

		x = 0;
		for (u64 j = 0; j < target; j++) {
			//(*((u32 *)(sp + 0))) = 0;
			x = 0;
			for (u64 i = 0; i < target; i++)
				x += 1;
			//(*((u32 *)(sp + 0)))++;
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	long elapsed_ns =
	    (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
	// printf("value=%i\n", *((u32 *)(sp + 0)));
	// assert(target == (*((u32 *)(sp + 0))));
	printf("value=%i\n", x);
	assert(target = x);
	format_time(elapsed_ns);

	return 0;
}
