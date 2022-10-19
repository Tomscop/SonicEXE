#include "nmouse.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//Needlemouse character structure
enum
{
	NMouse_ArcMain_Idle0,
	NMouse_ArcMain_Idle1,
	NMouse_ArcMain_Idle2,
	NMouse_ArcMain_Idle3,
	NMouse_ArcMain_Idle4,
	NMouse_ArcMain_Idle5,
	NMouse_ArcMain_Idle6,
	NMouse_ArcMain_Idle7,
	NMouse_ArcMain_Idle8,
	NMouse_ArcMain_Idle9,
	NMouse_ArcMain_Left0,
	NMouse_ArcMain_Left1,
	NMouse_ArcMain_Left2,
	NMouse_ArcMain_Left3,
	NMouse_ArcMain_Left4,
	NMouse_ArcMain_Down0,
	NMouse_ArcMain_Down1,
	NMouse_ArcMain_Down2,
	NMouse_ArcMain_Down3,
	NMouse_ArcMain_Down4,
	NMouse_ArcMain_Up0,
	NMouse_ArcMain_Up1,
	NMouse_ArcMain_Up2,
	NMouse_ArcMain_Up3,
	NMouse_ArcMain_Up4,
	NMouse_ArcMain_Right0,
	NMouse_ArcMain_Right1,
	NMouse_ArcMain_Right2,
	NMouse_ArcMain_Right3,
	NMouse_ArcMain_Right4,
	
	NMouse_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[NMouse_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_NMouse;

//Needlemouse character definitions
static const CharFrame char_nmouse_frame[] = {
	{NMouse_ArcMain_Idle0, {  0,  0,185,163}, { 63,163}}, //0 idle 1
	{NMouse_ArcMain_Idle1, {  0,  0,185,163}, { 63,163}}, //1 idle 2
	{NMouse_ArcMain_Idle2, {  0,  0,185,163}, { 63,163}}, //2 idle 3
	{NMouse_ArcMain_Idle3, {  0,  0,185,163}, { 63,163}}, //3 idle 4
	{NMouse_ArcMain_Idle4, {  0,  0,185,163}, { 63,163}}, //4 idle 5
	{NMouse_ArcMain_Idle5, {  0,  0,185,163}, { 63,163}}, //5 idle 6
	{NMouse_ArcMain_Idle6, {  0,  0,185,163}, { 63,163}}, //6 idle 7
	{NMouse_ArcMain_Idle7, {  0,  0,185,163}, { 63,163}}, //7 idle 8
	{NMouse_ArcMain_Idle8, {  0,  0,185,163}, { 63,163}}, //8 idle 9
	{NMouse_ArcMain_Idle9, {  0,  0,185,163}, { 63,163}}, //9 idle 10
	
	{NMouse_ArcMain_Left0, {  0,  0,185,163}, { 63,163}}, //10 left 1
	{NMouse_ArcMain_Left1, {  0,  0,185,163}, { 63,163}}, //11 left 2
	{NMouse_ArcMain_Left2, {  0,  0,185,163}, { 63,163}}, //12 left 3
	{NMouse_ArcMain_Left3, {  0,  0,185,163}, { 63,163}}, //13 left 4
	{NMouse_ArcMain_Left4, {  0,  0,185,163}, { 63,163}}, //14 left 5
	
	{NMouse_ArcMain_Down0, {  0,  0,185,163}, { 63,163}}, //15 down 1
	{NMouse_ArcMain_Down1, {  0,  0,185,163}, { 63,163}}, //16 down 2
	{NMouse_ArcMain_Down2, {  0,  0,185,163}, { 63,163}}, //17 down 3
	{NMouse_ArcMain_Down3, {  0,  0,185,163}, { 63,163}}, //18 down 4
	{NMouse_ArcMain_Down4, {  0,  0,185,163}, { 63,163}}, //19 down 5
	
	{NMouse_ArcMain_Up0, {  0,  0,185,163}, { 63,163}}, //20 up 1
	{NMouse_ArcMain_Up1, {  0,  0,185,163}, { 63,163}}, //21 up 2
	{NMouse_ArcMain_Up2, {  0,  0,185,163}, { 63,163}}, //22 up 3
	{NMouse_ArcMain_Up3, {  0,  0,185,163}, { 63,163}}, //23 up 4
	{NMouse_ArcMain_Up4, {  0,  0,185,163}, { 63,163}}, //24 up 5
	
	{NMouse_ArcMain_Right0, {  0,  0,185,163}, { 63,163}}, //25 right 1
	{NMouse_ArcMain_Right1, {  0,  0,185,163}, { 63,163}}, //26 right 2
	{NMouse_ArcMain_Right2, {  0,  0,185,163}, { 63,163}}, //27 right 3
	{NMouse_ArcMain_Right3, {  0,  0,185,163}, { 63,163}}, //28 right 4
	{NMouse_ArcMain_Right4, {  0,  0,185,163}, { 63,163}}, //29 right 5
};

static const Animation char_nmouse_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 10, 11, 12, 13, 14, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 15, 16, 17, 18, 19, ASCR_BACK, 0}},         //CharAnim_Down
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_DownAlt
	{2, (const u8[]){ 20, 21, 22, 23, 24, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){25, 26, 27, 28, 29, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Needlemouse character functions
void Char_NMouse_SetFrame(void *user, u8 frame)
{
	Char_NMouse *this = (Char_NMouse*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_nmouse_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_NMouse_Tick(Character *character)
{
	Char_NMouse *this = (Char_NMouse*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_NMouse_SetFrame);
	Character_Draw(character, &this->tex, &char_nmouse_frame[this->frame]);
}

void Char_NMouse_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_NMouse_Free(Character *character)
{
	Char_NMouse *this = (Char_NMouse*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_NMouse_New(fixed_t x, fixed_t y)
{
	//Allocate nmouse object
	Char_NMouse *this = Mem_Alloc(sizeof(Char_NMouse));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_NMouse_New] Failed to allocate nmouse object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_NMouse_Tick;
	this->character.set_anim = Char_NMouse_SetAnim;
	this->character.free = Char_NMouse_Free;
	
	Animatable_Init(&this->character.animatable, char_nmouse_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(12,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\NMOUSE.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",
		"idle1.tim",
		"idle2.tim",
		"idle3.tim",
		"idle4.tim",
		"idle5.tim",
		"idle6.tim",
		"idle7.tim",
		"idle8.tim",
		"idle9.tim",
		
		"left0.tim",
		"left1.tim",
		"left2.tim",
		"left3.tim",
		"left4.tim",
		
		"down0.tim",
		"down1.tim",
		"down2.tim",
		"down3.tim",
		"down4.tim",
		
		"up0.tim",
		"up1.tim",
		"up2.tim",
		"up3.tim",
		"up4.tim",
		
		"right0.tim",
		"right1.tim",
		"right2.tim",
		"right3.tim",
		"right4.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
