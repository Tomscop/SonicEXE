#include "bfsunky.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../psx/random.h"
#include "../../stage/stage.h"
#include "../../main.h"

//Boyfriend Sunky skull fragments
static SkullFragment char_bfsunky_skull[15] = {
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

//Boyfriend Sunky player types
enum
{
	BFSunky_ArcMain_Idle0,
	BFSunky_ArcMain_Idle1,
	BFSunky_ArcMain_Idle2,
	BFSunky_ArcMain_Left0,
	BFSunky_ArcMain_Left1,
	BFSunky_ArcMain_Down,
	BFSunky_ArcMain_Up0,
	BFSunky_ArcMain_Up1,
	BFSunky_ArcMain_Right0,
	BFSunky_ArcMain_Right1,
	BFSunky_ArcMain_LeftM,
	BFSunky_ArcMain_DownM,
	BFSunky_ArcMain_UpM,
	BFSunky_ArcMain_RightM,
	BFSunky_ArcMain_Dead0, //BREAK
	BFSunky_ArcDead_Dead1, //Mic Drop
	BFSunky_ArcDead_Dead2, //Twitch
	BFSunky_ArcDead_Retry,
	
	BFSunky_ArcMain_Max,
};

#define BFSunky_Arc_Max BFSunky_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BFSunky_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bfsunky_skull)];
	u8 skull_scale;
} Char_BFSunky;

//Boyfriend Sunky player definitions
static const CharFrame char_bfsunky_frame[] = {
	{BFSunky_ArcMain_Idle0, {  0,  0,122,129}, {  0,  0}}, //0 idle 1
	{BFSunky_ArcMain_Idle0, {123,  0,122,129}, {  2,  0}}, //1 idle 2
	{BFSunky_ArcMain_Idle1, {  0,  0,117,135}, { -6,  5}}, //2 idle 3
	{BFSunky_ArcMain_Idle1, {118,  0,116,133}, { -6,  3}}, //3 idle 4
	{BFSunky_ArcMain_Idle2, {  0,  0,117,133}, { -6,  3}}, //4 idle 5
	
	{BFSunky_ArcMain_Left0, {  0,  0,109,133}, { -3,  3}}, //5 left 1
	{BFSunky_ArcMain_Left0, {110,  0,109,133}, { -3,  2}}, //6 left 2
	{BFSunky_ArcMain_Left1, {  0,  0,108,132}, { -3,  2}}, //7 left 3
	{BFSunky_ArcMain_Left1, {109,  0,109,133}, { -3,  3}}, //8 left 4
	
	{BFSunky_ArcMain_Down, {  0,  0,122,110}, {  0,-19}}, //9 down 1
	{BFSunky_ArcMain_Down, {123,  0,120,112}, { -1,-17}}, //10 down 2
	{BFSunky_ArcMain_Down, {  0,113,119,113}, { -1,-16}}, //11 down 3
	{BFSunky_ArcMain_Down, {120,113,121,111}, {  0,-18}}, //12 down 4
	
	{BFSunky_ArcMain_Up0, {  0,  0,126,157}, {-15, 27}}, //13 up 1
	{BFSunky_ArcMain_Up0, {127,  0,126,154}, {-13, 24}}, //14 up 2
	{BFSunky_ArcMain_Up1, {  0,  0,126,153}, {-13, 24}}, //15 up 3
	{BFSunky_ArcMain_Up1, {127,  0,124,155}, {-16, 25}}, //16 up 4
	
	{BFSunky_ArcMain_Right0, {  0,  0,126,135}, { -7,  5}}, //17 right 1
	{BFSunky_ArcMain_Right0, {127,  0,126,135}, { -6,  4}}, //18 right 2
	{BFSunky_ArcMain_Right1, {  0,  0,125,134}, { -6,  4}}, //19 right 3
	{BFSunky_ArcMain_Right1, {126,  0,126,135}, { -6,  4}}, //20 right 4
	
	{BFSunky_ArcMain_LeftM, {  0,  0,108,133}, { -3,  3}}, //21 left miss 1
	{BFSunky_ArcMain_LeftM, {109,  0,108,131}, { -3,  1}}, //22 left miss 2
	
	{BFSunky_ArcMain_DownM, {  0,  0,120,113}, {  0,-17}}, //23 down miss 1
	{BFSunky_ArcMain_DownM, {121,  0,122,111}, {  1,-19}}, //24 down miss 2
	
	{BFSunky_ArcMain_UpM, {  0,  0,125,157}, {-16, 27}}, //25 up miss 1
	{BFSunky_ArcMain_UpM, {126,  0,130,155}, {-14, 24}}, //26 up miss 2
	
	{BFSunky_ArcMain_RightM, {  0,  0,125,135}, { -7,  4}}, //27 right miss 1
	{BFSunky_ArcMain_RightM, {126,  0,125,133}, { -6,  3}}, //28 right miss 2

	{BFSunky_ArcMain_Dead0, {  0,   0, 128, 128}, {  0,   0}}, //29 dead0 0
	{BFSunky_ArcMain_Dead0, {128,   0, 128, 128}, {  0,   0}}, //30 dead0 1
	{BFSunky_ArcMain_Dead0, {  0, 128, 128, 128}, {  0,   0}}, //31 dead0 2
	{BFSunky_ArcMain_Dead0, {128, 128, 128, 128}, {  0,   0}}, //32 dead0 3
	
	{BFSunky_ArcDead_Dead1, {  0,   0, 128, 128}, {  0,   0}}, //33 dead1 0
	{BFSunky_ArcDead_Dead1, {128,   0, 128, 128}, {  0,   0}}, //34 dead1 1
	{BFSunky_ArcDead_Dead1, {  0, 128, 128, 128}, {  0,   0}}, //35 dead1 2
	{BFSunky_ArcDead_Dead1, {128, 128, 128, 128}, {  0,   0}}, //36 dead1 3
	
	{BFSunky_ArcDead_Dead2, {  0,   0, 128, 128}, {  0,   0}}, //37 dead2 body twitch 0
	{BFSunky_ArcDead_Dead2, {128,   0, 128, 128}, {  0,   0}}, //38 dead2 body twitch 1
	{BFSunky_ArcDead_Dead2, {  0, 128, 128, 128}, {  0,   0}}, //39 dead2 balls twitch 0
	{BFSunky_ArcDead_Dead2, {128, 128, 128, 128}, {  0,   0}}, //40 dead2 balls twitch 1
};

static const Animation char_bfsunky_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5, 6, 7, 8, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 9, 10, 11, 12, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 13, 14, 15, 16, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){ 17, 18, 19, 20, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){ 21, 22, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 23, 24, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 25, 26, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){ 27, 28, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //PlayerAnim_Peace
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},      //PlayerAnim_Sweat
	
	{2, (const u8[]){29, 30, 31, 32, 32, 32, 32, 32, 32, 32, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{2, (const u8[]){32, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){33, 34, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{2, (const u8[]){36, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){37, 38, 36, 36, 36, 36, 36, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){39, 40, 36, 36, 36, 36, 36, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){36, 36, 36, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){39, 40, 36, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend Sunky player functions
void Char_BFSunky_SetFrame(void *user, u8 frame)
{
	Char_BFSunky *this = (Char_BFSunky*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfsunky_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFSunky_Tick(Character *character)
{
	Char_BFSunky *this = (Char_BFSunky*)character;
	
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
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFSunky, skull); i++, frag++)
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
			character->x - FIXED_DEC(29,1) - stage.camera.x,
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
			29
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(29,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_BFSunky_SetFrame);
	Character_Draw(character, &this->tex, &char_bfsunky_frame[this->frame]);
}

void Char_BFSunky_SetAnim(Character *character, u8 anim)
{
	Char_BFSunky *this = (Char_BFSunky*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-37,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BFSunky_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BFSunky_Free(Character *character)
{
	Char_BFSunky *this = (Char_BFSunky*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BFSunky_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend sunky object
	Char_BFSunky *this = Mem_Alloc(sizeof(Char_BFSunky));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFSunky_New] Failed to allocate boyfriend sunky object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFSunky_Tick;
	this->character.set_anim = Char_BFSunky_SetAnim;
	this->character.free = Char_BFSunky_Free;
	
	Animatable_Init(&this->character.animatable, char_bfsunky_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 0;
	
	this->character.focus_x = FIXED_DEC(-20,1);
	this->character.focus_y = FIXED_DEC(-20,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFSUNKY.ARC;1");
	this->arc_dead = NULL;
	//IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFSunkyDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",   //BFSunky_ArcMain_Idle0
		"idle1.tim",   //BFSunky_ArcMain_Idle1
		"idle2.tim",   //BFSunky_ArcMain_Idle2
		"left0.tim",   //BFSunky_ArcMain_Left0
		"left1.tim",   //BFSunky_ArcMain_Left1
		"down.tim",   //BFSunky_ArcMain_Down
		"up0.tim",   //BFSunky_ArcMain_Up0
		"up1.tim",   //BFSunky_ArcMain_Up1
		"right0.tim",   //BFSunky_ArcMain_Right0
		"right1.tim",   //BFSunky_ArcMain_Right1
		"leftm.tim",   //BFSunky_ArcMain_LeftM
		"downm.tim",   //BFSunky_ArcMain_DownM
		"upm.tim",   //BFSunky_ArcMain_UpM
		"rightm.tim",   //BFSunky_ArcMain_RightM
		"dead0.tim", //BFSunky_ArcMain_Dead0
		"dead1.tim", //BFSunky_ArcDead_Dead1
		"dead2.tim", //BFSunky_ArcDead_Dead2
		"retry.tim", //BFSunky_ArcDead_Retry
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
	memcpy(this->skull, char_bfsunky_skull, sizeof(char_bfsunky_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFSunky, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
