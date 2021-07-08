#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define TIMING_ON_WIN
#elif defined(__UNIX__) || defined(__APPLE__)
#define TIMING_ON_UNIX
#endif

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////
#include <stdint.h>

#if defined(TIMING_ON_WIN)
typedef int64_t tickcount_t;
#elif defined(TIMING_ON_UNIX)
typedef struct timespect *tickcount_t;
#else
#  error Unknown platform. Unable to use high-resolution processor counter.
#endif

// Return high-resolution processor counter.
inline tickcount_t timing_getticks();

// Compute micro-seconds from start and end counter values.
inline int64_t timing_timediff(tickcount_t start, tickcount_t end);

////////////////////////////////////////
#if defined(TIMING_ON_WIN)
#include <windows.h>

inline tickcount_t timing_getticks() {
    static LARGE_INTEGER ticks;
    if (QueryPerformanceCounter(&ticks) == 0) return 0;
    return ticks.QuadPart;
}

inline int64_t timing_timediff(tickcount_t start, tickcount_t end) {
    static LARGE_INTEGER freq;
    static bool success = QueryPerformanceFrequency(&freq);

    return ((end - start) * 1000000)/freq.QuadPart;
}

#elif defined(TIMING_ON_UNIX)
#include <time.h>

inline int64_t timing_getticks() {
    return 0;
}

inline int64_t timing_timediff(int64_t start, int64_t end) {
    return end - start;
}

#endif


////////////////////////////////////////
#ifdef __cplusplus
}
#endif
