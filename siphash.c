#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "timing.h"

#define VEC_SZ 256
#define REPEATS 1000

uint64_t siphash24(const char *src, unsigned long src_sz, const char key[16]);


struct {
	unsigned sz[VEC_SZ];
	char buf[VEC_SZ][16];
} sh;

void throw_sh(int count) {
	char key[16] = {0};
	uint64_t results[VEC_SZ];

	unsigned repeat;
	uint64_t min_cycles = -1;
	for (repeat = 0; repeat < REPEATS; repeat++) {
		uint64_t cycles0, cycles1;
		RDTSC_START(cycles0);
		int i;
		for (i = 0; i < count; i++) {
			results[i] = siphash24(sh.buf[i], sh.sz[i], key);
		}
		RDTSC_STOP(cycles1);
		uint64_t diff = cycles1 - cycles0;
		if (diff > 1) // for some reason on vmware this is 1
			min_cycles = MIN(min_cycles, diff);
	}
	fprintf(stderr, "cycles=%llu\n", min_cycles);

	int i;
	for (i = 0; i < count; i++) {
		printf("%016llx\n", results[i]);
	}
}

int main() {
	int count = 0;

	while (1) {
		char buf[1024];
		char *s = fgets(buf, sizeof(buf), stdin);
		if (!s) break;
		sh.sz[count] = strlen(buf);
		memcpy(sh.buf[count], buf, sh.sz[count]);
		count += 1;

		if (count == VEC_SZ) {
			throw_sh(count);
			count = 0;
		}
	}

	if (count > 0)
		throw_sh(count);

	return 0;
}
