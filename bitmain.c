#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "bitslice.h"


void throw_bs(struct bs_state *bs, int count){
	siphash_bs_calculate(bs);

	int i;
	for (i = 0; i < count; i++) {
		uint64_t hash = siphash_bs_hash(bs, i);
		printf("%016llx\n", hash);
	}
	siphash_bs_reset(bs);
}


int main() {
	struct bs_state *bs = malloc(sizeof(struct bs_state));

	char key[16] = {0};

	siphash_bs_init(bs, key);

	int count = 0;

	while (1) {
		char buf[1024];
		char *s = fgets(buf, sizeof(buf), stdin);
		if (!s) break;
		siphash_bs_add(bs, buf, strlen(buf));
		count += 1;
		if (count == VEC_SZ) {
			throw_bs(bs, count);
			count = 0;
		}
	}

	if (count > 0)
		throw_bs(bs, count);

	return 0;
}
