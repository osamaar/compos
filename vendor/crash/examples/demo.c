#include <crash.h>
#include <stdio.h>
#include <limits.h>

#define NBUCKETS 10
#define STEP ((unsigned)(UINT_MAX/NBUCKETS))
#define N 10000000

int main() {
    int buckets[NBUCKETS];
    for (int i = 0; i < NBUCKETS; i++) buckets[i] = 0;

    for (int i = 0; i < N; i++) {
        unsigned x = crash_noise1d(i, 9);
        if (x < 10000) printf("%8u ", x);
        buckets[x/STEP]++;
    }

    printf("\n");

    for (int i = 0; i < NBUCKETS; i++) {
        printf("Bucket %d: %d\n", i, buckets[i]);
    }

    return 0;
}