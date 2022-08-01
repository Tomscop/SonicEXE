#include "event.h"
#include "../stage/stage.h"

#include "../psx/mem.h"
#include "../psx/timer.h"
#include "../psx/audio.h"
#include "../psx/pad.h"
#include "../psx/random.h"
#include "../psx/movie.h"
#include "../psx/mutil.h"

void Events()
{		
	if(stage.followcamera)
		FollowCharCamera();
}

void FollowCharCamera()
{
	u8 sensitivity = 5;
	if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
	{
		if (stage.opponent->animatable.anim == CharAnim_Up)
			stage.camera.y -= FIXED_DEC(sensitivity,10);
		if (stage.opponent->animatable.anim == CharAnim_Down)
			stage.camera.y += FIXED_DEC(sensitivity,10);
		if (stage.opponent->animatable.anim == CharAnim_Left)
			stage.camera.x -= FIXED_DEC(sensitivity,10);
		if (stage.opponent->animatable.anim == CharAnim_Right)
			stage.camera.x += FIXED_DEC(sensitivity,10);
	}
	else
	{
		if (stage.player->animatable.anim == CharAnim_Up)
			stage.camera.y -= FIXED_DEC(sensitivity,10);
		if (stage.player->animatable.anim == CharAnim_Down)
			stage.camera.y += FIXED_DEC(sensitivity,10);
		if (stage.player->animatable.anim == CharAnim_Left)
			stage.camera.x -= FIXED_DEC(sensitivity,10);
		if (stage.player->animatable.anim == CharAnim_Right)
			stage.camera.x += FIXED_DEC(sensitivity,10);
	}
}

void NoteHitEvent(u8 type)
{

}

void NoteMissEvent(u8 type, u8 state)
{
	PlayerState *this = &stage.player_state[state];
	if (type & NOTE_FLAG_STATIC)
	{
		Audio_PlaySound(stage.sound[1], 0x3fff);
		this->health -= 2000;
	}
}