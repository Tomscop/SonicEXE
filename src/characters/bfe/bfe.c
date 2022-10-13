#include "bfe.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../psx/random.h"
#include "../../stage/stage.h"
#include "../../main.h"

//Boyfriend Encore skull fragments
static SkullFragment char_bfe_skull[15] = {
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

//Boyfriend Encore player types
enum
{
	BFE_ArcMain_Idle0,
	BFE_ArcMain_Idle1,
	BFE_ArcMain_Idle2,
	BFE_ArcMain_Idle3,
	BFE_ArcMain_Idle4,
	BFE_ArcMain_Left0,
	BFE_ArcMain_Left1,
	BFE_ArcMain_Left2,
	BFE_ArcMain_Left3,
	BFE_ArcMain_Down0,
	BFE_ArcMain_Down1,
	BFE_ArcMain_Down2,
	BFE_ArcMain_Down3,
	BFE_ArcMain_Up0,
	BFE_ArcMain_Up1,
	BFE_ArcMain_Up2,
	BFE_ArcMain_Up3,
	BFE_ArcMain_Right0,
	BFE_ArcMain_Right1,
	BFE_ArcMain_Right2,
	BFE_ArcMain_Right3,
	BFE_ArcMain_LeftM0,
	BFE_ArcMain_LeftM1,
	BFE_ArcMain_LeftM2,
	BFE_ArcMain_DownM0,
	BFE_ArcMain_DownM1,
	BFE_ArcMain_DownM2,
	BFE_ArcMain_UpM0,
	BFE_ArcMain_UpM1,
	BFE_ArcMain_UpM2,
	BFE_ArcMain_RightM0,
	BFE_ArcMain_RightM1,
	BFE_ArcMain_RightM2,
	BFE_ArcMain_Dead0, //BREAK
	BFE_ArcDead_Dead1, //Mic Drop
	BFE_ArcDead_Dead2, //Twitch
	BFE_ArcDead_Retry,
	
	BFE_ArcMain_Max,
};

#define BFE_Arc_Max BFE_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BFE_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bfe_skull)];
	u8 skull_scale;
} Char_BFE;

//Boyfriend Encore player definitions
static const CharFrame char_bfe_frame[] = {
	{BFE_ArcMain_Idle0, {  0,  0,155,136}, { 53, 92}}, //0 idle 1
	{BFE_ArcMain_Idle1, {  0,  0,155,136}, { 53, 92}}, //1 idle 2
	{BFE_ArcMain_Idle2, {  0,  0,155,136}, { 53, 92}}, //2 idle 3
	{BFE_ArcMain_Idle3, {  0,  0,155,136}, { 53, 92}}, //3 idle 4
	{BFE_ArcMain_Idle4, {  0,  0,155,136}, { 53, 92}}, //4 idle 5
	
	{BFE_ArcMain_Left0, {  0,  0,155,136}, { 53, 92}}, //5 left 1
	{BFE_ArcMain_Left1, {  0,  0,155,136}, { 53, 92}}, //6 left 2
	{BFE_ArcMain_Left2, {  0,  0,155,136}, { 53, 92}}, //7 left 3
	{BFE_ArcMain_Left3, {  0,  0,155,136}, { 53, 92}}, //8 left 4
	
	{BFE_ArcMain_Down0, {  0,  0,155,136}, { 53, 92}}, //9 down 1
	{BFE_ArcMain_Down1, {  0,  0,155,136}, { 53, 92}}, //10 down 2
	{BFE_ArcMain_Down2, {  0,  0,155,136}, { 53, 92}}, //11 down 3
	{BFE_ArcMain_Down3, {  0,  0,155,136}, { 53, 92}}, //12 down 4
	
	{BFE_ArcMain_Up0, {  0,  0,155,136}, { 53, 92}}, //13 up 1
	{BFE_ArcMain_Up1, {  0,  0,155,136}, { 53, 92}}, //14 up 2
	{BFE_ArcMain_Up2, {  0,  0,155,136}, { 53, 92}}, //15 up 3
	{BFE_ArcMain_Up3, {  0,  0,155,136}, { 53, 92}}, //16 up 4
	
	{BFE_ArcMain_Right0, {  0,  0,155,136}, { 53, 92}}, //17 right 1
	{BFE_ArcMain_Right1, {  0,  0,155,136}, { 53, 92}}, //18 right 2
	{BFE_ArcMain_Right2, {  0,  0,155,136}, { 53, 92}}, //19 right 3
	{BFE_ArcMain_Right3, {  0,  0,155,136}, { 53, 92}}, //20 right 4
	
	{BFE_ArcMain_LeftM0, {  0,  0,155,136}, { 53, 92}}, //21 left miss 1
	{BFE_ArcMain_LeftM1, {  0,  0,155,136}, { 53, 92}}, //22 left miss 2
	{BFE_ArcMain_LeftM2, {  0,  0,155,136}, { 53, 92}}, //23 left miss 3
	
	{BFE_ArcMain_DownM0, {  0,  0,155,136}, { 53, 92}}, //24 down miss 1
	{BFE_ArcMain_DownM1, {  0,  0,155,136}, { 53, 92}}, //25 down miss 2
	{BFE_ArcMain_DownM2, {  0,  0,155,136}, { 53, 92}}, //26 down miss 3
	
	{BFE_ArcMain_UpM0, {  0,  0,155,136}, { 53, 92}}, //27 up miss 1
	{BFE_ArcMain_UpM1, {  0,  0,155,136}, { 53, 92}}, //28 up miss 2
	{BFE_ArcMain_UpM2, {  0,  0,155,136}, { 53, 92}}, //29 up miss 3
	
	{BFE_ArcMain_RightM0, {  0,  0,155,136}, { 53, 92}}, //30 right miss 1
	{BFE_ArcMain_RightM1, {  0,  0,155,136}, { 53, 92}}, //31 right miss 2
	{BFE_ArcMain_RightM2, {  0,  0,155,136}, { 53, 92}}, //32 right miss 3

	{BFE_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //33 dead0 0
	{BFE_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //34 dead0 1
	{BFE_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //35 dead0 2
	{BFE_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //36 dead0 3
	
	{BFE_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //37 dead1 0
	{BFE_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //38 dead1 1
	{BFE_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //39 dead1 2
	{BFE_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //40 dead1 3
	
	{BFE_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //41 dead2 body twitch 0
	{BFE_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //42 dead2 body twitch 1
	{BFE_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //43 dead2 balls twitch 0
	{BFE_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //44 dead2 balls twitch 1
};

static const Animation char_bfe_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5, 6, 7, 8, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 9, 10, 11, 12, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 13, 14, 15, 16, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){ 17, 18, 19, 20, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){ 21, 22, 23, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 24, 25, 26, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 27, 28, 29, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){ 30, 31, 32, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //PlayerAnim_Peace
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},      //PlayerAnim_Sweat
	
	{2, (const u8[]){33, 34, 35, 36, 36, 36, 36, 36, 36, 36, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{2, (const u8[]){36, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){37, 38, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{2, (const u8[]){40, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){41, 42, 40, 40, 40, 40, 40, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){43, 44, 40, 40, 40, 40, 40, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){40, 40, 40, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){43, 44, 40, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend Encore player functions
void Char_BFE_SetFrame(void *user, u8 frame)
{
	Char_BFE *this = (Char_BFE*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfe_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFE_Tick(Character *character)
{
	Char_BFE *this = (Char_BFE*)character;
	
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
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFE, skull); i++, frag++)
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
	Animatable_Animate(&character->animatable, (void*)this, Char_BFE_SetFrame);
	Character_Draw(character, &this->tex, &char_bfe_frame[this->frame]);
}

void Char_BFE_SetAnim(Character *character, u8 anim)
{
	Char_BFE *this = (Char_BFE*)character;
	
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
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BFE_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BFE_Free(Character *character)
{
	Char_BFE *this = (Char_BFE*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BFE_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriendencore object
	Char_BFE *this = Mem_Alloc(sizeof(Char_BFE));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFE_New] Failed to allocate boyfriendencore object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFE_Tick;
	this->character.set_anim = Char_BFE_SetAnim;
	this->character.free = Char_BFE_Free;
	
	Animatable_Init(&this->character.animatable, char_bfe_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 0;
	
	this->character.focus_x = FIXED_DEC(-20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFE.ARC;1");
	this->arc_dead = NULL;
	//IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",   //BFE_ArcMain_Idle0
		"idle1.tim",   //BFE_ArcMain_Idle1
		"idle2.tim",   //BFE_ArcMain_Idle2
		"idle3.tim",   //BFE_ArcMain_Idle3
		"idle4.tim",   //BFE_ArcMain_Idle4
		"left0.tim",   //BFE_ArcMain_Left0
		"left1.tim",   //BFE_ArcMain_Left1
		"left2.tim",   //BFE_ArcMain_Left2
		"left3.tim",   //BFE_ArcMain_Left3
		"down0.tim",   //BFE_ArcMain_Down0
		"down1.tim",   //BFE_ArcMain_Down1
		"down2.tim",   //BFE_ArcMain_Down2
		"down3.tim",   //BFE_ArcMain_Down3
		"up0.tim",   //BFE_ArcMain_Up0
		"up1.tim",   //BFE_ArcMain_Up1
		"up2.tim",   //BFE_ArcMain_Up2
		"up3.tim",   //BFE_ArcMain_Up3
		"right0.tim",   //BFE_ArcMain_Right0
		"right1.tim",   //BFE_ArcMain_Right1
		"right2.tim",   //BFE_ArcMain_Right2
		"right3.tim",   //BFE_ArcMain_Right3
		"leftm0.tim",   //BFE_ArcMain_LeftM0
		"leftm1.tim",   //BFE_ArcMain_LeftM1
		"leftm2.tim",   //BFE_ArcMain_LeftM2
		"downm0.tim",   //BFE_ArcMain_DownM0
		"downm1.tim",   //BFE_ArcMain_DownM1
		"downm2.tim",   //BFE_ArcMain_DownM2
		"upm0.tim",   //BFE_ArcMain_UpM0
		"upm1.tim",   //BFE_ArcMain_UpM1
		"upm2.tim",   //BFE_ArcMain_UpM2
		"rightm0.tim",   //BFE_ArcMain_RightM0
		"rightm1.tim",   //BFE_ArcMain_RightM1
		"rightm2.tim",   //BFE_ArcMain_RightM2
		"dead0.tim", //BFE_ArcMain_Dead0
		"dead1.tim", //BFE_ArcDead_Dead1
		"dead2.tim", //BFE_ArcDead_Dead2
		"retry.tim", //BFE_ArcDead_Retry
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
	memcpy(this->skull, char_bfe_skull, sizeof(char_bfe_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFE, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
