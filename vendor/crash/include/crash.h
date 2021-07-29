#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrashState {
    uint32_t seed;
    uint32_t position;
} CrashState;

uint32_t crash_noise1d(uint32_t seed, int32_t x);
uint32_t crash_noise2d(uint32_t seed, int32_t x, int32_t y);
uint32_t crash_noise3d(uint32_t seed, int32_t i, int32_t y, int32_t z);

CrashState crash_seed(uint32_t seed);
uint32_t crash_rand(CrashState *state);
bool crash_roll_with_prob(CrashState *state, double p);
int32_t crash_choose_int32(CrashState *state, int32_t lo, int32_t hi);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif