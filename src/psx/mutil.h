#ifndef PSXF_GUARD_MUTIL_H
#define PSXF_GUARD_MUTIL_H

#include "psx.h"

#include "fixed.h"

//Math utility functions
s16 MUtil_Sin(u8 x);
s16 MUtil_Cos(u8 x);
int lerp(fixed_t a, fixed_t b, fixed_t t);
void MUtil_RotatePoint(POINT *p, s16 s, s16 c);
fixed_t MUtil_Pull(fixed_t a, fixed_t b, fixed_t t);

#endif