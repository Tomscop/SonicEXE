#ifndef PSXF_GUARD_EVENT_H
#define PSXF_GUARD_EVENT_H

#include "../psx/io.h"
#include "../psx/gfx.h"
#include "../psx/pad.h"

#include "../psx/fixed.h"
#include "../stage/character.h"
#include "../stage/player.h"
#include "../stage/object.h"
#include "../fonts/font.h"

typedef struct
{
	u8 flash_col;
} Event;

extern Event event;

void FollowCharCamera();
void Events();
void NoteHitEvent(u8 type);
void NoteMissEvent(u8 type, u8 state);
void Flash_tick();

#endif