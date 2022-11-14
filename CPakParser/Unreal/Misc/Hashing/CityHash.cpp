import CityHash;

#include <memory>

#define uint32_in_expected_order(x) (x)
#define uint64_in_expected_order(x) (x)
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)

static uint64_t UNALIGNED_LOAD64(const char* p) {
	uint64_t result;
	memcpy(&result, p, sizeof(result));
	return result;
}

static uint32_t UNALIGNED_LOAD32(const char* p) {
	uint32_t result;
	memcpy(&result, p, sizeof(result));
	return result;
}

static __forceinline uint64_t Fetch64(const char* p) {
	return uint64_in_expected_order(UNALIGNED_LOAD64(p));
}

static __forceinline uint64_t Rotate(uint64_t val, int shift) {
	// Avoid shifting by 64: doing so yields an undefined result.
	return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

static __forceinline uint64_t ShiftMix(uint64_t val) {
	return val ^ (val >> 47);
}

static __forceinline uint32_t Fetch32(const char* p) {
	return uint32_in_expected_order(UNALIGNED_LOAD32(p));
}

namespace CityHash_Internal
{
	// Some primes between 2^63 and 2^64 for various uses.
	static const uint64_t k0 = 0xc3a5c85c97cb3127ULL;
	static const uint64_t k1 = 0xb492b66fbe98f273ULL;
	static const uint64_t k2 = 0x9ae16a3b2f90404fULL;

	// Magic numbers for 32-bit hashing.  Copied from Murmur3.
	static const uint32_t c1 = 0xcc9e2d51;
	static const uint32_t c2 = 0x1b873593;
}

static uint64_t HashLen16(uint64_t u, uint64_t v, uint64_t mul) {
	// Murmur-inspired hashing.
	uint64_t a = (u ^ v) * mul;
	a ^= (a >> 47);
	uint64_t b = (v ^ a) * mul;
	b ^= (b >> 47);
	b *= mul;
	return b;
}

static uint64_t HashLen0to16(const char* s, uint32_t len) 
{
	using namespace CityHash_Internal;

	if (len >= 8) {
		uint64_t mul = k2 + len * 2;
		uint64_t a = Fetch64(s) + k2;
		uint64_t b = Fetch64(s + len - 8);
		uint64_t c = Rotate(b, 37) * mul + a;
		uint64_t d = (Rotate(a, 25) + b) * mul;
		return HashLen16(c, d, mul);
	}
	if (len >= 4) {
		uint64_t mul = k2 + len * 2;
		uint64_t a = Fetch32(s);
		return HashLen16(len + (a << 3), Fetch32(s + len - 4), mul);
	}
	if (len > 0) {
		uint8_t a = s[0];
		uint8_t b = s[len >> 1];
		uint8_t c = s[len - 1];
		uint32_t y = static_cast<uint32_t>(a) + (static_cast<uint32_t>(b) << 8);
		uint32_t z = len + (static_cast<uint32_t>(c) << 2);
		return ShiftMix(y * k2 ^ z * k0) * k2;
	}
	return k2;
}

static uint64_t HashLen17to32(const char* s, uint32_t len) {
	using namespace CityHash_Internal;

	uint64_t mul = k2 + len * 2;
	uint64_t a = Fetch64(s) * k1;
	uint64_t b = Fetch64(s + 8);
	uint64_t c = Fetch64(s + len - 8) * mul;
	uint64_t d = Fetch64(s + len - 16) * k2;
	return HashLen16(Rotate(a + b, 43) + Rotate(c, 30) + d,
		a + Rotate(b + k2, 18) + c, mul);
}

static uint64_t HashLen33to64(const char* s, uint32_t len) {
	using namespace CityHash_Internal;

	uint64_t mul = k2 + len * 2;
	uint64_t a = Fetch64(s) * k2;
	uint64_t b = Fetch64(s + 8);
	uint64_t c = Fetch64(s + len - 24);
	uint64_t d = Fetch64(s + len - 32);
	uint64_t e = Fetch64(s + 16) * k2;
	uint64_t f = Fetch64(s + 24) * 9;
	uint64_t g = Fetch64(s + len - 8);
	uint64_t h = Fetch64(s + len - 16) * mul;
	uint64_t u = Rotate(a + g, 43) + (Rotate(b, 30) + c) * 9;
	uint64_t v = ((a + g) ^ d) + f + 1;
	uint64_t w = bswap_64((u + v) * mul) + h;
	uint64_t x = Rotate(e + f, 42) + c;
	uint64_t y = (bswap_64((v + w) * mul) + g) * mul;
	uint64_t z = e + f + c;
	a = bswap_64((x + z) * mul + y) + b;
	b = ShiftMix((z + a) * mul + d + h) * mul;
	return b + x;
}

static Uint128_64 WeakHashLen32WithSeeds(
	uint64_t w, uint64_t x, uint64_t y, uint64_t z, uint64_t a, uint64_t b) {
	a += w;
	b = Rotate(b + a + z, 21);
	uint64_t c = a;
	a += x;
	a += y;
	b += Rotate(a, 44);
	return { (a + z), (b + c) };
}

static Uint128_64 WeakHashLen32WithSeeds(
	const char* s, uint64_t a, uint64_t b) {
	return WeakHashLen32WithSeeds(Fetch64(s),
		Fetch64(s + 8),
		Fetch64(s + 16),
		Fetch64(s + 24),
		a,
		b);
}

template<typename T>
__forceinline void SwapValues(T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}

static __forceinline uint64_t HashLen16(uint64_t u, uint64_t v) {
	return CityHash128to64({ u, v });
}

uint64_t CityHash64(const char* s, uint32_t len)
{
	using namespace CityHash_Internal;

	if (len <= 32) {
		if (len <= 16) {
			return HashLen0to16(s, len);
		}
		else {
			return HashLen17to32(s, len);
		}
	}
	else if (len <= 64) {
		return HashLen33to64(s, len);
	}

	// For strings over 64 bytes we hash the end first, and then as we
	// loop we keep 56 bytes of state: v, w, x, y, and z.
	uint64_t x = Fetch64(s + len - 40);
	uint64_t y = Fetch64(s + len - 16) + Fetch64(s + len - 56);
	uint64_t z = HashLen16(Fetch64(s + len - 48) + len, Fetch64(s + len - 24));
	Uint128_64 v = WeakHashLen32WithSeeds(s + len - 64, len, z);
	Uint128_64 w = WeakHashLen32WithSeeds(s + len - 32, y + k1, x);
	x = x * k1 + Fetch64(s);

	// Decrease len to the nearest multiple of 64, and operate on 64-byte chunks.
	len = (len - 1) & ~static_cast<uint32_t>(63);
	do {
		x = Rotate(x + y + v.lo + Fetch64(s + 8), 37) * k1;
		y = Rotate(y + v.hi + Fetch64(s + 48), 42) * k1;
		x ^= w.hi;
		y += v.lo + Fetch64(s + 40);
		z = Rotate(z + w.lo, 33) * k1;
		v = WeakHashLen32WithSeeds(s, v.hi * k1, x + w.lo);
		w = WeakHashLen32WithSeeds(s + 32, z + w.hi, y + Fetch64(s + 16));
		SwapValues(z, x);
		s += 64;
		len -= 64;
	} while (len != 0);
	return HashLen16(HashLen16(v.lo, w.lo) + ShiftMix(y) * k1 + z,
		HashLen16(v.hi, w.hi) + x);
}