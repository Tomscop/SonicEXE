#include "xenophanes.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//Xenophanes character structure
enum
{
  	Dad_ArcMain_Dad0,
  	Dad_ArcMain_Dad1,
  	Dad_ArcMain_Dad2,
  	Dad_ArcMain_Dad3,
  	Dad_ArcMain_Dad4,
	Dad_ArcMain_Dad5,
  	Dad_ArcMain_Dad6,
  	Dad_ArcMain_Dad7,
  	Dad_ArcMain_Dad8,
  	Dad_ArcMain_Dad9,
  	Dad_ArcMain_Dad10,
  	Dad_ArcMain_Dad11,
  	Dad_ArcMain_Dad12,
  	Dad_ArcMain_Dad13,
  	Dad_ArcMain_Dad14,
  	Dad_ArcMain_Dad15,
  	Dad_ArcMain_Dad16,
  	Dad_ArcMain_Dad17,
  	Dad_ArcMain_Dad18,
  	Dad_ArcMain_Dad19,
  	Dad_ArcMain_Dad20,
  	Dad_ArcMain_Dad21,
  	Dad_ArcMain_Dad22,
  	Dad_ArcMain_Dad23,
	
	Dad_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Dad_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Dad;

//Dad character definitions
static const CharFrame char_dad_frame[] = {
  	{Dad_ArcMain_Dad0, {  2,  0,129,217}, { 86, 84}}, //0 idle 1
  	{Dad_ArcMain_Dad1, {  0,  0,128,216}, { 86, 82}}, //1 idle 2
  	{Dad_ArcMain_Dad2, {  0,  0,127,217}, { 86, 83}}, //2 idle 3
  	{Dad_ArcMain_Dad3, {  0,  0,127,218}, { 87, 84}}, //3 idle 4
  	{Dad_ArcMain_Dad4, {  0,  0,130,219}, { 90, 85}}, //4 idle 5
  	{Dad_ArcMain_Dad5, {  0,  0,128,223}, { 89, 89}}, //5 idle 6
  	{Dad_ArcMain_Dad6, {  0,  0,128,223}, { 89, 89}}, //6 idle 7
  	{Dad_ArcMain_Dad7, {  0,  0,128,223}, { 89, 89}}, //7 idle 8

  	{Dad_ArcMain_Dad8, {  0,  0,206,203}, {129, 68}}, //8 left 1
  	{Dad_ArcMain_Dad9, {  0,  0,202,203}, {123, 68}}, //9 left 2
  	{Dad_ArcMain_Dad10, {  0,  0,194,197}, {105, 62}}, //10 left 3
  	{Dad_ArcMain_Dad11, {  0,  0,199,199}, {110, 64}}, //11 left 4

  	{Dad_ArcMain_Dad12, {  0,  0,146,191}, { 95, 57}}, //12 down 1
  	{Dad_ArcMain_Dad13, {  0,  0,146,203}, { 95, 68}}, //13 down 2
  	{Dad_ArcMain_Dad14, {  0,  0,146,199}, { 95, 65}}, //14 down 3
  	{Dad_ArcMain_Dad15, {  0,  0,145,200}, { 94, 65}}, //15 down 4

  	{Dad_ArcMain_Dad16, {  0,  0,149,242}, {100,105}}, //16 up 1
  	{Dad_ArcMain_Dad17, {  0,  0,149,239}, {100,102}}, //17 up 2
  	{Dad_ArcMain_Dad18, {  0,  0,149,228}, {100, 92}}, //18 up 3
 	{Dad_ArcMain_Dad19, {  0,  0,149,230}, {100, 94}}, //19 up 4

  	{Dad_ArcMain_Dad20, {  0,  0,179,207}, { 44, 74}}, //20 right 1
  	{Dad_ArcMain_Dad21, {  0,  0,178,208}, { 44, 75}}, //21 right 2
  	{Dad_ArcMain_Dad22, {  0,  0,174,207}, { 44, 74}}, //22 right 3
  	{Dad_ArcMain_Dad23, {  0,  0,174,208}, { 44, 75}}, //23 right 4
};

static const Animation char_dad_anim[CharAnim_Max] = {
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

//Dad character functions
void Char_Dad_SetFrame(void *user, u8 frame)
{
	Char_Dad *this = (Char_Dad*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_dad_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Dad_Tick(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Dad_SetFrame);
	Character_Draw(character, &this->tex, &char_dad_frame[this->frame]);
}

void Char_Dad_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Dad_Free(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Dad_New(fixed_t x, fixed_t y)
{
	//Allocate Dad object
	Char_Dad *this = Mem_Alloc(sizeof(Char_Dad));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Dad_New] Failed to allocate dad object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Dad_Tick;
	this->character.set_anim = Char_Dad_SetAnim;
	this->character.free = Char_Dad_Free;
	
	Animatable_Init(&this->character.animatable, char_dad_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(11,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\DAD.ARC;1");
	
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
  		"dad23.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
