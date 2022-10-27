#include "majin.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//Majin OG character structure
enum
{
	Majin_ArcMain_Idle0,
	Majin_ArcMain_Idle1,
	Majin_ArcMain_Idle2,
	
	Majin_ArcMain_Left0,
	Majin_ArcMain_Left1,
	Majin_ArcMain_Left2,
	Majin_ArcMain_Left3,
	
	Majin_ArcMain_Down0,
	Majin_ArcMain_Down1,
	Majin_ArcMain_Down2,
	Majin_ArcMain_Down3,
	
	Majin_ArcMain_Up0,
	Majin_ArcMain_Up1,
	Majin_ArcMain_Up2,
	Majin_ArcMain_Up3,
	
	Majin_ArcMain_Right0,
	Majin_ArcMain_Right1,
	Majin_ArcMain_Right2,
	Majin_ArcMain_Right3,
	
	Majin_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Majin_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Majin;

//Majin OG character definitions
static const CharFrame char_majin_frame[] = {
	{Majin_ArcMain_Idle0,{0,0,125,170},{41,160}},
	{Majin_ArcMain_Idle0,{125,0,123,170},{38,160}},
	{Majin_ArcMain_Idle1,{0,0,115,166},{27,156}},
	{Majin_ArcMain_Idle1,{115,0,115,166},{27,156}},
	{Majin_ArcMain_Idle2,{0,0,117,170},{30,160}},
	{Majin_ArcMain_Idle2,{117,0,125,170},{42,160}},
	
	{Majin_ArcMain_Left0,{0,0,180,138},{152,127}},
	{Majin_ArcMain_Left1,{0,0,182,138},{154,128}},
	{Majin_ArcMain_Left2,{0,0,186,140},{156,129}},
	{Majin_ArcMain_Left3,{0,0,186,138},{156,129}},
	
	{Majin_ArcMain_Down0,{0,0,154,132},{70,123}},
	{Majin_ArcMain_Down1,{0,0,156,130},{70,120}},
	{Majin_ArcMain_Down2,{0,0,156,134},{70,125}},
	{Majin_ArcMain_Down3,{0,0,158,134},{70,125}},
	
	{Majin_ArcMain_Up0,{0,0,126,174},{61,164}},
	{Majin_ArcMain_Up1,{0,0,128,172},{61,162}},
	{Majin_ArcMain_Up2,{0,0,124,172},{58,162}},
	{Majin_ArcMain_Up3,{0,0,124,172},{58,162}},
	
	{Majin_ArcMain_Right0,{0,0,158,156},{22,146}},
	{Majin_ArcMain_Right1,{0,0,156,156},{22,146}},
	{Majin_ArcMain_Right2,{0,0,154,156},{22,147}},
	{Majin_ArcMain_Right3,{0,0,154,156},{22,147}},
};

static const Animation char_majin_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 6, 7, 8, 9, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 10, 11, 12, 13, ASCR_BACK, 0}},         //CharAnim_Down
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_DownAlt
	{2, (const u8[]){ 14, 15, 16, 17, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){18, 19, 20, 21, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Majin OG character functions
void Char_Majin_SetFrame(void *user, u8 frame)
{
	Char_Majin *this = (Char_Majin*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_majin_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Majin_Tick(Character *character)
{
	Char_Majin *this = (Char_Majin*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Majin_SetFrame);
	Character_Draw(character, &this->tex, &char_majin_frame[this->frame]);
}

void Char_Majin_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Majin_Free(Character *character)
{
	Char_Majin *this = (Char_Majin*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Majin_New(fixed_t x, fixed_t y)
{
	//Allocate majin object
	Char_Majin *this = Mem_Alloc(sizeof(Char_Majin));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Majin_New] Failed to allocate majin object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Majin_Tick;
	this->character.set_anim = Char_Majin_SetAnim;
	this->character.free = Char_Majin_Free;
	
	Animatable_Init(&this->character.animatable, char_majin_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MAJIN.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Majin_ArcMain_Idle0
		"idle1.tim", //Majin_ArcMain_Idle1
		"idle2.tim", //Majin_ArcMain_Idle2
		
		"left0.tim", //Majin_ArcMain_Left0
		"left1.tim", //Majin_ArcMain_Left1
		"left2.tim", //Majin_ArcMain_Left2
		"left3.tim", //Majin_ArcMain_Left3
		
		"down0.tim", //Majin_ArcMain_Down0
		"down1.tim", //Majin_ArcMain_Down1
		"down2.tim", //Majin_ArcMain_Down2
		"down3.tim", //Majin_ArcMain_Down3
		
		"up0.tim", //Majin_ArcMain_Up0
		"up1.tim", //Majin_ArcMain_Up1
		"up2.tim", //Majin_ArcMain_Up2
		"up3.tim", //Majin_ArcMain_Up3
		
		"right0.tim", //Majin_ArcMain_Right0
		"right1.tim", //Majin_ArcMain_Right1
		"right2.tim", //Majin_ArcMain_Right2
		"right3.tim", //Majin_ArcMain_Right3
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
