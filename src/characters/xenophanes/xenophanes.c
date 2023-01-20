#include "xenophanes.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//Xenophanes character structure
enum
{
  	Xeno_ArcMain_Xeno0,
  	Xeno_ArcMain_Xeno1,
  	Xeno_ArcMain_Xeno2,
  	Xeno_ArcMain_Xeno3,
  	Xeno_ArcMain_Xeno4,
	Xeno_ArcMain_Xeno5,
  	Xeno_ArcMain_Xeno6,
  	Xeno_ArcMain_Xeno7,
  	Xeno_ArcMain_Xeno8,
  	Xeno_ArcMain_Xeno9,
  	Xeno_ArcMain_Xeno10,
  	Xeno_ArcMain_Xeno11,
  	Xeno_ArcMain_Xeno12,
  	Xeno_ArcMain_Xeno13,
  	Xeno_ArcMain_Xeno14,
  	Xeno_ArcMain_Xeno15,
  	Xeno_ArcMain_Xeno16,
  	Xeno_ArcMain_Xeno17,
  	Xeno_ArcMain_Xeno18,
  	Xeno_ArcMain_Xeno19,
  	Xeno_ArcMain_Xeno20,
  	Xeno_ArcMain_Xeno21,
  	Xeno_ArcMain_Xeno22,
  	Xeno_ArcMain_Xeno23,
	
	Xeno_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Xeno_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Xeno;

//Xeno character definitions
static const CharFrame char_xeno_frame[] = {
  	{Xeno_ArcMain_Xeno0, {  2,  0,126,217}, { 84, 84}}, //0 idle 1
  	{Xeno_ArcMain_Xeno1, {  0,  0,126,216}, { 85, 83}}, //1 idle 2
  	{Xeno_ArcMain_Xeno1, {128,  0,126,217}, { 85, 83}}, //2 idle 3
  	{Xeno_ArcMain_Xeno2, {  0,  0,126,218}, { 86, 84}}, //3 idle 4
  	{Xeno_ArcMain_Xeno3, {  0,  0,129,219}, { 89, 85}}, //4 idle 5
  	{Xeno_ArcMain_Xeno4, {  0,  0,127,223}, { 88, 89}}, //5 idle 6
  	{Xeno_ArcMain_Xeno5, {  0,  0,127,223}, { 88, 89}}, //6 idle 7
  	{Xeno_ArcMain_Xeno6, {  0,  0,127,223}, { 88, 89}}, //7 idle 8

  	{Xeno_ArcMain_Xeno7, {  0,  0,204,202}, {128, 68}}, //8 left 1
  	{Xeno_ArcMain_Xeno8, {  0,  0,201,202}, {122, 67}}, //9 left 2
  	{Xeno_ArcMain_Xeno9, {  0,  0,194,196}, {105, 61}}, //10 left 3
  	{Xeno_ArcMain_Xeno10, {  0,  0,198,198}, {109, 63}}, //11 left 4

  	{Xeno_ArcMain_Xeno11, {  0,  0,146,189}, { 95, 55}}, //12 down 1
  	{Xeno_ArcMain_Xeno12, {  0,  0,146,202}, { 95, 67}}, //13 down 2
  	{Xeno_ArcMain_Xeno13, {  0,  0,146,198}, { 95, 64}}, //14 down 3
  	{Xeno_ArcMain_Xeno14, {  0,  0,145,198}, { 94, 64}}, //15 down 4

  	{Xeno_ArcMain_Xeno15, {  0,  0,147,241}, { 99,104}}, //16 up 1
  	{Xeno_ArcMain_Xeno16, {  0,  0,148,238}, {100,101}}, //17 up 2
  	{Xeno_ArcMain_Xeno17, {  0,  0,147,227}, { 99, 91}}, //18 up 3
  	{Xeno_ArcMain_Xeno18, {  0,  0,148,229}, {100, 93}}, //19 up 4

  	{Xeno_ArcMain_Xeno19, {  0,  0,179,206}, { 44, 73}}, //20 right 1
  	{Xeno_ArcMain_Xeno20, {  0,  0,178,207}, { 44, 74}}, //21 right 2
  	{Xeno_ArcMain_Xeno21, {  0,  0,174,207}, { 44, 74}}, //22 right 3
  	{Xeno_ArcMain_Xeno22, {  0,  0,173,208}, { 43, 75}}, //23 right 4
};

static const Animation char_xeno_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5, 6, 7, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 8,  9,  10,  11, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 12,  13, 14, 15, ASCR_BACK, 0}},         //CharAnim_Down
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_DownAlt
	{2, (const u8[]){ 16,  17, 18, 19, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){20, 21, 22, 23, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Xeno character functions
void Char_Xeno_SetFrame(void *user, u8 frame)
{
	Char_Xeno *this = (Char_Xeno*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_xeno_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Xeno_Tick(Character *character)
{
	Char_Xeno *this = (Char_Xeno*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Xeno_SetFrame);
	Character_Draw(character, &this->tex, &char_xeno_frame[this->frame]);
}

void Char_Xeno_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Xeno_Free(Character *character)
{
	Char_Xeno *this = (Char_Xeno*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Xeno_New(fixed_t x, fixed_t y)
{
	//Allocate Xeno object
	Char_Xeno *this = Mem_Alloc(sizeof(Char_Xeno));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Xeno_New] Failed to allocate Xeno object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Xeno_Tick;
	this->character.set_anim = Char_Xeno_SetAnim;
	this->character.free = Char_Xeno_Free;
	
	Animatable_Init(&this->character.animatable, char_xeno_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(11,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\XENO.ARC;1");
	
	const char **pathp = (const char *[]){
  		"dad0.tim",
  		"dad1.tim",
  		"dad2.tim",
  		"dad3.tim",
  		"dad4.tim",
  		"dad5.tim",
  		"dad6.tim",
  		"dad7.tim",
  		"dad8.tim",
  		"dad9.tim",
  		"dad10.tim",
  		"dad11.tim",
  		"dad12.tim",
  		"dad13.tim",
  		"dad14.tim",
  		"dad15.tim",
  		"dad16.tim",
  		"dad17.tim",
  		"dad18.tim",
  		"dad19.tim",
  		"dad20.tim",
  		"dad21.tim",
  		"dad22.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
