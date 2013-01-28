
#define vec_fill(a, value)					\
	do {							\
		unsigned o;					\
		u64 _value = value;				\
		for (o = 0; o < 64; o++) {			\
			if (_value & (0x1ULL << o)) {		\
				a[o] = one;			\
			} else {				\
				a[o] = zero;			\
			}					\
		}						\
	} while (0)

#define vec_xor(dst, b)				\
	do {					\
		unsigned o;			\
		for (o = 0; o < 64; o++) {	\
			dst[o] ^= b[o];		\
		}	       	       		\
	} while (0)

#define vec_4xor(dst, a, b, c, d)				\
	do {							\
		unsigned o;					\
		for (o = 0; o < 64; o++) {			\
			dst[o] = (a[o] ^ b[o]) ^ (c[o] ^ d[o]);	\
		}						\
	} while (0)

#define double_round(v0, v1, v2, v3)					\
	do {								\
		vec _v0[64], _v1[64], _v2[64], _v3[64];			\
		HALF_ROUND(v0, v1, v2, v3, 13, 16, _v0, _v1, _v2, _v3);	\
		HALF_ROUND(_v2, _v1, _v0, _v3, 17, 21, v2, v1, v0, v3);	\
		HALF_ROUND(v0, v1, v2, v3, 13, 16, _v0, _v1, _v2, _v3);	\
		HALF_ROUND(_v2, _v1, _v0, _v3, 17, 21, v2, v1, v0, v3);	\
	} while (0)


/* Add without carry (first addition) */
#define ADDZ(a, b, out, carry)					\
	do {							\
		vec __a = a, __b = b;				\
		out = __a ^ __b;				\
		carry = __a & __b;				\
	} while (0)

/* Add with carry */
#define ADD(a, b, out, carry)					\
	do {							\
		vec __a = a, __b = b;				\
		vec _a_or_b = __a ^ __b;			\
		out = carry ^ _a_or_b;				\
		carry = (_a_or_b & carry) | (__a & __b);	\
	} while (0)

#define M(e) (e) & 0x3f
#define HALF_ROUND(a,b,c,d,s,t, _a, _b, _c, _d)			\
	do {							\
		unsigned o;					\
		vec tmp, carry;					\
		carry = zero;					\
		for (o = 0; o < 64; o++) {			\
			ADD(a[o], b[o], tmp, carry);		\
			_a[M(o-32)] = tmp;			\
			_b[o] = b[M(o-s)] ^ tmp;		\
		}						\
		carry = zero;					\
		for (o = 0; o < 64; o++) {			\
			ADD(c[o], d[o], tmp, carry);		\
			_c[o] = tmp;				\
			_d[o] = d[M(o-t)] ^ tmp;		\
		}						\
	} while (0)

/* Xor lower 8 bits with 0xff. */
#define vec_xor_0xff(v)							\
	do {								\
		v[0] ^= one; v[1] ^= one; v[2] ^= one; v[3] ^= one;	\
		v[4] ^= one; v[5] ^= one; v[6] ^= one; v[7] ^= one;	\
	} while (0)
