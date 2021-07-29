#include <crash.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

// #define P 258241
// The base bit-noise constants were crafted to have distinctive and interesting
// bits, and have so far produced excellent experimental test results.
#define NOISE1 0xb5297a4d  // 0b0110'1000'1110'0011'0001'1101'1010'0100
#define NOISE2 0x68e31da4  // 0b1011'0101'0010'1001'0111'1010'0100'1101
#define NOISE3 0x1b56c4e9  // 0b0001'1011'0101'0110'1100'0100'1110'1001

uint32_t crash_noise1d(uint32_t seed, int32_t x) {
    uint32_t y = x;

    y = y*NOISE1;
    y += seed;
    y ^= y << 8;
    y += NOISE2;
    y ^= (y >> 8) * NOISE3;

    // y = y*NOISE1;
    // y += seed;
    // y ^= y >> 8;
    // y += NOISE2;
    // y ^= y << 8;
    // y *= NOISE3;
    // y ^= y >> 8;
    return y;
}

uint32_t crash_noise2d(uint32_t seed, int32_t x, int32_t y) {
    return crash_noise1d(x + y*NOISE2, seed);
}

uint32_t crash_noise3d(uint32_t seed, int32_t x, int32_t y, int32_t z) {
    return crash_noise1d(x + y*NOISE2 + z*NOISE3, seed);
}

CrashState crash_seed(uint32_t seed) {
    struct CrashState state = {seed, 0};
    return state;
}

uint32_t crash_rand(CrashState *state) {
    state->position = crash_noise1d(state->position, state->seed);
    return state->position;
}

bool crash_roll_with_prob(CrashState *state, double p) {
    return (crash_rand(state)/(double)UINT32_MAX) <= p;
}

int32_t crash_choose_int32(CrashState *state, int32_t lo, int32_t hi) {
    return lo + crash_rand(state)%(hi - lo);
}