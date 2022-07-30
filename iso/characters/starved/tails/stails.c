/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "stails.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend skull fragments
static SkullFragment char_stails_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend player types
enum
{
	stails_ArcMain_Idle0,
	stails_ArcMain_Idle1,
	stails_ArcMain_Idle2,
	stails_ArcMain_Left0,
	stails_ArcMain_Left0,
	stails_ArcMain_Down0,
	stails_ArcMain_Down1,
	stails_ArcMain_Up0,
	stails_ArcMain_Up1,
	stails_ArcMain_Up2,
	stails_ArcMain_Up3,
	stails_ArcMain_Right0,
	stails_ArcMain_Right1,
	stails_ArcMain_Leftm0,
	stails_ArcMain_Downm0,
	stails_ArcMain_Upm0,
	stails_ArcMain_Upm1,
	stails_ArcMain_Rightm0,

	stails_ArcMain_Dead0, //BREAK
	
	stails_ArcMain_Max,
};

enum
{
	stails_ArcDead_Dead1, //Mic Drop
	stails_ArcDead_Dead2, //Twitch
	stails_ArcDead_Retry, //Retry prompt
	
	stails_ArcDead_Max,
};

#define stails_Arc_Max stails_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[stails_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_stails_skull)];
	u8 skull_scale;
} Char_stails;

//Boyfriend player definitions
static const CharFrame char_stails_frame[] = {
	{stails_ArcMain_Idle0, {  0,   0, 136, 140}, {  0,   0}}, 
	{stails_ArcMain_Idle0, {120, 116, 134, 135}, {  0,   0}}, 
	{stails_ArcMain_Idle1, {  0,   0, 130, 132}, {  0,   0}}, 
	{stails_ArcMain_Idle1, { 71, 122, 130, 129}, {  0,   0}}, 
	{stails_ArcMain_Idle2, {  0,   0, 134, 140}, {  0,   0}}, 
	{stails_ArcMain_Idle2, {119, 116, 137, 140}, {  0,   0}}, 

	{stails_ArcMain_Left0, {  0,   0, 122, 135}, {  0,   0}}, 
	{stails_ArcMain_Left0, {  0,   0, 121, 135}, {  0,   0}}, 
	{stails_ArcMain_Left1, {  0,   0, 119, 137}, {  0,   0}}, 
	{stails_ArcMain_Left1, {  0,   0, 125, 137}, {  0,   0}}, 

	{stails_ArcMain_Down0, {  0,   0, 158, 119}, {  0,   0}}, 
	{stails_ArcMain_Down0, {  0, 119, 158, 122}, {  0,   0}}, 
	{stails_ArcMain_Down1, {  0,   0, 157, 119}, {  0,   0}}, 
	{stails_ArcMain_Down1, {  0, 119, 157, 131}, {  0,   0}}, 

	{stails_ArcMain_Up0, {  0,   0, 145, 150}, {  0,   0}}, 
	{stails_ArcMain_Up1, {  0,   0, 146, 138}, {  0,   0}}, 
	{stails_ArcMain_Up2, {  0,   0, 145, 135}, {  0,   0}}, 
	{stails_ArcMain_Up3, {  0,   0, 152, 147}, {  0,   0}}, 

	{stails_ArcMain_Right0, {  0,   0, 167, 135}, {  0,   0}}, 
	{stails_ArcMain_Right0, { 93, 119, 161, 124}, {  0,   0}}, 
	{stails_ArcMain_Right1, {  0,   0, 167, 122}, {  0,   0}}, 
	{stails_ArcMain_Right1, {  0, 122, 167, 134}, {  0,   0}}, 
	//miss
	{stails_ArcMain_Leftm0, {  0,   0, 124, 136}, {  0,   0}}, 
	{stails_ArcMain_Leftm0, {124,   0, 125, 136}, {  0,   0}}, 

	{stails_ArcMain_Downm0, {  0,   0, 158, 119}, {  0,   0}}, 
	{stails_ArcMain_Downm0, {  0, 119, 158, 122}, {  0,   0}}, 

	{stails_ArcMain_Upm0, {  0,   0, 149, 146}, {  0,   0}}, 
	{stails_ArcMain_Upm1, {  0,   0, 150, 147}, {  0,   0}}, 

	{stails_ArcMain_Rightm0, {  0,   0, 165, 132}, {  0,   0}}, 
	{stails_ArcMain_Rightm0, { 87, 116, 165, 132}, {  0,   0}}, 
/*

	{stails_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //23 dead0 0
	{stails_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //24 dead0 1
	{stails_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //25 dead0 2
	{stails_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //26 dead0 3
	
	{stails_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //27 dead1 0
	{stails_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //28 dead1 1
	{stails_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //29 dead1 2
	{stails_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //30 dead1 3
	
	{stails_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //31 dead2 body twitch 0
	{stails_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //32 dead2 body twitch 1
	{stails_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //33 dead2 balls twitch 0
	{stails_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //34 dead2 balls twitch 1*/
};

static const Animation char_stails_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, ASCR_BACK, 0}}, //CharAnim_Idle
	{2, (const u8[]){ 7, 8, 9, 10, ASCR_BACK, 0}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 11, 12, 13, 14, ASCR_BACK, 0}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 15, 16, 17, 18, ASCR_BACK, 0}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){ 19, 20, 21, 22, ASCR_BACK, 0}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){  7, 23, 23, 24, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 11, 25, 25, 26, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 15, 27, 27, 28, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){ 19, 29, 29, 30, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{2, (const u8[]){13, 14, 15, ASCR_BACK, 1}},         //PlayerAnim_Peace
	{2, (const u8[]){16, 17, 18, 19, ASCR_REPEAT}},      //PlayerAnim_Sweat
	
	{5, (const u8[]){28, 29, 30, 31, 31, 31, 31, 31, 31, 31, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){31, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){32, 33, 34, 35, 35, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){35, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){36, 37, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){38, 39, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){35, 35, 35, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){38, 39, 35, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_stails_SetFrame(void *user, u8 frame)
{
	Char_stails *this = (Char_stails*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_stails_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_stails_Tick(Character *character)
{
	Char_stails *this = (Char_stails*)character;
	
	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_stails, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(56,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_stails_SetFrame);
	Character_Draw(character, &this->tex, &char_stails_frame[this->frame]);
}

void Char_stails_SetAnim(Character *character, u8 anim)
{
	Char_stails *this = (Char_stails*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;
			
			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //stails_ArcDead_Dead1
				"dead2.tim", //stails_ArcDead_Dead2
				"retry.tim", //stails_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[stails_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_stails_Free(Character *character)
{
	Char_stails *this = (Char_stails*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_stails_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_stails *this = Mem_Alloc(sizeof(Char_stails));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_stails_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_stails_Tick;
	this->character.set_anim = Char_stails_SetAnim;
	this->character.free = Char_stails_Free;
	
	Animatable_Init(&this->character.animatable, char_stails_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;

	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\STAILS.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
		
	const char **pathp = (const char *[]){
		"idle0.tim",  
		"idle1.tim",   
		"idle2.tim",  
		"left0.tim",  
		"left1.tim",   
		"down0.tim",  
		"down1.tim", 
		"up0.tim",  
		"up1.tim",  
		"up2.tim",  
		"up3.tim", 
		"right0.tim", 
		"right1.tim",  
		"leftm0.tim",  
		"downm0.tim",  
		"upm0.tim",   
		"upm1.tim", 
		"rightm0.tim",
	//	"dead0.tim", //stails_ArcMain_Dead0

		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_stails_skull, sizeof(char_stails_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_stails, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
