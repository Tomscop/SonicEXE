#include "sanic.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//sanic character structure
enum
{
	sanic_ArcMain_Idle,
	sanic_ArcMain_Left,
	sanic_ArcMain_Down,
	sanic_ArcMain_Up,
	sanic_ArcMain_Right,
	
	sanic_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[sanic_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_sanic;

//sanic character definitions
static const CharFrame char_sanic_frame[] = {
	{sanic_ArcMain_Idle, {  1,   0, 120, 111}, { -4, -6}}, //0
	{sanic_ArcMain_Idle, {122,   1, 122, 110}, { -4, -7}}, //1
	{sanic_ArcMain_Idle, {  1, 112, 121, 110}, { -4, -7}}, //2
	{sanic_ArcMain_Idle, {123, 111, 120, 111}, { -5, -6}}, //3
	
	{sanic_ArcMain_Left, {  0,   0,  130, 170}, { 80, 170}}, //4
	{sanic_ArcMain_Left, {131,   6,  121, 164}, { 70, 164}}, //5
	{sanic_ArcMain_Left, {  0,   0,  122, 163}, { 70, 163}}, //6
	{sanic_ArcMain_Left, {123,   1,  122, 162}, { 70, 162}}, //7
	
	{sanic_ArcMain_Down, {  0,   0, 129, 133}, { 59, 133}}, //8
	{sanic_ArcMain_Down, {  0,   0, 137, 142}, { 67, 141}}, //9
	{sanic_ArcMain_Down, {  0,   0, 138, 144}, { 67, 143}}, //10
	{sanic_ArcMain_Down, {  0,   0, 137, 144}, { 67, 143}}, //11
	
	{sanic_ArcMain_Up, {  0,   0, 150, 191}, { 71, 191}}, //12
	{sanic_ArcMain_Up, {  0,   0, 152, 183}, { 72, 183}}, //13
	{sanic_ArcMain_Up, {  0,   0, 154, 182}, { 73, 182}}, //14
	{sanic_ArcMain_Up, {  0,   0, 153, 182}, { 74, 182}}, //15
	
	{sanic_ArcMain_Right, {  4,   0, 127, 177}, { 43, 177}}, //16
	{sanic_ArcMain_Right, {132,   0, 116, 177}, { 42, 177}}, //17
	{sanic_ArcMain_Right, {  0,   0, 116, 179}, { 43, 179}}, //18
	{sanic_ArcMain_Right, {117,   0, 115, 179}, { 42, 179}}, //19
};

static const Animation char_sanic_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5,  6,  7, ASCR_BACK}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9, 10, 11, ASCR_BACK}},         //CharAnim_Down
	{2, (const u8[]){ 20,  21, 22, 23, ASCR_BACK}},         //CharAnim_DownAlt
	{2, (const u8[]){ 12,  13, 14, 15, ASCR_BACK}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){16, 17, 18, 19,ASCR_BACK}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//sanic character functions
void Char_sanic_SetFrame(void *user, u8 frame)
{
	Char_sanic *this = (Char_sanic*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_sanic_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_sanic_Tick(Character *character)
{
	Char_sanic *this = (Char_sanic*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_sanic_SetFrame);
	Character_Draw(character, &this->tex, &char_sanic_frame[this->frame]);
}

void Char_sanic_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_sanic_Free(Character *character)
{
	Char_sanic *this = (Char_sanic*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_sanic_New(fixed_t x, fixed_t y)
{
	//Allocate sanic object
	Char_sanic *this = Mem_Alloc(sizeof(Char_sanic));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_sanic_New] Failed to allocate sanic object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_sanic_Tick;
	this->character.set_anim = Char_sanic_SetAnim;
	this->character.free = Char_sanic_Free;
	
	Animatable_Init(&this->character.animatable, char_sanic_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\sanic.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim", //sanic_ArcMain_Idle
		"left.tim",  //sanic_ArcMain_Left
		"down.tim",  //sanic_ArcMain_Down
		"up.tim",    //sanic_ArcMain_Up
		"right.tim", //sanic_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
