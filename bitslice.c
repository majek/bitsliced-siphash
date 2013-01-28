#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <emmintrin.h>

#include "timing.h"

#if defined(__APPLE__)
#  include <libkern/OSByteOrder.h>
#  define le64toh(x) OSSwapLittleToHostInt64(x)
#else
#  include <endian.h>
#endif


#include "bitslice.h"
#include "vec.h"

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint64_t vec __attribute__ ((vector_size (VEC_SZ/8)));
const vec zero = {0ULL, 0ULL, 0ULL, 0ULL};
const vec one = {0xffffffffffffffffULL, 0xffffffffffffffffULL,
		 0xffffffffffffffffULL, 0xffffffffffffffffULL};

/* Bit-transposition code stolen from:
   http://mischasan.wordpress.com/2011/10/03/the-full-sse2-bit-matrix-transpose-routine/ */
#define BYTES(x) (((x) + 7) >> 3)
#define DOBLOCK(Rows, Bits)						\
        for (cc = 0; cc < ncols; cc += 8 ) {				\
		for (i = 0; i < Rows; ++i)				\
			minp.b[i] = inp[(rr + i)*cb + (cc >> 3)];	\
		for (i = 8; --i >= 0; ) {				\
			*(uint##Bits##_t*)&out[(rr >> 3) + (cc + i)*rb] = \
				_mm_movemask_epi8(minp.x);		\
			minp.x = _mm_slli_epi64(minp.x, 1);		\
		}							\
        }

static inline void transpose(uint8_t const *inp, uint8_t *out, int nrows, int ncols) {
	int rr, cc, i, cb = BYTES(ncols), rb = BYTES(nrows), left = nrows & 15;
	union { __m128i x; uint8_t b[16]; } minp;
	for (rr = 0; rr <= nrows - 16; rr += 16)
		DOBLOCK(16, 16);
	if (left > 8 ) {
		DOBLOCK(left, 16);
	} else if (left > 0) {
		DOBLOCK(left,  8 );
	}
}

static inline void transpose_to_vec(vec vec[64], u64 raw[VEC_SZ]) {
	transpose((u8*)raw, (u8*)vec, VEC_SZ, 64);
}

static inline void transpose_to_raw(u64 raw[VEC_SZ], vec vec[64]) {
	transpose((u8*)vec, (u8*)raw, 64, VEC_SZ);
}


void siphash_bs_init(struct bs_state *bs, const char key[16]) {
	memset(bs, 0, sizeof(struct bs_state));

	u64 k0 = le64toh(*(u64 *)(key));
	u64 k1 = le64toh(*(u64 *)(key + 8));

	bs->v[0] = k0 ^ 0x736f6d6570736575ULL;
	bs->v[1] = k1 ^ 0x646f72616e646f6dULL;
	bs->v[2] = k0 ^ 0x6c7967656e657261ULL;
	bs->v[3] = k1 ^ 0x7465646279746573ULL;
}

int siphash_bs_add(struct bs_state *bs, const char *src, unsigned src_sz) {
	unsigned idx = bs->idx;
	bs->idx += 1;
	assert(idx < VEC_SZ);

	int level = 0;

	u64 b = (u64)src_sz << 56;
	u64 *in = (u64*)src;
	while (src_sz >= 8) {
		u64 mi = le64toh(*in);
		in += 1; src_sz -= 8;

		bs->m[level++][idx] = mi;
		assert(src_sz < 8);
	}

	u64 t = 0; u8 *pt = (u8 *)&t; u8 *m = (u8 *)in;
	switch (src_sz) {
	case 7: pt[6] = m[6];
	case 6: pt[5] = m[5];
	case 5: pt[4] = m[4];
	case 4: *((u32*)&pt[0]) = *((u32*)&m[0]); break;
	case 3: pt[2] = m[2];
	case 2: pt[1] = m[1];
	case 1: pt[0] = m[0];
	}
	bs->m[level++][idx] = b | le64toh(t);

	assert(level == 1);
	return idx;
}

void siphash_bs_reset(struct bs_state *bs) {
	bs->idx = 0;
}

static void _siphash_bs_calculate(struct bs_state *bs,
				  uint64_t *trans_cyc, uint64_t *counting_cyc) {
	vec m[64];

	uint64_t cycles0, cycles1;

	vec v0[64], v1[64], v2[64], v3[64];

	RDTSC_START(cycles0);

	vec_fill(v0, bs->v[0]);
	vec_fill(v1, bs->v[1]);
	vec_fill(v2, bs->v[2]);
	vec_fill(v3, bs->v[3]);

	RDTSC_STOP(cycles1);
	*counting_cyc = cycles1 - cycles0;

	#if 0
	// first compression
	transpose_to_vec(m, bs->m[BLAH]);

	vec_xor(v3, m);
	vec_xor(v3, m);
	double_round(v0, v1, v2, v3);
	vec_xor(v0, m);
	#endif

	// padding
	RDTSC_START(cycles0);
	transpose_to_vec(m, bs->m[0]);
	RDTSC_STOP(cycles1);
	*trans_cyc = cycles1 - cycles0;

	RDTSC_START(cycles0);
	vec_xor(v3, m);
	double_round(v0, v1, v2, v3);
	vec_xor(v0, m);

	// finalization
	vec_xor_0xff(v2);

	double_round(v0, v1, v2, v3);
	double_round(v0, v1, v2, v3);

	vec_4xor(m, v0, v1, v2, v3);

	RDTSC_STOP(cycles1);
	*counting_cyc += cycles1 - cycles0;

	RDTSC_START(cycles0);
	transpose_to_raw(bs->results, m);
	RDTSC_STOP(cycles1);
	*trans_cyc += cycles1 - cycles0;
}

void siphash_bs_calculate(struct bs_state *bs) {
	unsigned repeat;
	uint64_t min_trans = -1, min_count = -1;
	for (repeat = 0; repeat < REPEATS; repeat++) {
		uint64_t trans_cyc, counting_cyc;
		_siphash_bs_calculate(bs, &trans_cyc, &counting_cyc);
		min_trans = MIN(min_trans, trans_cyc);
		min_count = MIN(min_count, counting_cyc);
	}
	fprintf(stderr, "cycles: counting=%llu transpose=%llu\n",
		min_count, min_trans);
}

uint64_t siphash_bs_hash(struct bs_state *bs, unsigned idx) {
	assert(idx < bs->idx);

	return bs->results[idx];
}
