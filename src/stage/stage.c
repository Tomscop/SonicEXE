#include "stage.h"

#include "../psx/mem.h"
#include "../psx/timer.h"
#include "../psx/audio.h"
#include "../psx/pad.h"
#include "../main.h"
#include "../psx/random.h"
#include "../psx/movie.h"
#include "../psx/mutil.h"
#include "../psx/save.h"

#include "../menu/menu.h"
#include "../psx/trans.h"
#include "../psx/loadscr.h"

#include "object/combo.h"
#include "object/splash.h"

#include "../events/event.h"

//Stage constants
//#define STAGE_PERFECT //Play all notes perfectly
//#define STAGE_NOHUD //Disable the HUD

//#define STAGE_FREECAM //Freecam

static const u8 charicon[3][3][4] = {
	//BF
	{
		//Normal
		{93,10,46,30},
		//Happy
		{1,10,47,31},
		//Sad
		{49,10,43,35}
	},
	//SonicEXE
	{
		//Normal
		{141,10,43,39},
		//Happy
		{141,10,43,39},
		//Sad
		{185,10,40,45}
	},
	//YCN
	{
		//Normal
		{0,42,48,41},
		//Happy
		{0,42,48,41},
		//Sad
		{49,46,52,44}
	}
};

static u32 note_x[8];
static u32 note_y[8];

static const u16 note_key[] = {INPUT_LEFT, INPUT_DOWN, INPUT_UP, INPUT_RIGHT};
static const u8 note_anims[4][3] = {
	{CharAnim_Left,  CharAnim_LeftAlt,  PlayerAnim_LeftMiss},
	{CharAnim_Down,  CharAnim_DownAlt,  PlayerAnim_DownMiss},
	{CharAnim_Up,    CharAnim_UpAlt,    PlayerAnim_UpMiss},
	{CharAnim_Right, CharAnim_RightAlt, PlayerAnim_RightMiss},
};

//Stage definitions
#include "../characters/bf/bf.h"
#include "../characters/bfe/bfe.h"
#include "../characters/starvedtails/stails.h"
#include "../characters/bfn/bfn.h"
#include "../characters/bfsunky/bfsunky.h"

#include "../characters/gf/gf.h"
#include "../characters/gfn/gfn.h"

#include "../characters/sonicexe/sonicexe.h"
#include "../characters/ycn/ycn.h"
#include "../characters/majin/majin.h"
#include "../characters/majinog/majinog.h"
#include "../characters/lordx/lordx.h"
#include "../characters/starvedeggman/seggm.h"
#include "../characters/nmouse/nmouse.h"
#include "../characters/sarah/sarah.h"
#include "../characters/xenophanes/xenophanes.h"

#include "../stages/gh/gh.h"
#include "../stages/ycr/ycr.h"
#include "../stages/trichael/trichael.h"
#include "../stages/starved/starved.h"

static const StageDef stage_defs[StageId_Max] = {
	#include "../songs.h"
};


//Stage state
Stage stage;

//Stage music functions
static void Stage_StartVocal(void)
{
	if (!(stage.flag & STAGE_FLAG_VOCAL_ACTIVE))
	{
		Audio_ChannelXA(stage.stage_def->music_channel);
		stage.flag |= STAGE_FLAG_VOCAL_ACTIVE;
	}
}

static void Stage_CutVocal(void)
{
	if (stage.flag & STAGE_FLAG_VOCAL_ACTIVE)
	{
		Audio_ChannelXA(stage.stage_def->music_channel + 1);
		stage.flag &= ~STAGE_FLAG_VOCAL_ACTIVE;
	}
}

//Stage camera functions
static void Stage_FocusCharacter(Character *ch, fixed_t div)
{
	//Use character focus settings to update target position and zoom
	stage.camera.tx = ch->x + ch->focus_x;
	stage.camera.ty = ch->y + ch->focus_y;
	stage.camera.tz = ch->focus_zoom;
	stage.camera.td = div;
}

static void Stage_ScrollCamera(void)
{
	#ifdef STAGE_FREECAM
		if (pad_state.held & PAD_LEFT)
			stage.camera.x -= FIXED_DEC(2,1);
		if (pad_state.held & PAD_UP)
			stage.camera.y -= FIXED_DEC(2,1);
		if (pad_state.held & PAD_RIGHT)
			stage.camera.x += FIXED_DEC(2,1);
		if (pad_state.held & PAD_DOWN)
			stage.camera.y += FIXED_DEC(2,1);
		if (pad_state.held & PAD_TRIANGLE)
			stage.camera.zoom -= FIXED_DEC(1,100);
		if (pad_state.held & PAD_CROSS)
			stage.camera.zoom += FIXED_DEC(1,100);
	#else
		//Get delta position
		fixed_t dx = stage.camera.tx - stage.camera.x;
		fixed_t dy = stage.camera.ty - stage.camera.y;
		fixed_t dz = stage.camera.tz - stage.camera.zoom;
		
		//Scroll based off current divisor
		stage.camera.x += FIXED_MUL(dx, stage.camera.td);
		stage.camera.y += FIXED_MUL(dy, stage.camera.td);
		stage.camera.zoom += FIXED_MUL(dz, stage.camera.td);
	#endif
	
	//Update other camera stuff
	stage.camera.bzoom = FIXED_MUL(stage.camera.zoom, stage.bump);
}

static void Stage_DrawStartScreen(void)
{
	u8 st_col = 255;
	static const RECT st = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	stage.startscreen -= 2;
	if(stage.startscreen < 256)
		st_col = stage.startscreen;
	Gfx_BlendRect(&st, st_col, st_col, st_col, 2);
}

//Stage section functions
static void Stage_ChangeBPM(u16 bpm, u16 step)
{
	//Update last BPM
	stage.last_bpm = bpm;
	
	//Update timing base
	if (stage.step_crochet)
		stage.time_base += FIXED_DIV(((fixed_t)step - stage.step_base) << FIXED_SHIFT, stage.step_crochet);
	stage.step_base = step;
	
	//Get new crochet and times
	stage.step_crochet = ((fixed_t)bpm << FIXED_SHIFT) * 8 / 240; //15/12/24
	stage.step_time = FIXED_DIV(FIXED_DEC(12,1), stage.step_crochet);
	
	//Get new crochet based values
	stage.early_safe = stage.late_safe = stage.step_crochet / 6; //10 frames
	stage.late_sus_safe = stage.late_safe;
	stage.early_sus_safe = stage.early_safe * 2 / 5;
}

static Section *Stage_GetPrevSection(Section *section)
{
	if (section > stage.sections)
		return section - 1;
	return NULL;
}

static u16 Stage_GetSectionStart(Section *section)
{
	Section *prev = Stage_GetPrevSection(section);
	if (prev == NULL)
		return 0;
	return prev->end;
}

//Section scroll structure
typedef struct
{
	fixed_t start;   //Seconds
	fixed_t length;  //Seconds
	u16 start_step;  //Sub-steps
	u16 length_step; //Sub-steps
	
	fixed_t size; //Note height
} SectionScroll;

static void Stage_GetSectionScroll(SectionScroll *scroll, Section *section)
{
	//Get BPM
	u16 bpm = section->flag & SECTION_FLAG_BPM_MASK;
	
	//Get section step info
	scroll->start_step = Stage_GetSectionStart(section);
	scroll->length_step = section->end - scroll->start_step;
	
	//Get section time length
	scroll->length = (scroll->length_step * FIXED_DEC(15,1) / 12) * 24 / bpm;
	
	//Get note height
	scroll->size = FIXED_MUL(stage.speed, scroll->length * (12 * 150) / scroll->length_step) + FIXED_UNIT;
}

//Note hit detection
static u8 Stage_HitNote(PlayerState *this, u8 type, fixed_t offset)
{
	//Get hit type
	if (offset < 0)
		offset = -offset;
	
	u8 hit_type;
	if (offset > stage.late_safe * 10 / 11)
		hit_type = 3; //SHIT
	else if (offset > stage.late_safe * 6 / 11)
		hit_type = 2; //BAD
	else if (offset > stage.late_safe * 3 / 11)
		hit_type = 1; //GOOD
	else
		hit_type = 0; //SICK
	
	//Increment combo and score
	this->combo++;
	
	static const s32 score_inc[] = {
		35, //SICK
		20, //GOOD
		10, //BAD
		 5, //SHIT
	};
	this->score += score_inc[hit_type];
	this->min_accuracy += 3 - hit_type;
	this->refresh_score = true;
	
	//Restore vocals and health
	Stage_StartVocal();
	u16 heal = (hit_type > 1) ? -400 * hit_type : 800;
	this->health += heal - this->antispam;
	this->refresh_accuracy = true;
	this->max_accuracy += 3;
	
	//Create combo object telling of our combo
	Obj_Combo *combo = Obj_Combo_New(
		FIXED_DEC(-100,1),
		FIXED_DEC(0,1),
		hit_type,
		this->combo >= 10 ? this->combo : 0xFFFF
	);
	if (combo != NULL)
		ObjectList_Add(&stage.objlist_fg, (Object*)combo);
	
	//Create note splashes if SICK
	if (hit_type == 0 && stage.prefs.splash)
	{
		for (int i = 0; i < 3; i++)
		{
			//Create splash object
			Obj_Splash *splash = Obj_Splash_New(
				note_x[type ^ stage.note_swap],
				note_y[type ^ stage.note_swap] * (stage.prefs.downscroll ? -1 : 1),
				type & 0x3
			);
			if (splash != NULL)
				ObjectList_Add(&stage.objlist_splash, (Object*)splash);
		}
	}
	
	return hit_type;
}

static void Stage_MissNote(PlayerState *this, u8 type)
{
	this->max_accuracy += 3;
	this->refresh_accuracy = true;
	this->miss += 1;
	this->refresh_miss = true;
	
	if (this->character->spec & CHAR_SPEC_MISSANIM)
		this->character->set_anim(this->character, note_anims[type & 0x3][2]);
	else
		this->character->set_anim(this->character, note_anims[type & 0x3][0]);
	
	if (this->combo)
	{
		//Kill combo
		if (stage.stage_id != StageId_Round_A_Bout)
		{
			if (stage.gf != NULL && this->combo > 5)
				stage.gf->set_anim(stage.gf, CharAnim_DownAlt); //Cry if we lost a large combo
			this->combo = 0;
		}
		
		//Create combo object telling of our lost combo
		Obj_Combo *combo = Obj_Combo_New(
			this->character->focus_x,
			this->character->focus_y,
			0xFF,
			0
		);
		if (combo != NULL)
			ObjectList_Add(&stage.objlist_fg, (Object*)combo);
	}
}

static void Stage_NoteCheck(PlayerState *this, u8 type)
{
	//Perform note check
	for (Note *note = stage.cur_note;; note++)
	{
		if (note->type & NOTE_FLAG_MINE)
		{
			//Check if mine can be hit
			fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
			if (note_fp - (stage.late_safe * 3 / 5) > stage.note_scroll)
				break;
			if (note_fp + (stage.late_safe * 2 / 5) < stage.note_scroll)
				continue;
			if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
				continue;
			
			//Hit the mine
			note->type |= NOTE_FLAG_HIT;
			
			//if (stage.stage_id == StageId_Clwn_4)
			NoteHitEvent(note->type);
			if (false)
				this->health = -0x7000;
			else
				this->health -= 2000;
			if (this->character->spec & CHAR_SPEC_MISSANIM)
				this->character->set_anim(this->character, note_anims[type & 0x3][2]);
			else
				this->character->set_anim(this->character, note_anims[type & 0x3][0]);
			this->arrow_hitan[type & 0x3] = -1;
			return;
		}
		else
		{
			//Check if note can be hit
			fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
			if (note_fp - stage.early_safe > stage.note_scroll)
				break;
			if (note_fp + stage.late_safe < stage.note_scroll)
				continue;
			if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
				continue;
			
			//Hit the note
			note->type |= NOTE_FLAG_HIT;
			
			NoteHitEvent(note->type);
			this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
			u8 hit_type = Stage_HitNote(this, type, stage.note_scroll - note_fp);
			this->arrow_hitan[type & 0x3] = stage.step_time;
			return;
		}
	}
	
	//Missed a note
	this->arrow_hitan[type & 0x3] = -1;
	
	if (!stage.prefs.ghost)
	{
		Stage_MissNote(this, type);
		
		this->health -= 400;
		this->score -= 1;
		this->refresh_score = true;
	}
	else if(this->antispam < 1600)
		this->antispam += 400;
}

static void Stage_SustainCheck(PlayerState *this, u8 type)
{
	//Perform note check
	for (Note *note = stage.cur_note;; note++)
	{
		//Check if note can be hit
		fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
		if (note_fp - stage.early_sus_safe > stage.note_scroll)
			break;
		if (note_fp + stage.late_sus_safe < stage.note_scroll)
			continue;
		if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || !(note->type & NOTE_FLAG_SUSTAIN))
			continue;
		
		//Hit the note
		note->type |= NOTE_FLAG_HIT;
		
		this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
		
		Stage_StartVocal();
		this->arrow_hitan[type & 0x3] = stage.step_time;
	}
}

static void Stage_ProcessPlayer(PlayerState *this, Pad *pad, boolean playing)
{
	//Handle player note presses
	if (stage.prefs.botplay == 0) {
		if (playing)
		{
			u8 i = ((this->character == stage.opponent) || (this->character == stage.opponent2)) ? NOTE_FLAG_OPPONENT : 0;
			
			this->pad_held = this->character->pad_held = pad->held;
			this->pad_press = pad->press;
			
			if (this->pad_held & INPUT_LEFT)
				Stage_SustainCheck(this, 0 | i);
			if (this->pad_held & INPUT_DOWN)
				Stage_SustainCheck(this, 1 | i);
			if (this->pad_held & INPUT_UP)
				Stage_SustainCheck(this, 2 | i);
			if (this->pad_held & INPUT_RIGHT)
				Stage_SustainCheck(this, 3 | i);
			
			if (this->pad_press & INPUT_LEFT)
				Stage_NoteCheck(this, 0 | i);
			if (this->pad_press & INPUT_DOWN)
				Stage_NoteCheck(this, 1 | i);
			if (this->pad_press & INPUT_UP)
				Stage_NoteCheck(this, 2 | i);
			if (this->pad_press & INPUT_RIGHT)
				Stage_NoteCheck(this, 3 | i);
		}
		else
		{
			this->pad_held = this->character->pad_held = 0;
			this->pad_press = 0;
		}
	}
	
	if (this->antispam > 0)
		this->antispam -= 50;
	
	FntPrint("%d",this->antispam);
	
	if (stage.prefs.botplay == 1) {
		//Do perfect note checks
		if (playing)
		{
			u8 i = ((this->character == stage.opponent) || (this->character == stage.opponent2)) ? NOTE_FLAG_OPPONENT : 0;
			
			u8 hit[4] = {0, 0, 0, 0};
			for (Note *note = stage.cur_note;; note++)
			{
				//Check if note can be hit
				fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
				if (note_fp - stage.early_safe - FIXED_DEC(12,1) > stage.note_scroll)
					break;
				if (note_fp + stage.late_safe < stage.note_scroll)
					continue;
				if ((note->type & NOTE_FLAG_MINE) || (note->type & NOTE_FLAG_OPPONENT) != i)
					continue;
				
				//Handle note hit
				if (!(note->type & NOTE_FLAG_SUSTAIN))
				{
					if (note->type & NOTE_FLAG_HIT)
						continue;
					if (stage.note_scroll >= note_fp)
						hit[note->type & 0x3] |= 1;
					else if (!(hit[note->type & 0x3] & 8))
						hit[note->type & 0x3] |= 2;
				}
				else if (!(hit[note->type & 0x3] & 2))
				{
					if (stage.note_scroll <= note_fp)
						hit[note->type & 0x3] |= 4;
					hit[note->type & 0x3] |= 8;
				}
			}
			
			//Handle input
			this->pad_held = 0;
			this->pad_press = 0;
			
			for (u8 j = 0; j < 4; j++)
			{
				if (hit[j] & 5)
				{
					this->pad_held |= note_key[j];
					Stage_SustainCheck(this, j | i);
				}
				if (hit[j] & 1)
				{
					this->pad_press |= note_key[j];
					Stage_NoteCheck(this, j | i);
				}
			}
			
			this->character->pad_held = this->pad_held;
		}
		else
		{
			this->pad_held = this->character->pad_held = 0;
			this->pad_press = 0;
		}
	}
}

//Stage drawing functions
void Stage_DrawTexCol(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 cr, u8 cg, u8 cb)
{
	fixed_t xz = dst->x;
	fixed_t yz = dst->y;
	fixed_t wz = dst->w;
	fixed_t hz = dst->h;
	
	#ifdef STAGE_NOHUD
		if (tex == &stage.tex_hud0 || tex == &stage.tex_hud1 || tex == &stage.tex_hudmajin)
			return;
	#endif
	
	fixed_t l = (SCREEN_WIDTH2  << FIXED_SHIFT) + FIXED_MUL(xz, zoom);// + FIXED_DEC(1,2);
	fixed_t t = (SCREEN_HEIGHT2 << FIXED_SHIFT) + FIXED_MUL(yz, zoom);// + FIXED_DEC(1,2);
	fixed_t r = l + FIXED_MUL(wz, zoom);
	fixed_t b = t + FIXED_MUL(hz, zoom);
	
	l >>= FIXED_SHIFT;
	t >>= FIXED_SHIFT;
	r >>= FIXED_SHIFT;
	b >>= FIXED_SHIFT;
	
	RECT sdst = {
		l,
		t,
		r - l,
		b - t,
	};
	Gfx_DrawTexCol(tex, src, &sdst, cr, cg, cb);
}

void Stage_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom)
{
	Stage_DrawTexCol(tex, src, dst, zoom, 0x80, 0x80, 0x80);
}

void Stage_DrawTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom)
{
	//Don't draw if HUD and HUD is disabled
	#ifdef STAGE_NOHUD
		if (tex == &stage.tex_hud0 || tex == &stage.tex_hud1 || tex == &stage.tex_hudmajin)
			return;
	#endif
	
	//Get screen-space points
	POINT s0 = {SCREEN_WIDTH2 + (FIXED_MUL(p0->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p0->y, zoom) >> FIXED_SHIFT)};
	POINT s1 = {SCREEN_WIDTH2 + (FIXED_MUL(p1->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p1->y, zoom) >> FIXED_SHIFT)};
	POINT s2 = {SCREEN_WIDTH2 + (FIXED_MUL(p2->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p2->y, zoom) >> FIXED_SHIFT)};
	POINT s3 = {SCREEN_WIDTH2 + (FIXED_MUL(p3->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p3->y, zoom) >> FIXED_SHIFT)};
	
	Gfx_DrawTexArb(tex, src, &s0, &s1, &s2, &s3);
}

void Stage_BlendTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom, u8 mode)
{
	//Don't draw if HUD and HUD is disabled
	#ifdef STAGE_NOHUD
		if (tex == &stage.tex_hud0 || tex == &stage.tex_hud1 || tex == &stage.tex_hudmajin)
			return;
	#endif
	
	//Get screen-space points
	POINT s0 = {SCREEN_WIDTH2 + (FIXED_MUL(p0->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p0->y, zoom) >> FIXED_SHIFT)};
	POINT s1 = {SCREEN_WIDTH2 + (FIXED_MUL(p1->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p1->y, zoom) >> FIXED_SHIFT)};
	POINT s2 = {SCREEN_WIDTH2 + (FIXED_MUL(p2->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p2->y, zoom) >> FIXED_SHIFT)};
	POINT s3 = {SCREEN_WIDTH2 + (FIXED_MUL(p3->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p3->y, zoom) >> FIXED_SHIFT)};
	
	Gfx_BlendTexArb(tex, src, &s0, &s1, &s2, &s3, mode);
}

void Stage_DrawTexRotate(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, u8 angle, fixed_t zoom, fixed_t fx, fixed_t fy)
{	
	s16 sin = MUtil_Sin(angle);
	s16 cos = MUtil_Cos(angle);
	int pw = dst->w / 2000;
	int ph = dst->h / 2000;
	
	POINT p0 = {-pw, -ph};
	MUtil_RotatePoint(&p0, sin, cos);
	
	POINT p1 = { pw, -ph};
	MUtil_RotatePoint(&p1, sin, cos);
	
	POINT p2 = {-pw,  ph};
	MUtil_RotatePoint(&p2, sin, cos);
	
	POINT p3 = { pw,  ph};
	MUtil_RotatePoint(&p3, sin, cos);
	
	POINT_FIXED d0 = {
		dst->x + ((fixed_t)p0.x << FIXED_SHIFT) - fx,
		dst->y + ((fixed_t)p0.y << FIXED_SHIFT) - fy
	};
	POINT_FIXED d1 = {
		dst->x + ((fixed_t)p1.x << FIXED_SHIFT) - fx,
		dst->y + ((fixed_t)p1.y << FIXED_SHIFT) - fy
	};
	POINT_FIXED d2 = {
        dst->x + ((fixed_t)p2.x << FIXED_SHIFT) - fx,
		dst->y + ((fixed_t)p2.y << FIXED_SHIFT) - fy
	};
	POINT_FIXED d3 = {
        dst->x + ((fixed_t)p3.x << FIXED_SHIFT) - fx,
		dst->y + ((fixed_t)p3.y << FIXED_SHIFT) - fy
	};
	
    Stage_DrawTexArb(tex, src, &d0, &d1, &d2, &d3, zoom);
}

//Stage HUD functions
static void Stage_DrawHealth(s16 health, u8 i, s8 ox)
{
	u8 status;
	//Check if we should use 'dying' frame
	if (ox < 0)
	{
		if (health >= 18000)
			status = 2;
		else if (health <= 2000)
			status = 1;
		else
			status = 0;
	}
	else
	{
		if (health <= 2000)
			status = 2;
		else if (health >= 18000)
			status = 1;
		else
			status = 0;
	}
	
	//Get src and dst
	fixed_t hx = (105 << FIXED_SHIFT) * (10000 - health) / 10000;
	RECT src = {
		charicon[i][status][0],
		charicon[i][status][1],
		charicon[i][status][2],
		charicon[i][status][3]
	};
	RECT_FIXED dst = {
		hx + ox * FIXED_DEC(20,1) + FIXED_DEC(4,1),
		(SCREEN_HEIGHT2 - 34) << FIXED_SHIFT,
		src.w << FIXED_SHIFT,
		src.h << FIXED_SHIFT
	};
	
	//if (ox < 0)
	//	dst.w = -dst.w;
	
	if (stage.prefs.downscroll)
		dst.y = -dst.y;
	
	//Draw health icon
	Stage_DrawTexRotate(&stage.tex_hud1, &src, &dst, 0, FIXED_MUL(stage.bump, stage.sbump), 0, 0);
}

static void Stage_Player2(void)
{
	//check which mode you choose
	static char* checkoption;

	//change mode to single(only player2 sing)
	if (strcmp(stage.player2sing, "single") == 0 && checkoption != stage.player2sing)
	{
		stage.player_state[0].character->pad_held = 0;
		stage.player_state[0].character = stage.player2;
	}

	//change mode to none (player2 don't sing)
    else if (strcmp(stage.player2sing, "none") == 0 && checkoption != stage.player2sing)
	{
		stage.player_state[0].character->pad_held = 0;
		stage.player_state[0].character = stage.player;
	}

	if (checkoption != stage.player2sing)
		checkoption = stage.player2sing;
}

static void Stage_Opponent2(void)
{
	//check which mode you choose
	static char* checkoption;

	//change mode to single(only opponent2 sing)
	if (strcmp(stage.oppo2sing, "single") == 0 && checkoption != stage.oppo2sing)
	{
		stage.player_state[1].character->pad_held = 0;
		stage.player_state[1].character = stage.opponent2;
	}

	//change mode to none (opponent2 don't sing)
    else if (strcmp(stage.oppo2sing, "none") == 0 && checkoption != stage.oppo2sing)
	{
		stage.player_state[1].character->pad_held = 0;
		 stage.player_state[1].character = stage.opponent;
	}

	if (checkoption != stage.oppo2sing)
		checkoption = stage.oppo2sing;
}

static void Stage_DrawStrum(u8 i, RECT *note_src, RECT_FIXED *note_dst)
{
	(void)note_dst;
	
	PlayerState *this = &stage.player_state[(i & NOTE_FLAG_OPPONENT) != 0];
	i &= 0x3;
	
	if (this->arrow_hitan[i] > 0)
	{
		//Play hit animation
		u8 frame = ((this->arrow_hitan[i] << 1) / stage.step_time) & 1;
		note_src->x = (i + 1) << 5;
		note_src->y = 64 - (frame << 5);
		
		this->arrow_hitan[i] -= timer_dt;
		if (this->arrow_hitan[i] <= 0)
		{
			if (this->pad_held & note_key[i])
				this->arrow_hitan[i] = 1;
			else
				this->arrow_hitan[i] = 0;
		}
	}
	else if (this->arrow_hitan[i] < 0)
	{
		//Play depress animation
		note_src->x = (i + 1) << 5;
		note_src->y = 96;
		if (!(this->pad_held & note_key[i]))
			this->arrow_hitan[i] = 0;
	}
	else
	{
		note_src->x = 0;
		note_src->y = i << 5;
	}
}

static void Stage_DrawNotes(void)
{
	//Check if opponent should draw as bot
	u8 bot = (stage.prefs.mode >= StageMode_2P) ? 0 : NOTE_FLAG_OPPONENT;
	
	//Initialize scroll state
	SectionScroll scroll;
	scroll.start = stage.time_base;
	
	Section *scroll_section = stage.section_base;
	Stage_GetSectionScroll(&scroll, scroll_section);
	
	//Push scroll back until cur_note is properly contained
	while (scroll.start_step > stage.cur_note->pos)
	{
		//Look for previous section
		Section *prev_section = Stage_GetPrevSection(scroll_section);
		if (prev_section == NULL)
			break;
		
		//Push scroll back
		scroll_section = prev_section;
		Stage_GetSectionScroll(&scroll, scroll_section);
		scroll.start -= scroll.length;
	}
	
	//Draw notes
	for (Note *note = stage.cur_note; note->pos != 0xFFFF; note++)
	{
		//Update scroll
		while (note->pos >= scroll_section->end)
		{
			//Push scroll forward
			scroll.start += scroll.length;
			Stage_GetSectionScroll(&scroll, ++scroll_section);
		}
		
		//Get note information
		u8 i = (note->type & NOTE_FLAG_OPPONENT) != 0;
		PlayerState *this = &stage.player_state[i];
		
		fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
		fixed_t time = (scroll.start - stage.song_time) + (scroll.length * (note->pos - scroll.start_step) / scroll.length_step);
		fixed_t y = note_y[(note->type & 0x7) ^ stage.note_swap] + FIXED_MUL(stage.speed, time * 150);
		
		//Check if went above screen
		if (y < FIXED_DEC(-16 - SCREEN_HEIGHT2, 1))
		{
			//Wait for note to exit late time
			if (note_fp + stage.late_safe >= stage.note_scroll)
				continue;
			
			//Miss note if player's note
			if (!(note->type & (bot | NOTE_FLAG_HIT | NOTE_FLAG_MINE)))
			{
				Stage_CutVocal();
				Stage_MissNote(this, note->type);
				NoteMissEvent(note->type,i);
				if (stage.instakill)
					this->health = -0x7000;
				else
					this->health -= 1000;
			}
			
			//Update current note
			stage.cur_note++;
		}
		else
		{
			//Don't draw if below screen
			RECT note_src;
			RECT_FIXED note_dst;
			if (y > (FIXED_DEC(SCREEN_HEIGHT,2) + scroll.size) || note->pos == 0xFFFF)
				break;
			
			//Draw note
			if (note->type & NOTE_FLAG_SUSTAIN)
			{
				//Check for sustain clipping
				fixed_t clip;
				y -= scroll.size;
				if ((note->type & (bot | NOTE_FLAG_HIT)) || ((this->pad_held & note_key[note->type & 0x3]) && (note_fp + stage.late_sus_safe >= stage.note_scroll)))
				{
					clip = note_y[(note->type & 0x7) ^ stage.note_swap] - y;
					if (clip < 0)
						clip = 0;
				}
				else
				{
					clip = 0;
				}
				
				//Draw sustain
				if (note->type & NOTE_FLAG_SUSTAIN_END)
				{
					if (clip < (24 << FIXED_SHIFT))
					{
						note_src.x = 160;
						note_src.y = ((note->type & 0x3) << 5) + 4 + (clip >> FIXED_SHIFT);
						note_src.w = 32;
						note_src.h = 28 - (clip >> FIXED_SHIFT);
						
						note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
						note_dst.y = y + clip;
						note_dst.w = note_src.w << FIXED_SHIFT;
						note_dst.h = (note_src.h << FIXED_SHIFT);
						
						if (stage.prefs.downscroll)
						{
							note_dst.y = -note_dst.y;
							note_dst.h = -note_dst.h;
						}
						
                        if ((stage.stage_id == StageId_Endless) && (stage.song_step >= 900))
                            Stage_DrawTex(&stage.tex_hudmajin, &note_src, &note_dst, stage.bump);
                        else
				            Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
					}
				}
				else
				{
					//Get note height
					fixed_t next_time = (scroll.start - stage.song_time) + (scroll.length * (note->pos + 12 - scroll.start_step) / scroll.length_step);
					fixed_t next_y = note_y[(note->type & 0x7) ^ stage.note_swap] + FIXED_MUL(stage.speed, next_time * 150) - scroll.size;
					fixed_t next_size = next_y - y;
					
					if (clip < next_size)
					{
						note_src.x = 160;
						note_src.y = ((note->type & 0x3) << 5);
						note_src.w = 32;
						note_src.h = 16;
						
						note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
						note_dst.y = y + clip;
						note_dst.w = note_src.w << FIXED_SHIFT;
						note_dst.h = (next_y - y) - clip;
						
						if (stage.prefs.downscroll)
							note_dst.y = -note_dst.y - note_dst.h;
						
                        if ((stage.stage_id == StageId_Endless) && (stage.song_step >= 900))
                            Stage_DrawTex(&stage.tex_hudmajin, &note_src, &note_dst, stage.bump);
                        else
				            Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
					}
				}
			}
			else if (note->type & NOTE_FLAG_MINE)
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note body
				note_src.x = 192 + ((note->type & 0x1) << 5);
				note_src.y = (note->type & 0x2) << 4;
				note_src.w = 32;
				note_src.h = 32;
				
				note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
				note_dst.y = y - FIXED_DEC(16,1);
				note_dst.w = note_src.w << FIXED_SHIFT;
				note_dst.h = note_src.h << FIXED_SHIFT;
				
				if (stage.prefs.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
                
                if ((stage.stage_id == StageId_Endless) && (stage.song_step >= 900))
                    Stage_DrawTex(&stage.tex_hudmajin, &note_src, &note_dst, stage.bump);
                else
				    Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				
				//if (stage.stage_id == StageId_Clwn_4)
				if (false)
				{
					//Draw note halo
					note_src.x = 160;
					note_src.y = 128 + ((animf_count & 0x3) << 3);
					note_src.w = 32;
					note_src.h = 8;
					
					note_dst.y -= FIXED_DEC(6,1);
					note_dst.h >>= 2;
					
                    if ((stage.stage_id == StageId_Endless) && (stage.song_step >= 900))
                        Stage_DrawTex(&stage.tex_hudmajin, &note_src, &note_dst, stage.bump);
                    else
				        Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				}
				else
				{
					//Draw note fire
					note_src.x = 192 + ((animf_count & 0x1) << 5);
					note_src.y = 64 + ((animf_count & 0x2) * 24);
					note_src.w = 32;
					note_src.h = 48;
					
					if (stage.prefs.downscroll)
					{
						note_dst.y += note_dst.h;
						note_dst.h = note_dst.h * -3 / 2;
					}
					else
					{
						note_dst.h = note_dst.h * 3 / 2;
					}
					
                    if ((stage.stage_id == StageId_Endless) && (stage.song_step >= 900))
                        Stage_DrawTex(&stage.tex_hudmajin, &note_src, &note_dst, stage.bump);
                    else
				        Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				}
			}
			else if (note->type & NOTE_FLAG_STATIC)
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note body
				note_src.x = 0 + ((note->type & 0x1) << 5);
				note_src.y = (note->type & 0x2) << 4;
				note_src.w = 32;
				note_src.h = 32;
				
				note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
				note_dst.y = y - FIXED_DEC(16,1);
				note_dst.w = note_src.w << FIXED_SHIFT;
				note_dst.h = note_src.h << FIXED_SHIFT;
				
				if (stage.prefs.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				Stage_DrawTex(&stage.tex_note, &note_src, &note_dst, stage.bump);
			}
			else
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note
				note_src.x = 32 + ((note->type & 0x3) << 5);
				note_src.y = 0;
				note_src.w = 32;
				note_src.h = 32;
				
				note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
				note_dst.y = y - FIXED_DEC(16,1);
				note_dst.w = note_src.w << FIXED_SHIFT;
				note_dst.h = note_src.h << FIXED_SHIFT;
				
				if (stage.prefs.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				
                if ((stage.stage_id == StageId_Endless) && (stage.song_step >= 900))
                    Stage_DrawTex(&stage.tex_hudmajin, &note_src, &note_dst, stage.bump);
                else
				    Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			}
		}
	}
}

//Stage loads
static void Stage_SwapChars(void)
{
	if (stage.prefs.mode == StageMode_Swap)
	{
		Character *temp = stage.player;
		stage.player = stage.opponent;
		stage.opponent = temp;
	}
}

static void Stage_LoadPlayer(void)
{
	//Load player character
	Character_Free(stage.player);
	stage.player = stage.stage_def->pchar.new(stage.stage_def->pchar.x, stage.stage_def->pchar.y);
}

static void Stage_LoadPlayer2(void)
{
	//Load player character
	Character_Free(stage.player2);
	if (stage.stage_def->pchar2.new != NULL) {
		stage.player2 = stage.stage_def->pchar2.new(stage.stage_def->pchar2.x, stage.stage_def->pchar2.y);
	}
	else
		stage.player2 = NULL;
}

static void Stage_LoadOpponent(void)
{
	//Load opponent character
	Character_Free(stage.opponent);
	stage.opponent = stage.stage_def->ochar.new(stage.stage_def->ochar.x, stage.stage_def->ochar.y);
}

static void Stage_LoadOpponent2(void)
{
	//Load opponent character
	Character_Free(stage.opponent2);
	if (stage.stage_def->ochar2.new != NULL) {
		stage.opponent2 = stage.stage_def->ochar2.new(stage.stage_def->ochar2.x, stage.stage_def->ochar2.y);
	}
	else
		stage.opponent2 = NULL;
}

static void Stage_LoadGirlfriend(void)
{
	//Load girlfriend character
	Character_Free(stage.gf);
	if (stage.stage_def->gchar.new != NULL)
		stage.gf = stage.stage_def->gchar.new(stage.stage_def->gchar.x, stage.stage_def->gchar.y);
	else
		stage.gf = NULL;
}

static void Stage_LoadStage(void)
{
	//Load back
	if (stage.back != NULL)
		stage.back->free(stage.back);
	stage.back = stage.stage_def->back();
}

static void Stage_LoadChart(void)
{
	//Load stage data
	char chart_path[64];
	if (stage.stage_def->week & 0x80)
	{
		//Use custom path convention
		sprintf(chart_path, "\\SONGS\\CUSTOM.%d.CHT;1", stage.stage_def->week_song);
	}
	else
	{
		//Use standard path convention
		sprintf(chart_path, "\\SONGS\\%d.%d.CHT;1", stage.stage_def->week, stage.stage_def->week_song);
	}
	
	if (stage.chart_data != NULL)
		Mem_Free(stage.chart_data);
	stage.chart_data = IO_Read(chart_path);
	u8 *chart_byte = (u8*)stage.chart_data;
	
	//Directly use section and notes pointers
	stage.sections = (Section*)(chart_byte + 6);
	stage.notes = (Note*)(chart_byte + ((u16*)stage.chart_data)[2]);
		
	for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
		stage.num_notes++;
	
	//Swap chart
	if (stage.prefs.mode == StageMode_Swap)
	{
		for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
			note->type ^= NOTE_FLAG_OPPONENT;
		for (Section *section = stage.sections;; section++)
		{
			section->flag ^= SECTION_FLAG_OPPFOCUS;
			if (section->end == 0xFFFF)
				break;
		}
	}
	
	//Count max scores
	stage.player_state[0].max_score = 0;
	stage.player_state[1].max_score = 0;
	for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
	{
		if (note->type & (NOTE_FLAG_SUSTAIN | NOTE_FLAG_MINE))
			continue;
		if (note->type & NOTE_FLAG_OPPONENT)
			stage.player_state[1].max_score += 35;
		else
			stage.player_state[0].max_score += 35;
	}
	if (stage.prefs.mode >= StageMode_2P && stage.player_state[1].max_score > stage.player_state[0].max_score)
		stage.max_score = stage.player_state[1].max_score;
	else
		stage.max_score = stage.player_state[0].max_score;
	
	stage.cur_section = stage.sections;
	stage.cur_note = stage.notes;
	
	stage.speed = *((fixed_t*)stage.chart_data);
	strcpy(stage.credits, stage.stage_def->credits);
	
	stage.step_crochet = 0;
	stage.time_base = 0;
	stage.step_base = 0;
	stage.section_base = stage.cur_section;
	Stage_ChangeBPM(stage.cur_section->flag & SECTION_FLAG_BPM_MASK, 0);
}

static void Stage_LoadMusic(void)
{
	//Offset sing ends
	stage.player->sing_end -= stage.note_scroll;
	if (stage.player2 != NULL)
		stage.player2->sing_end -= stage.note_scroll;
	stage.opponent->sing_end -= stage.note_scroll;
	if (stage.opponent2 != NULL)
		stage.opponent2->sing_end -= stage.note_scroll;
	if (stage.gf != NULL)
		stage.gf->sing_end -= stage.note_scroll;
	
	//Find music file and begin seeking to it
	Audio_SeekXA_Track(stage.stage_def->music_track);
	
	//Initialize music state
	stage.note_scroll = FIXED_DEC(-5 * 6 * 12,1);
	stage.song_time = FIXED_DIV(stage.note_scroll, stage.step_crochet);
	stage.interp_time = 0;
	stage.interp_ms = 0;
	stage.interp_speed = 0;
	
	//Offset sing ends again
	stage.player->sing_end += stage.note_scroll;
	if (stage.player2 != NULL)
		stage.player2->sing_end -= stage.note_scroll;
	stage.opponent->sing_end += stage.note_scroll;
	if (stage.opponent2 != NULL)
		stage.opponent2->sing_end += stage.note_scroll;
	if (stage.gf != NULL)
		stage.gf->sing_end += stage.note_scroll;
}

static void Stage_LoadState(void)
{
	//Initialize stage state
	stage.flag = STAGE_FLAG_VOCAL_ACTIVE;
	
	stage.gf_speed = 1 << 2;
	
	stage.state = StageState_Play;
	
	stage.player_state[0].character = stage.player;
	stage.player_state[1].character = stage.opponent;
	
	for (int i = 0; i < 2; i++)
	{
		memset(stage.player_state[i].arrow_hitan, 0, sizeof(stage.player_state[i].arrow_hitan));
		
		stage.player_state[i].health = 10000;
		stage.player_state[i].combo = 0;
		stage.oppo2sing = "none";
		stage.player2sing = "none";
		stage.player_state[i].refresh_score = false;
		stage.player_state[i].refresh_miss = false;
		stage.player_state[i].refresh_accuracy = false;
		stage.player_state[i].score = 0;
		stage.player_state[i].miss = 0;
		stage.player_state[i].accuracy = 0;
		stage.player_state[i].max_accuracy = 0;
		stage.player_state[i].min_accuracy = 0;
		strcpy(stage.player_state[i].score_text, "Score: 0 | Misses: 0 | Accuracy: 0%");
		strcpy(stage.player_state[i].P2_text, "SC: 0 | MS: 0 | AC: 0%");
		
		stage.player_state[i].pad_held = stage.player_state[i].pad_press = 0;
	}
	
	ObjectList_Free(&stage.objlist_splash);
	ObjectList_Free(&stage.objlist_fg);
	ObjectList_Free(&stage.objlist_bg);
}

//Stage functions
void Stage_Load(StageId id, StageDiff difficulty, boolean story)
{
	//Get stage definition
	stage.stage_def = &stage_defs[stage.stage_id = id];
	stage.stage_diff = difficulty;
	stage.story = story;
	
	//Manage Notes Position
	s16 note_def[3][8] = {
		{-80,-80,-80,-80,-80,-80,-80,-80}, //Notes Y Position
		{26,60,94,128,-128,-94,-60,-26}, //Notes X Position
		{-51,-17,17,51,-128,-94,94,128} //Notes X Position with Middle Scroll
	};
	for (int i = 0; i < 8; i++)
	{
		note_x[i] = FIXED_DEC(note_def[(stage.prefs.middlescroll) ? 2 : 1][i],1);
		note_y[i] = FIXED_DEC(note_def[0][i],1);
	}

	//Load HUD textures
	Gfx_LoadTex(&stage.tex_hud0, IO_Read("\\STAGE\\HUD0.TIM;1"), GFX_LOADTEX_FREE);
    if (stage.stage_id == StageId_Endless)
        Gfx_LoadTex(&stage.tex_hudmajin, IO_Read("\\STAGE\\HUDMAJIN.TIM;1"), GFX_LOADTEX_FREE);
	Gfx_LoadTex(&stage.tex_hud1, IO_Read("\\STAGE\\HUD1.TIM;1"), GFX_LOADTEX_FREE);
	
	//Load Special Notes
	Gfx_LoadTex(&stage.tex_note, IO_Read("\\STAGE\\NOTE.TIM;1"), GFX_LOADTEX_FREE);
	
	//Load Start Screen Textures
	if (stage.stage_def->tim)
	{
		stage.startscreen = 512;
		IO_Data strscr_arc = IO_Read("\\STAGE\\STRSCR.ARC;1");
		Gfx_LoadTex(&stage.tex_strscr, Archive_Find(strscr_arc, stage.stage_def->timfile), 0);
		Mem_Free(strscr_arc);
	}
	
	//Load Events
	LoadEvents();
	
	//Load stage background
	Stage_LoadStage();
	
	//Load characters
	Stage_LoadPlayer();
	Stage_LoadPlayer2();
	Stage_LoadOpponent();
	Stage_LoadOpponent2();
	Stage_LoadGirlfriend();
	stage.hidegf = false;
	Stage_SwapChars();
	
	//Load stage chart
	Stage_LoadChart();
	
	//Initialize stage state
	stage.story = story;
	
	Stage_LoadState();
	
	//Initialize camera
	if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
		Stage_FocusCharacter(stage.opponent, FIXED_UNIT);
	else
		Stage_FocusCharacter(stage.player, FIXED_UNIT);
	stage.camera.x = stage.camera.tx;
	stage.camera.y = stage.camera.ty;
	stage.camera.zoom = stage.camera.tz;
	stage.camera.force = false;
	
	stage.bump = FIXED_UNIT;
	stage.sbump = FIXED_UNIT;
	
	//Initialize stage according to mode
	stage.note_swap = (stage.prefs.mode == StageMode_Swap) ? 4 : 0;
	
	//Load VAG files
	Audio_ClearAlloc();
	CdlFILE file;

	IO_FindFile(&file, "\\SOUNDS\\MICDROP.VAG;1");
   	u32 *data = IO_ReadFile(&file);
    stage.sound[0] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);
	
	IO_FindFile(&file, "\\SOUNDS\\HITSTA1.VAG;1");
   	data = IO_ReadFile(&file);
    stage.sound[1] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);
	
	IO_FindFile(&file, "\\SOUNDS\\GAMOEND.VAG;1");
	data = IO_ReadFile(&file);
	stage.sound[2] = Audio_LoadVAGData(data, file.size);
	Mem_Free(data);
	
	//load fonts
	FontData_Load(&stage.font_cdr, Font_CDR);
	
	//Load music
	stage.note_scroll = 0;
	Stage_LoadMusic();
	
	//Test offset
	stage.offset = 0;
	Gfx_SetClear(0, 0, 0);
}

void Stage_Unload(void)
{
	//Unload stage background
	if (stage.back != NULL)
		stage.back->free(stage.back);
	stage.back = NULL;
	
	//Unload stage data
	Mem_Free(stage.chart_data);
	stage.chart_data = NULL;
	
	//Free objects
	ObjectList_Free(&stage.objlist_splash);
	ObjectList_Free(&stage.objlist_fg);
	ObjectList_Free(&stage.objlist_bg);
	
	//Free characters
	Character_Free(stage.player);
	stage.player = NULL;
	Character_Free(stage.player2);
	stage.player2 = NULL;
	Character_Free(stage.opponent);
	stage.opponent = NULL;
	Character_Free(stage.opponent2);
	stage.opponent2 = NULL;
	Character_Free(stage.gf);
	stage.gf = NULL;
}

static boolean Stage_NextLoad(void)
{
	u8 load = stage.stage_def->next_load;
	if (load == 0)
	{
		//Do stage transition if full reload
		stage.trans = StageTrans_NextSong;
		Trans_Start();
		return false;
	}
	else
	{
		//Get stage definition
		stage.stage_def = &stage_defs[stage.stage_id = stage.stage_def->next_stage];
		
		//Load stage background
		if (load & STAGE_LOAD_STAGE)
			Stage_LoadStage();
		
		//Load characters
		Stage_SwapChars();
		if (load & STAGE_LOAD_PLAYER)
		{
			Stage_LoadPlayer();
		}
		else
		{
			stage.player->x = stage.stage_def->pchar.x;
			stage.player->y = stage.stage_def->pchar.y;
		}
		if (load & STAGE_LOAD_PLAYER2)
		{
			Stage_LoadPlayer2();
		}
		else if (stage.player2 != NULL)
		{
			stage.player2->x = stage.stage_def->pchar2.x;
			stage.player2->y = stage.stage_def->pchar2.y;
		}
		if (load & STAGE_LOAD_OPPONENT)
		{
			Stage_LoadOpponent();
		}
		else if (stage.opponent2 != NULL)
		{
			stage.opponent->x = stage.stage_def->ochar.x;
			stage.opponent->y = stage.stage_def->ochar.y;
		}
		if (load & STAGE_LOAD_OPPONENT2)
		{
			Stage_LoadOpponent2();
		}
		else if (stage.opponent2 != NULL)
		{
			stage.opponent2->x = stage.stage_def->ochar2.x;
			stage.opponent2->y = stage.stage_def->ochar2.y;
		}
		Stage_SwapChars();
		if (load & STAGE_LOAD_GIRLFRIEND)
		{
			Stage_LoadGirlfriend();
		}
		else if (stage.gf != NULL)
		{
			stage.gf->x = stage.stage_def->gchar.x;
			stage.gf->y = stage.stage_def->gchar.y;
		}
		
		//Load stage chart
		Stage_LoadChart();
		
		//Initialize stage state
		Stage_LoadState();
		
		//Load music
		Stage_LoadMusic();
		
		//Reset timer
		Timer_Reset();
		return true;
	}
}

fixed_t song_time2;

void Stage_Tick(void)
{
	SeamLoad:;
	
	//Tick transition
	if (stage.state == StageState_Play)
	{
		//Return to menu when start is pressed
		if (pad_state.press & PAD_START)
		{
			stage.trans = StageTrans_Menu;
			Trans_Start();
		}
	}
	else if(stage.state != StageState_Continued)
	{
		//Return to menu when start is pressed
		if (pad_state.press & PAD_START)
		{
			stage.state = StageState_Continued;
			Audio_PlaySound(stage.sound[2], 0x3fff);
			stage.continflash = 252;
			Audio_StopXA();
		}
	}
	
	if (Trans_Tick())
	{
		switch (stage.trans)
		{
			case StageTrans_Menu:
				//Load appropriate menu
				Stage_Unload();
				
				LoadScr_Start();
				if (stage.stage_id <= StageId_LastVanilla)
				{
					if (stage.story)
						Menu_Load(MenuPage_Story);
					else
						Menu_Load(MenuPage_Freeplay);
				}
				else
				{
					//Menu_Load(MenuPage_Mods); //this changed for now
					Menu_Load(MenuPage_Freeplay);
				}
				LoadScr_End();
				
				gameloop = GameLoop_Menu;
				return;
			case StageTrans_NextSong:
				//Load next song
				Stage_Unload();
				
				LoadScr_Start();
				Stage_Load(stage.stage_def->next_stage, stage.stage_diff, stage.story);
				LoadScr_End();
				break;
			case StageTrans_Reload:
				//Reload song
				Stage_Unload();
				
				LoadScr_Start();
				Stage_Load(stage.stage_id, stage.stage_diff, stage.story);
				LoadScr_End();
				break;
		}
	}
	switch (stage.state)
	{
		case StageState_Play:
		{
			if (stage.startscreen > 0)
				Stage_DrawStartScreen();
			Events();
			stage.font_cdr.draw(&stage.font_cdr,
				stage.credits,
				FIXED_DEC(-159,1),
				FIXED_DEC(108,1),
				FontAlign_Left
			);
			
			Stage_Player2();
			Stage_Opponent2();
			
			if (stage.prefs.botplay)
			{
				//Draw botplay
				RECT bot_src = {140, 224, 67, 16};
				RECT_FIXED bot_dst = {FIXED_DEC(-bot_src.w / 2,1), FIXED_DEC(-58,1), FIXED_DEC(bot_src.w,1), FIXED_DEC(bot_src.h,1)};

                if ((stage.stage_id == StageId_Endless) && (stage.song_step >= 900))
                    Stage_DrawTex(&stage.tex_hudmajin, &bot_src, &bot_dst, stage.bump);
                else
				    Stage_DrawTex(&stage.tex_hud0, &bot_src, &bot_dst, stage.bump);
			}
			
			//Clear per-frame flags
			stage.flag &= ~(STAGE_FLAG_JUST_STEP | STAGE_FLAG_SCORE_REFRESH);
			
			//Get song position
			boolean playing;
			fixed_t next_scroll;
			
			if (stage.note_scroll < 0)
			{
				//Play countdown sequence
				stage.song_time += timer_dt;
					
				//Update song
				if (stage.song_time >= 0)
				{
					//Song has started
					playing = true;

					Audio_PlayXA_Track(stage.stage_def->music_track, 0x40, stage.stage_def->music_channel, 0);
						
					//Update song time
					fixed_t audio_time = (fixed_t)Audio_TellXA_Milli() - stage.offset;
					if (audio_time < 0)
						audio_time = 0;
					stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
					stage.interp_time = 0;
					stage.song_time = stage.interp_ms;
				}
				else
				{
					//Still scrolling
					playing = false;
				}
				
				//Update scroll
				next_scroll = FIXED_MUL(stage.song_time, stage.step_crochet);
			}
			else if (Audio_PlayingXA())
			{
				fixed_t audio_time_pof = (fixed_t)Audio_TellXA_Milli();
				fixed_t audio_time = (audio_time_pof > 0) ? (audio_time_pof - stage.offset) : 0;
				
				//Old sync
				stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
				stage.interp_time = 0;
				stage.song_time = stage.interp_ms;
				
				playing = true;
				
				//Update scroll
				next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
			}
			else
			{
				//Song has ended
				playing = false;
				stage.song_time += timer_dt;
					
				//Update scroll
				next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
				
				//Transition to menu or next song
				if (stage.story && stage.stage_def->next_stage != stage.stage_id)
				{
					if (Stage_NextLoad())
						goto SeamLoad;
				}
				else
				{
					stage.trans = StageTrans_Menu;
					Trans_Start();
				}
			}	

			RecalcScroll:;
			//Update song scroll and step
			if (next_scroll > stage.note_scroll)
			{
				if (((stage.note_scroll / 12) & FIXED_UAND) != ((next_scroll / 12) & FIXED_UAND))
					stage.flag |= STAGE_FLAG_JUST_STEP;
				stage.note_scroll = next_scroll;
				stage.song_step = (stage.note_scroll >> FIXED_SHIFT);
				if (stage.note_scroll < 0)
					stage.song_step -= 11;
				stage.song_step /= 12;
			}
			
			//Update section
			if (stage.note_scroll >= 0)
			{
				//Check if current section has ended
				u16 end = stage.cur_section->end;
				if ((stage.note_scroll >> FIXED_SHIFT) >= end)
				{
					//Increment section pointer
					stage.cur_section++;
					
					//Update BPM
					u16 next_bpm = stage.cur_section->flag & SECTION_FLAG_BPM_MASK;
					Stage_ChangeBPM(next_bpm, end);
					stage.section_base = stage.cur_section;
					
					//Recalculate scroll based off new BPM
					next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
					goto RecalcScroll;
				}
			}
			
			//Handle bump
			if ((stage.bump = FIXED_UNIT + FIXED_MUL(stage.bump - FIXED_UNIT, FIXED_DEC(95,100))) <= FIXED_DEC(1003,1000))
				stage.bump = FIXED_UNIT;
			stage.sbump = FIXED_UNIT + FIXED_MUL(stage.sbump - FIXED_UNIT, FIXED_DEC(90,100));
			
			if (playing && (stage.flag & STAGE_FLAG_JUST_STEP))
			{
				//Check if screen should bump
				boolean is_bump_step = (stage.song_step & 0xF) == 0;
				
				//Bump screen
				if (is_bump_step)
					stage.bump += FIXED_DEC(3,100);
				
				//Bump health every 4 steps
				if ((stage.song_step & 0x3) == 0)
					stage.sbump += FIXED_DEC(8,100);
			}
			
			//Scroll camera
			if (!stage.camera.force)
			{
				if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
					Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
				else
					Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
			}
			Stage_ScrollCamera();
			
			switch (stage.prefs.mode)
			{
				case StageMode_Normal:
				case StageMode_Swap:
				{
					//Handle player 1 inputs
					Stage_ProcessPlayer(&stage.player_state[0], &pad_state, playing);
					
					//Handle opponent notes
					u8 opponent_anote = CharAnim_Idle;
					u8 opponent_snote = CharAnim_Idle;
					
					for (Note *note = stage.cur_note;; note++)
					{
						if (note->pos > (stage.note_scroll >> FIXED_SHIFT))
							break;
						
						//Opponent note hits
						if (playing && (note->type & NOTE_FLAG_OPPONENT) && !(note->type & NOTE_FLAG_HIT))
						{
							//Opponent hits note
							Stage_StartVocal();
							if (note->type & NOTE_FLAG_SUSTAIN)
								opponent_snote = note_anims[note->type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0];
							else
								opponent_anote = note_anims[note->type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0];
							note->type |= NOTE_FLAG_HIT;
						}
					}
					
					if (opponent_anote != CharAnim_Idle)
						stage.opponent->set_anim(stage.opponent, opponent_anote);
					else if (opponent_snote != CharAnim_Idle)
						stage.opponent->set_anim(stage.opponent, opponent_snote);
					break;
				}
				case StageMode_2P:
				{
					//Handle player 1 and 2 inputs
					Stage_ProcessPlayer(&stage.player_state[0], &pad_state, playing);
					Stage_ProcessPlayer(&stage.player_state[1], &pad_state_2, playing);
					break;
				}
			}
			
			//Tick note splashes
			ObjectList_Tick(&stage.objlist_splash);
			
			PlayerState *this;
			s32 VScore;
			
			//Draw Score
			if (stage.prefs.mode >= StageMode_2P)
			{
				for (int i = 0; i < ((stage.prefs.mode >= StageMode_2P) ? 2 : 1); i++)
				{
					this = &stage.player_state[i];
					
					this->accuracy = (this->min_accuracy * 100) / (this->max_accuracy);
					
					if (this->refresh_score || this->refresh_miss || this->refresh_miss)
					{
						VScore = (this->score * stage.max_score / this->max_score) * 10;
						sprintf(this->P2_text, "SC: %d | MS: %d | AC: %d%%", VScore, this->miss, this->accuracy);
						this->refresh_score = false;
						this->refresh_miss = false;
						this->refresh_accuracy = false;
					}
					
					if (i == 0)
						stage.font_cdr.draw(&stage.font_cdr,
							this->P2_text,
							FIXED_DEC(11,1), 
							FIXED_DEC(100,1),
							FontAlign_Left
						);
					else
						stage.font_cdr.draw(&stage.font_cdr,
							this->P2_text,
							FIXED_DEC(-11,1),
							FIXED_DEC(100,1),
							FontAlign_Right
						);
				}
			}
			else
			{
				this = &stage.player_state[0];
				
				this->accuracy = (this->min_accuracy * 100) / (this->max_accuracy);
				
				if (this->refresh_score || this->refresh_miss || this->refresh_miss)
				{
					VScore = (this->score * stage.max_score / this->max_score) * 10;
					sprintf(this->score_text, "Score: %d | Misses: %d | Accuracy: %d%%", VScore, this->miss, this->accuracy);
					this->refresh_score = false;
					this->refresh_miss = false;
					this->refresh_accuracy = false;
				}
				
				s32 texty = 100;
				
				if (stage.prefs.downscroll)
					texty = -70;
				
				stage.font_cdr.draw(&stage.font_cdr,
					this->score_text,
					FIXED_DEC(20,1), 
					FIXED_DEC(texty,1),
					FontAlign_Center
				);
			}
			
			if (stage.prefs.mode < StageMode_2P)
			{
				//Perform health checks
				if (stage.player_state[0].health <= 0)
				{
					//Player has died
					stage.player_state[0].health = 0;
					stage.state = StageState_Dead;
				}
				if (stage.player_state[0].health > 20000)
					stage.player_state[0].health = 20000;
				
				if (!stage.instakill)
				{
					//Draw health heads
					Stage_DrawHealth(stage.player_state[0].health, stage.player->health_i, 1);
					Stage_DrawHealth(stage.player_state[0].health, stage.opponent->health_i, -1);
					
					//Draw health bar
					RECT health_border_src = {0, 0, 210, 8};
					RECT_FIXED health_border_dst = {FIXED_DEC(-100,1), (SCREEN_HEIGHT2 - 36) << FIXED_SHIFT, FIXED_DEC(210,1), FIXED_DEC(8,1)};
				
					if (stage.prefs.downscroll)
						health_border_dst.y = -health_border_dst.y;
				
					Stage_DrawTex(&stage.tex_hud1, &health_border_src, &health_border_dst, stage.bump);
					
					
					RECT health_color_src = {210, 0, 1, 1};
					RECT_FIXED health_color_dst = {FIXED_DEC(109 - (208 * stage.player_state[0].health / 20000),1), (SCREEN_HEIGHT2 - 36) << FIXED_SHIFT, FIXED_DEC(0 + (208 * stage.player_state[0].health / 20000),1), FIXED_DEC(8,1)};
					
					if (stage.prefs.downscroll)
						health_color_dst.y = -health_color_dst.y;
				
					Stage_DrawTex(&stage.tex_hud1, &health_color_src, &health_color_dst, stage.bump);
					
					
					RECT health_back_src = {210, 2, 1, 1};
					RECT_FIXED health_back_dst = {FIXED_DEC(-99,1), (SCREEN_HEIGHT2 - 36) << FIXED_SHIFT, FIXED_DEC(208,1), FIXED_DEC(8,1)};
					
					if (stage.prefs.downscroll)
						health_back_dst.y = -health_back_dst.y;
					
					Stage_DrawTex(&stage.tex_hud1, &health_back_src, &health_back_dst, stage.bump);
				}
			}
			
			//Draw stage notes
			Stage_DrawNotes();
			
			//Draw note HUD
			RECT note_src = {0, 0, 32, 32};
			RECT_FIXED note_dst = {0, 0, FIXED_DEC(32,1), FIXED_DEC(32,1)};
			
			for (u8 i = 0; i < 4; i++)
			{
				//BF
				note_dst.x = note_x[i ^ stage.note_swap] - FIXED_DEC(16,1);
				note_dst.y = note_y[i ^ stage.note_swap] - FIXED_DEC(16,1);
				if (stage.prefs.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				
				Stage_DrawStrum(i, &note_src, &note_dst);
                if ((stage.stage_id == StageId_Endless) && (stage.song_step >= 900))
                    Stage_DrawTex(&stage.tex_hudmajin, &note_src, &note_dst, stage.bump);
                else
				    Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				
				//Opponent
				note_dst.x = note_x[(i | 0x4) ^ stage.note_swap] - FIXED_DEC(16,1);
				note_dst.y = note_y[(i | 0x4) ^ stage.note_swap] - FIXED_DEC(16,1);
				
				if (stage.prefs.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				Stage_DrawStrum(i | 4, &note_src, &note_dst);
				
                if ((stage.stage_id == StageId_Endless) && (stage.song_step >= 900))
                    Stage_DrawTex(&stage.tex_hudmajin, &note_src, &note_dst, stage.bump);
                else
				    Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			}
			
			//Draw stage foreground
			if (stage.back->draw_fg != NULL)
				stage.back->draw_fg(stage.back);
			
			//Tick foreground objects
			ObjectList_Tick(&stage.objlist_fg);
			
			//Tick characters
			stage.player->tick(stage.player);
			stage.opponent->tick(stage.opponent);
			if (stage.player2 != NULL)
				stage.player2->tick(stage.player2);
			if (stage.opponent2 != NULL)
				stage.opponent2->tick(stage.opponent2);
			
			//Draw stage middle
			if (stage.back->draw_md != NULL)
				stage.back->draw_md(stage.back);
			
			//Tick girlfriend
			if (!stage.hidegf)
				if (stage.gf != NULL )
					stage.gf->tick(stage.gf);
			
			//static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
			//Gfx_BlendRect(&flash, stage.camera.zoom / 2, 0, stage.camera.zoom / 2, 1);
			
			//Tick background objects
			ObjectList_Tick(&stage.objlist_bg);
			
			//Draw stage background
			if (stage.back->draw_bg != NULL)
				stage.back->draw_bg(stage.back);
			break;
		}
		case StageState_Dead: //Start BREAK animation and reading extra data from CD
		{
			//Stop music immediately
			Audio_StopXA();
			Audio_PlaySound(stage.sound[0], 0x3fff);
			
			//Unload stage data
			Mem_Free(stage.chart_data);
			stage.chart_data = NULL;
			
			//Free background
			stage.back->free(stage.back);
			stage.back = NULL;
			
			//Free objects
			ObjectList_Free(&stage.objlist_fg);
			ObjectList_Free(&stage.objlist_bg);
			
			//Free opponent and girlfriend
			Stage_SwapChars();
			Character_Free(stage.player2);
			stage.player2 = NULL;
			Character_Free(stage.opponent);
			stage.opponent = NULL;
			Character_Free(stage.opponent2);
			stage.opponent2 = NULL;
			Character_Free(stage.gf);
			stage.gf = NULL;
			
			//Reset stage state
			stage.flag = 0;
			stage.bump = stage.sbump = FIXED_UNIT;
			
			//Change background colour to black
			Gfx_SetClear(0, 0, 0);
			
			//Run death animation, focus on player, and change state
			stage.player->set_anim(stage.player, PlayerAnim_Dead0);
			
			Stage_FocusCharacter(stage.player, 0);
			stage.song_time = 0;
			
			stage.state = StageState_DeadLoad;
		}
	//Fallthrough
		case StageState_DeadLoad:
		{
			stage.player->tick(stage.player);
			
			//Scroll camera and tick player
			if (stage.song_time < FIXED_UNIT)
				stage.song_time += FIXED_UNIT / 60;
			stage.camera.td = FIXED_DEC(-2, 100) + FIXED_MUL(stage.song_time, FIXED_DEC(45, 1000));
			if (stage.camera.td > 0)
				Stage_ScrollCamera();
			stage.player->tick(stage.player);
			
			//Drop mic and change state if CD has finished reading and animation has ended
			//if (IO_IsReading() || stage.player->animatable.anim != PlayerAnim_Dead1)
			
			stage.camera.td = FIXED_DEC(25, 1000);
			stage.state = StageState_DeadDrop;
			break;
		}
		case StageState_DeadDrop:
		{
			//Scroll camera and tick player
			Stage_ScrollCamera();
			stage.player->tick(stage.player);
			
			//Enter next state once mic has been dropped
			if (stage.player->animatable.anim == PlayerAnim_Dead1)
				stage.player->set_anim(stage.player, PlayerAnim_Dead2);
			//Enter next state once mic has been dropped
			if (stage.player->animatable.anim == PlayerAnim_Dead3)
			{
				stage.state = StageState_DeadRetry;
				Audio_PlayXA_Track(XA_GameOver, 0x40, 1, true);
			}
			break;
		}
		case StageState_DeadRetry:
		{
			//Randomly twitch
			if (stage.player->animatable.anim == PlayerAnim_Dead3)
			{
				if (RandomRange(0, 29) == 0)
					stage.player->set_anim(stage.player, PlayerAnim_Dead4);
				if (RandomRange(0, 29) == 0)
					stage.player->set_anim(stage.player, PlayerAnim_Dead5);
			}
			
			//Scroll camera and tick player
			Stage_ScrollCamera();
			stage.player->tick(stage.player);
			break;
		}
		case StageState_Continued:
		{
			if(stage.continflash > 0)
			{
				stage.continflash -= 4;
				Gfx_SetClear(stage.continflash, 0, 0);
			}
			else if (Trans_Idle())
			{
				stage.trans = StageTrans_Reload;
				Trans_Start();
				Gfx_SetClear(0, 0, 0);
			}
			
			break;
		}
		default:
			break;
	}
}