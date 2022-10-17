#include "gfn.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//GFN character structure
enum
{
	GFN_ArcMain_GFN0,
	GFN_ArcMain_GFN1,
	GFN_ArcMain_GFN2,
	GFN_ArcMain_GFN3,
	GFN_ArcMain_GFN4,
	GFN_ArcMain_GFN5,

	GFN_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_scene;
	IO_Data arc_ptr[GFN_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Pico test
	u16 *pico_p;
} Char_GFN;

//GFN character definitions
static const CharFrame char_gfn_frame[] = {
	{GFN_ArcMain_GFN0, {  0,  0,109,139}, { 37, 72}}, //0 bop left 1
	{GFN_ArcMain_GFN0, {110,  0,109,139}, { 37, 72}}, //1 bop left 2
	{GFN_ArcMain_GFN1, {  0,  0,109,139}, { 37, 72}}, //2 bop left 3
	{GFN_ArcMain_GFN1, {110,  0,109,139}, { 37, 72}}, //3 bop left 4
	{GFN_ArcMain_GFN2, {  0,  0,109,139}, { 37, 72}}, //4 bop left 5
	{GFN_ArcMain_GFN2, {110,  0,109,139}, { 37, 72}}, //5 bop left 6
	
	{GFN_ArcMain_GFN3, {  0,  0,109,139}, { 37, 72}}, //6 bop right 1
	{GFN_ArcMain_GFN3, {110,  0,109,139}, { 37, 72}}, //7 bop right 2
	{GFN_ArcMain_GFN4, {  0,  0,109,139}, { 37, 72}}, //8 bop right 3
	{GFN_ArcMain_GFN4, {110,  0,109,139}, { 37, 72}}, //9 bop right 4
	{GFN_ArcMain_GFN5, {  0,  0,109,139}, { 37, 72}}, //10 bop right 5
	{GFN_ArcMain_GFN5, {110,  0,109,139}, { 37, 72}}, //11 bop right 6
};

static const Animation char_gfn_anim[CharAnim_Max] = {
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                           //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                           //CharAnim_Left
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  4,  4,  5, ASCR_BACK, 1}}, //CharAnim_LeftAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                           //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                           //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                           //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                           //CharAnim_UpAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                           //CharAnim_Right
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9, 10, 10, 11, ASCR_BACK, 1}}, //CharAnim_RightAlt
};

//GFN character functions
void Char_GFN_SetFrame(void *user, u8 frame)
{
	Char_GFN *this = (Char_GFN*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_gfn_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_GFN_Tick(Character *character)
{
	Char_GFN *this = (Char_GFN*)character;
	
	if (this->pico_p != NULL)
	{
		if (stage.note_scroll >= 0)
		{
			//Scroll through Pico chart
			u16 substep = stage.note_scroll >> FIXED_SHIFT;
			while (substep >= ((*this->pico_p) & 0x7FFF))
			{
				//Play animation
				character->set_anim(character, ((*this->pico_p) & 0x8000) ? CharAnim_RightAlt : CharAnim_LeftAlt);
				this->pico_p++;
			}
		}
	}
	else
	{
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			//Stage specific animations
			if (stage.note_scroll >= 0)
			{
				//switch (stage.stage_id)
				//{
				//	case StageId_1_4: //Tutorial cheer
				//		if (stage.song_step > 64 && stage.song_step < 192 && (stage.song_step & 0x3F) == 60)
				//			character->set_anim(character, CharAnim_UpAlt);
				//		break;
				//	default:
				//		break;
				//}
			}
			
			//Perform dance
			if (stage.note_scroll >= character->sing_end && (stage.song_step % stage.gf_speed) == 0)
			{
				//Switch animation
				if (character->animatable.anim == CharAnim_LeftAlt || character->animatable.anim == CharAnim_Right)
					character->set_anim(character, CharAnim_RightAlt);
				else
					character->set_anim(character, CharAnim_LeftAlt);
			}
		}
	}
	
	//Get parallax
	fixed_t parallax;
	parallax = FIXED_UNIT;
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_GFN_SetFrame);
	Character_DrawParallax(character, &this->tex, &char_gfn_frame[this->frame], parallax);
}

void Char_GFN_SetAnim(Character *character, u8 anim)
{
	//Set animation
	if (anim == CharAnim_Left || anim == CharAnim_Down || anim == CharAnim_Up || anim == CharAnim_Right || anim == CharAnim_UpAlt)
		character->sing_end = stage.note_scroll + FIXED_DEC(22,1); //Nearly 2 steps
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_GFN_Free(Character *character)
{
	Char_GFN *this = (Char_GFN*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_scene);
}

Character *Char_GFN_New(fixed_t x, fixed_t y)
{
	//Allocate gfn object
	Char_GFN *this = Mem_Alloc(sizeof(Char_GFN));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_GFN_New] Failed to allocate gfn object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_GFN_Tick;
	this->character.set_anim = Char_GFN_SetAnim;
	this->character.free = Char_GFN_Free;
	
	Animatable_Init(&this->character.animatable, char_gfn_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(2,1);
	this->character.focus_y = FIXED_DEC(-40,1);
	this->character.focus_zoom = FIXED_DEC(2,1);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\GFN.ARC;1");
	
	const char **pathp = (const char *[]){
		"gfn0.tim", //GFN_ArcMain_GFN0
		"gfn1.tim", //GFN_ArcMain_GFN1
		"gfn2.tim", //GFN_ArcMain_GFN2
		"gfn3.tim", //GFN_ArcMain_GFN3
		"gfn4.tim", //GFN_ArcMain_GFN4
		"gfn5.tim", //GFN_ArcMain_GFN5
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Load scene specific art
	switch (stage.stage_id)
	{
		//case StageId_1_4: //Tutorial
		//{
		//	this->arc_scene = IO_Read("\\CHAR\\GFTUT.ARC;1");
		//	
		//	const char **pathp = (const char *[]){
		//		"tut0.tim", //GFN_ArcScene_0
		//		"tut1.tim", //GFN_ArcScene_1
		//		NULL
		//	};
		//	IO_Data *arc_ptr = &this->arc_ptr[GF_ArcScene_0];
		//	for (; *pathp != NULL; pathp++)
		//		*arc_ptr++ = Archive_Find(this->arc_scene, *pathp);
		//	break;
		//}
		default:
			this->arc_scene = NULL;
			break;
	}
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	this->pico_p = NULL;
	
	return (Character*)this;
}
