#include "bfn.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../psx/random.h"
#include "../../stage/stage.h"
#include "../../main.h"

//Boyfriend NMouse skull fragments
static SkullFragment char_bfn_skull[15] = {
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

//Boyfriend NMouse player types
enum
{
	BFN_ArcMain_BFN0,
	BFN_ArcMain_BFN1,
	BFN_ArcMain_BFN2,
	BFN_ArcMain_BFN3,
	BFN_ArcMain_BFN4,
	BFN_ArcMain_BFN5,
	BFN_ArcMain_BFN6,
	BFN_ArcMain_BFN7,
	BFN_ArcMain_BFN8,
	BFN_ArcMain_BFN9,
	BFN_ArcMain_BFN10,
	BFN_ArcMain_BFN11,
	BFN_ArcMain_BFN12,
	BFN_ArcMain_BFN13,
	BFN_ArcMain_BFN14,
	BFN_ArcMain_BFN15,
	BFN_ArcMain_Dead0, //BREAK
	BFN_ArcDead_Dead1, //Mic Drop
	BFN_ArcDead_Dead2, //Twitch
	BFN_ArcDead_Retry,
	
	BFN_ArcMain_Max,
};

#define BFN_Arc_Max BFN_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BFN_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bfn_skull)];
	u8 skull_scale;
} Char_BFN;

//Boyfriend NMouse player definitions
static const CharFrame char_bfn_frame[] = {
	{BFN_ArcMain_BFN0, {  0,  0,119,145}, { 53, 98}}, //0 idle 1
	{BFN_ArcMain_BFN0, {120,  0,119,145}, { 53, 98}}, //1 idle 2
	{BFN_ArcMain_BFN1, {  0,  0,119,145}, { 53, 98}}, //2 idle 3
	{BFN_ArcMain_BFN1, {120,  0,119,145}, { 53, 98}}, //3 idle 4
	{BFN_ArcMain_BFN2, {  0,  0,119,145}, { 53, 98}}, //4 idle 5
	{BFN_ArcMain_BFN2, {120,  0,119,145}, { 53, 98}}, //5 idle 6
	{BFN_ArcMain_BFN3, {  0,  0,119,145}, { 53, 98}}, //6 idle 7
	{BFN_ArcMain_BFN3, {120,  0,119,145}, { 53, 98}}, //7 idle 8
	{BFN_ArcMain_BFN4, {  0,  0,119,145}, { 53, 98}}, //8 idle 9
	{BFN_ArcMain_BFN4, {120,  0,119,145}, { 53, 98}}, //9 idle 10
	{BFN_ArcMain_BFN5, {  0,  0,119,145}, { 53, 98}}, //10 idle 11
	{BFN_ArcMain_BFN5, {120,  0,119,145}, { 53, 98}}, //11 idle 12
	
	{BFN_ArcMain_BFN6, {  0,  0,119,145}, { 53, 98}}, //12 left 1
	{BFN_ArcMain_BFN6, {120,  0,119,145}, { 53, 98}}, //13 left 2
	{BFN_ArcMain_BFN7, {  0,  0,119,145}, { 53, 98}}, //14 left 3
	
	{BFN_ArcMain_BFN7, {120,  0,119,145}, { 53, 98}}, //15 down 1
	{BFN_ArcMain_BFN8, {  0,  0,119,145}, { 53, 98}}, //16 down 2
	{BFN_ArcMain_BFN8, {120,  0,119,145}, { 53, 98}}, //17 down 3
	
	{BFN_ArcMain_BFN9, {  0,  0,119,145}, { 53, 98}}, //18 up 1
	{BFN_ArcMain_BFN9, {120,  0,119,145}, { 53, 98}}, //19 up 2
	{BFN_ArcMain_BFN10, {  0,  0,119,145}, { 53, 98}}, //20 up 3
	
	{BFN_ArcMain_BFN10, {120,  0,119,145}, { 53, 98}}, //21 right 1
	{BFN_ArcMain_BFN11, {  0,  0,119,145}, { 53, 98}}, //22 right 2
	{BFN_ArcMain_BFN11, {120,  0,119,145}, { 53, 98}}, //23 right 3
	
	{BFN_ArcMain_BFN12, {  0,  0,119,145}, { 53, 98}}, //24 left miss 1
	{BFN_ArcMain_BFN12, {120,  0,119,145}, { 53, 98}}, //25 left miss 2
	
	{BFN_ArcMain_BFN13, {  0,  0,119,145}, { 53, 98}}, //26 down miss 1
	{BFN_ArcMain_BFN13, {120,  0,119,145}, { 53, 98}}, //27 down miss 2
	
	{BFN_ArcMain_BFN14, {  0,  0,119,145}, { 53, 98}}, //28 up miss 1
	{BFN_ArcMain_BFN14, {120,  0,119,145}, { 53, 98}}, //29 up miss 2
	
	{BFN_ArcMain_BFN15, {  0,  0,119,145}, { 53, 98}}, //30 right miss 1
	{BFN_ArcMain_BFN15, {120,  0,119,145}, { 53, 98}}, //31 right miss 2

	{BFN_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //32 dead0 0
	{BFN_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //33 dead0 1
	{BFN_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //34 dead0 2
	{BFN_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //35 dead0 3
	
	{BFN_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //36 dead1 0
	{BFN_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //37 dead1 1
	{BFN_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //38 dead1 2
	{BFN_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //39 dead1 3
	
	{BFN_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //40 dead2 body twitch 0
	{BFN_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //41 dead2 body twitch 1
	{BFN_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //42 dead2 balls twitch 0
	{BFN_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //43 dead2 balls twitch 1
};

static const Animation char_bfn_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 12, 13, 14, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 15, 16, 17, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 18, 19, 20, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){ 21, 22, 23, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){ 24, 25, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 26, 27, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 28, 29, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){ 30, 31, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{2, (const u8[]){13, 14, 15, ASCR_BACK, 1}},         //PlayerAnim_Peace
	{2, (const u8[]){16, 17, 18, 19, ASCR_REPEAT}},      //PlayerAnim_Sweat
	
	{2, (const u8[]){32, 33, 34, 35, 35, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{2, (const u8[]){35, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){36, 37, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{2, (const u8[]){39, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){40, 41, 39, 39, 39, 39, 39, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){42, 43, 39, 39, 39, 39, 39, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){39, 39, 39, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){42, 43, 39, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend NMouse player functions
void Char_BFN_SetFrame(void *user, u8 frame)
{
	Char_BFN *this = (Char_BFN*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfn_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFN_Tick(Character *character)
{
	Char_BFN *this = (Char_BFN*)character;
	
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
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFN, skull); i++, frag++)
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
	Animatable_Animate(&character->animatable, (void*)this, Char_BFN_SetFrame);
	Character_Draw(character, &this->tex, &char_bfn_frame[this->frame]);
}

void Char_BFN_SetAnim(Character *character, u8 anim)
{
	Char_BFN *this = (Char_BFN*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BFN_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BFN_Free(Character *character)
{
	Char_BFN *this = (Char_BFN*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BFN_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend nmouse object
	Char_BFN *this = Mem_Alloc(sizeof(Char_BFN));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFN_New] Failed to allocate boyfriend nmouse object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFN_Tick;
	this->character.set_anim = Char_BFN_SetAnim;
	this->character.free = Char_BFN_Free;
	
	Animatable_Init(&this->character.animatable, char_bfn_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 0;
	
	this->character.focus_x = FIXED_DEC(-20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(80,100);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFN.ARC;1");
	this->arc_dead = NULL;
	//IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFNDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"bfn0.tim",   //BFN_ArcMain_BFN0
		"bfn1.tim",   //BFN_ArcMain_BFN1
		"bfn2.tim",   //BFN_ArcMain_BFN2
		"bfn3.tim",   //BFN_ArcMain_BFN3
		"bfn4.tim",   //BFN_ArcMain_BFN4
		"bfn5.tim",   //BFN_ArcMain_BFN5
		"bfn6.tim",   //BFN_ArcMain_BFN6
		"bfn7.tim",   //BFN_ArcMain_BFN7
		"bfn8.tim",   //BFN_ArcMain_BFN8
		"bfn9.tim",   //BFN_ArcMain_BFN9
		"bfn10.tim",   //BFN_ArcMain_BFN10
		"bfn11.tim",   //BFN_ArcMain_BFN11
		"bfn12.tim",   //BFN_ArcMain_BFN12
		"bfn13.tim",   //BFN_ArcMain_BFN13
		"bfn14.tim",   //BFN_ArcMain_BFN14
		"bfn15.tim",   //BFN_ArcMain_BFN15
		"dead0.tim", //BFN_ArcMain_Dead0
		"dead1.tim", //BFN_ArcDead_Dead1
		"dead2.tim", //BFN_ArcDead_Dead2
		"retry.tim", //BFN_ArcDead_Retry
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
	memcpy(this->skull, char_bfn_skull, sizeof(char_bfn_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFN, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
