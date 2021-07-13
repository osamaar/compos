#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define TIMING_ON_WIN
#elif defined(__unix__) || defined(__APPLE__)
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
#include <time.h>
typedef struct timespec tickcount_t;
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
inline tickcount_t timing_getticks() {
    timespec temp;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &temp);;
    return  temp;
}

inline int64_t timing_timediff(tickcount_t start, tickcount_t end) {
    //return (end.tv_nsec - start.tv_nsec)*1000;
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp.tv_sec*1000000 + temp.tv_nsec/1000;
}

#endif


////////////////////////////////////////
#ifdef __cplusplus
}
#endif
