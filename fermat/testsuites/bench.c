#define CU_ENABLE_TIMER
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cu/cu.h>
#include <fermat/vec3.h>
#include "data.h"


#define REPEATS 400

int fd;

#define prTime() printf("%15s: %ld %09ld\n", __func__, t->tv_sec, t->tv_nsec)

__attribute__((noinline)) static void add(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_vec3_t v;
    fer_real_t d = 0;

    ferVec3Set(&v, FER_ZERO, FER_ZERO, FER_ZERO);
    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len; i++){
            ferVec3Add(&v, &vecs[i]);
        }
    }
    t = cuTimerStop();
    d = ferVec3X(&v);
    prTime();

    write(fd, (void *)&d, 1);
}

__attribute__((noinline)) static void sub(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_vec3_t v;
    fer_real_t d = 0;

    ferVec3Set(&v, FER_ZERO, FER_ZERO, FER_ZERO);
    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len; i++){
            ferVec3Sub(&v, &vecs[i]);
        }
    }
    t = cuTimerStop();
    d = ferVec3X(&v);
    prTime();

    write(fd, (void *)&d, 1);
}

__attribute__((noinline)) static void scale(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_vec3_t v;
    fer_real_t d = 0;

    ferVec3Copy(&v, &vecs[0]);
    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len; i++){
            ferVec3Scale(&v, ferVec3X(&vecs[i]));
        }
    }
    t = cuTimerStop();
    d = ferVec3X(&v);
    prTime();

    write(fd, (void *)&d, 1);
}

__attribute__((noinline)) static void normalize(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_vec3_t v;
    fer_real_t d = 0;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len; i++){
            ferVec3Copy(&v, &vecs[i]);
            ferVec3Normalize(&v);
        }
    }
    t = cuTimerStop();
    d = ferVec3X(&v);
    prTime();

    write(fd, (void *)&d, 1);
}

__attribute__((noinline)) static void dot(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_real_t dot = 0;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len - 1; i++){
            dot += ferVec3Dot(&vecs[i], &vecs[i + 1]);
        }
    }
    t = cuTimerStop();
    prTime();

    write(fd, (void *)&dot, 1);
}

__attribute__((noinline)) static void cross(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_vec3_t v;
    fer_real_t d = 0;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len - 1; i++){
            ferVec3Cross(&v, &vecs[i], &vecs[i + 1]);
        }
    }
    d = ferVec3X(&v);
    t = cuTimerStop();
    prTime();

    write(fd, (void *)&d, 1);
}

__attribute__((noinline)) static void len2(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_real_t len = 0;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len; i++){
            len += ferVec3Len2(&vecs[i]);
        }
    }
    t = cuTimerStop();
    prTime();

    write(fd, (void *)&len, 1);
}

__attribute__((noinline)) static void len(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_real_t len = 0;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len; i++){
            len += ferVec3Len(&vecs[i]);
        }
    }
    t = cuTimerStop();
    prTime();

    write(fd, (void *)&len, 1);
}

__attribute__((noinline)) static void dist2(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_real_t d = 0;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len - 1; i++){
            d += ferVec3Dist2(&vecs[i], &vecs[i + 1]);
        }
    }
    t = cuTimerStop();
    prTime();

    write(fd, (void *)&d, 1);
}

__attribute__((noinline)) static void dist(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_real_t d = 0;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len - 1; i++){
            d += ferVec3Dist(&vecs[i], &vecs[i + 1]);
        }
    }
    t = cuTimerStop();
    prTime();

    write(fd, (void *)&d, 1);
}

__attribute__((noinline)) static void segment_dist(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_real_t d = 0;
    fer_vec3_t w;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len - 2; i++){
            d += ferVec3PointSegmentDist2(&vecs[i], &vecs[i + 1], &vecs[i + 2], &w);
        }
    }
    t = cuTimerStop();
    prTime();

    write(fd, (void *)&d, 1);
}

__attribute__((noinline)) static void tri_dist(void)
{
    size_t i, j;
    const struct timespec *t;
    fer_real_t d = 0;
    fer_vec3_t w;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len - 3; i++){
            d += ferVec3PointTriDist2(&vecs[i], &vecs[i + 1], &vecs[i + 2], &vecs[i + 3], &w);
        }
    }
    t = cuTimerStop();
    prTime();

    write(fd, (void *)&d, 1);
}

__attribute__((noinline)) static void eq(void)
{
    size_t i, j;
    const struct timespec *t;
    int e = 0;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len - 1; i++){
            e += ferVec3Eq(&vecs[i], &vecs[i]);
            e += ferVec3Eq(&vecs[i], &vecs[i + 1]);
        }
    }
    t = cuTimerStop();
    prTime();

    write(fd, (void *)&e, 1);
}

__attribute__((noinline)) static void neq(void)
{
    size_t i, j;
    const struct timespec *t;
    int e = 0;

    cuTimerStart();
    for (j = 0; j < REPEATS; j++){
        for (i = 0; i < vecs_len - 1; i++){
            e += ferVec3NEq(&vecs[i], &vecs[i]);
            e += ferVec3NEq(&vecs[i], &vecs[i + 1]);
        }
    }
    t = cuTimerStop();
    prTime();

    write(fd, (void *)&e, 1);
}

TEST_SUITES {
    TEST_SUITES_CLOSURE
};

int main(int argc, char *argv[])
{
    if (argc != 2){
        fprintf(stderr, "Usage: %s add|sub|scale|normalize|dot|cross\n"
                        "            |len2|len|dist2|dist|segment_dist|\n"
                        "            |tri_dist|eq|neq\n", argv[0]);
        exit(-1);
    }

    fd = open("/dev/null", O_WRONLY);

    if (strcmp(argv[1], "add") == 0){
        add();
    }else if (strcmp(argv[1], "sub") == 0){
        sub();
    }else if (strcmp(argv[1], "scale") == 0){
        scale();
    }else if (strcmp(argv[1], "normalize") == 0){
        normalize();
    }else if (strcmp(argv[1], "dot") == 0){
        dot();
    }else if (strcmp(argv[1], "cross") == 0){
        cross();
    }else if (strcmp(argv[1], "len2") == 0){
        len2();
    }else if (strcmp(argv[1], "len") == 0){
        len();
    }else if (strcmp(argv[1], "dist2") == 0){
        dist2();
    }else if (strcmp(argv[1], "dist") == 0){
        dist();
    }else if (strcmp(argv[1], "segment_dist") == 0){
        segment_dist();
    }else if (strcmp(argv[1], "tri_dist") == 0){
        tri_dist();
    }else if (strcmp(argv[1], "eq") == 0){
        eq();
    }else if (strcmp(argv[1], "neq") == 0){
        neq();
    }

    close(fd);

    return 0;
}