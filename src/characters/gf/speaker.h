#ifndef PSXF_GUARD_SPEAKER_H
#define PSXF_GUARD_SPEAKER_H

#include "../../psx/gfx.h"
#include "../../psx/fixed.h"

//Speaker structure
typedef struct
{
	//Speaker state
	Gfx_Tex tex;
	fixed_t bump;
} Speaker;

//Speaker functions
void Speaker_Init(Speaker *this);
void Speaker_Bump(Speaker *this);
void Speaker_Tick(Speaker *this, fixed_t x, fixed_t y, fixed_t parallax);

#endif