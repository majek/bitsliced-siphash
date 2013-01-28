
/* vec is 256 bits long, AVX size */
#define VEC_SZ 256

#define REPEATS 1000


struct bs_state {
	/* Initial key ^ constants */
	uint64_t v[4];

	/* Input data for compressions and padding */
	uint64_t m[2][VEC_SZ];

	/* Computed hashes */
	uint64_t results[VEC_SZ];

	unsigned idx;
};

void siphash_bs_init(struct bs_state *bs, const char key[16]);
int siphash_bs_add(struct bs_state *bs, const char *src, unsigned src_sz);
void siphash_bs_reset(struct bs_state *bs);
void siphash_bs_calculate(struct bs_state *bs);
uint64_t siphash_bs_hash(struct bs_state *bs, unsigned idx);
