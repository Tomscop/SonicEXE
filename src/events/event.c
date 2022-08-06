#include "event.h"
#include "../stage/stage.h"

#include "../psx/mem.h"
#include "../psx/timer.h"
#include "../psx/audio.h"
#include "../psx/pad.h"
#include "../psx/random.h"
#include "../psx/movie.h"
#include "../psx/mutil.h"

Event event;

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
		event.hitstatic = 10;
	}
}

#include "../stages/ycr/ycr.h"
#include "../characters/ycn/ycn.h"

static void You_Cant_Run_Encore()
{
	FntPrint("steps: %d", stage.song_step);
	if (stage.song_step < 528)
		FollowCharCamera();
	if(stage.song_step == 528)
	{
		StagePixel();
		YCNChangePhase(1);
	}
	if(stage.song_step == 784)
	{
		StageNormal();
		YCNChangePhase(2);
	}
}

static void Too_Slow()
{
	
}

static void Static_tick()
{
	if (event.hitstatic > 0)
	{
		RECT src = {0, 0, 256, 256};
		RECT dst = {
			0,
			event.hitstatic * 10,
			330,
			250
		};
		if (event.hitstatic > 7)
			Gfx_BlendTex(&event.tex_static, &src, &dst, 10);
		else if (event.hitstatic > 4)
			Gfx_DrawTex(&event.tex_static, &src, &dst);
		else if (event.hitstatic > 1)
			Gfx_BlendTex(&event.tex_static, &src, &dst, 10);
		
		RECT src2 = {0, 0, 256, 256};
		RECT dst2 = {
			0,
			-248 + event.hitstatic * 10,
			330,
			250
		};
		if (event.hitstatic > 7)
			Gfx_BlendTex(&event.tex_static, &src2, &dst2, 10);
		else if (event.hitstatic > 4)
			Gfx_DrawTex(&event.tex_static, &src2, &dst2);
		else if (event.hitstatic > 1)
			Gfx_BlendTex(&event.tex_static, &src2, &dst2, 10);
		
		event.hitstatic -= 1;
	}
}

void Events()
{
	Flash_tick();
	Static_tick();
	if (stage.stage_id == StageId_YouCantRunEncore)
		You_Cant_Run_Encore();
	if (stage.stage_id == StageId_TooSlow)
		Too_Slow();
	if(stage.followcamera && !(stage.stage_id == StageId_YouCantRunEncore))
		FollowCharCamera();
}

void LoadEvents()
{
	Gfx_LoadTex(&event.tex_static, IO_Read("\\STAGE\\STATIC.TIM;1"), GFX_LOADTEX_FREE);
}

void FollowCharCamera()
{
	u8 sensitivity = 4;
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

void Flash_tick()
{
	if (event.flash_col > 10)
	{
		static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
		Gfx_BlendRect(&flash, event.flash_col, event.flash_col, event.flash_col, 1);
		event.flash_col -= 10;
	}
	else
		event.flash_col = 0;
}
