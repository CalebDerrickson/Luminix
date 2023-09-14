#include "lmath.h"
#include "platform/platform.h"

#include <math.h>
#include <stdlib.h>

static b8 rand_seed = false;

f32 lsin(f32 x)
{
    return sinf(x);

}

f32 lcos(f32 x)
{
    return cosf(x);

}

f32 ltan(f32 x)
{
    return tanf(x);

}

f32 lacos(f32 x)
{
    return acosf(x);

}

f32 lsqrt(f32 x)
{
    return sqrtf(x);

}

f32 labsf(f32 x)
{
    return fabsf(x);

}

i32 lrandom()
{
    if (!rand_seed) {
        srand((u32)platform_get_absolute_time());
        rand_seed = true;
    }
    return rand();
}

i32 lrandom_in_range(i32 min, i32 max)
{
    if (!rand_seed) {
        srand((u32)platform_get_absolute_time());
        rand_seed = true;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 flrandom()
{
    return (f32)lrandom() / (f32)RAND_MAX;

}

f32 flrandom_in_range(f32 min, f32 max)
{
    return min + ((f32)lrandom() / ((f32)RAND_MAX / (max - min)));

}