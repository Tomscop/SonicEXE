#include "ycn.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//YCN character structure
enum
{
	YCN_ArcMain_Idle0,
	YCN_ArcMain_Idle1,
	YCN_ArcMain_Left0,
	YCN_ArcMain_Left1,
	YCN_ArcMain_Down0,
	YCN_ArcMain_Down1,
	YCN_ArcMain_Up0,
	YCN_ArcMain_Up1,
	YCN_ArcMain_Right0,
	YCN_ArcMain_Right1,
	YCN_ArcMain_Laugh0,
	YCN_ArcMain_Laugh1,
	
	YCN_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[YCN_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_YCN;

//YCN character definitions
static const CharFrame char_ycn_frame[] = {
	{YCN_ArcMain_Idle0, {  0,   0, 106, 165}, { 53, 165}}, //0
	{YCN_ArcMain_Idle0, {107,   0, 106, 165}, { 53, 165}}, //1
	{YCN_ArcMain_Idle1, {  0,   0, 105, 163}, { 52, 165}}, //2
	{YCN_ArcMain_Idle1, {106,   0, 105, 163}, { 52, 165}}, //3
	{YCN_ArcMain_Idle1, {  0,   0, 106, 164}, { 53, 165}}, //4
	
	{YCN_ArcMain_Left0, {  0,   0,  130, 170}, { 80, 170}}, //4
	{YCN_ArcMain_Left0, {131,   6,  121, 164}, { 70, 164}}, //5
	{YCN_ArcMain_Left1, {  0,   0,  122, 163}, { 70, 163}}, //4
	{YCN_ArcMain_Left1, {123,   1,  122, 162}, { 70, 162}}, //5
	
	{YCN_ArcMain_Down0, {  0,   0, 129, 133}, { 59, 133}}, //6
	{YCN_ArcMain_Down0, {  0,   0, 137, 142}, { 67, 141}}, //7
	{YCN_ArcMain_Down1, {  0,   0, 138, 144}, { 67, 143}}, //6
	{YCN_ArcMain_Down1, {  0,   0, 137, 144}, { 67, 143}}, //7
	
	{YCN_ArcMain_Up0, {  0,   0, 150, 191}, { 71, 191}}, //8
	{YCN_ArcMain_Up0, {  0,   0, 152, 183}, { 72, 183}}, //9
	{YCN_ArcMain_Up1, {  0,   0, 154, 182}, { 73, 182}}, //8
	{YCN_ArcMain_Up1, {  0,   0, 153, 182}, { 74, 182}}, //9
	
	{YCN_ArcMain_Right0, {  4,   0, 127, 177}, { 43, 177}}, //10
	{YCN_ArcMain_Right0, {132,   0, 116, 177}, { 42, 177}}, //11
	{YCN_ArcMain_Right1, {  0,   0, 116, 179}, { 43, 179}}, //10
	{YCN_ArcMain_Right1, {117,   0, 115, 179}, { 42, 179}}, //11
	
	{YCN_ArcMain_Laugh0, {  0,   0, 117, 199}, { 43, 189}}, //10
	{YCN_ArcMain_Laugh0, {118,   0, 114, 199}, { 42, 189}}, //11
	{YCN_ArcMain_Laugh1, {  0,   0, 117, 199}, { 43, 189}}, //10
	{YCN_ArcMain_Laugh1, {118,   0, 114, 199}, { 42, 189}}, //11
};

static const Animation char_ycn_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 5,  6,  7,  8, ASCR_BACK}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 9,  10, 11, 12, ASCR_BACK}},         //CharAnim_Down
	{2, (const u8[]){ 21,  22, 23, 24, ASCR_BACK}},         //CharAnim_DownAlt
	{2, (const u8[]){ 13,  14, 15, 16, ASCR_BACK}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){17, 18, 19, 20,ASCR_BACK}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//YCN character functions
void Char_YCN_SetFrame(void *user, u8 frame)
{
	Char_YCN *this = (Char_YCN*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_ycn_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_YCN_Tick(Character *character)
{
	Char_YCN *this = (Char_YCN*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_YCN_SetFrame);
	Character_Draw(character, &this->tex, &char_ycn_frame[this->frame]);
}

void Char_YCN_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_YCN_Free(Character *character)
{
	Char_YCN *this = (Char_YCN*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_YCN_New(fixed_t x, fixed_t y)
{
	//Allocate ycn object
	Char_YCN *this = Mem_Alloc(sizeof(Char_YCN));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_YCN_New] Failed to allocate ycn object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_YCN_Tick;
	this->character.set_anim = Char_YCN_SetAnim;
	this->character.free = Char_YCN_Free;
	
	Animatable_Init(&this->character.animatable, char_ycn_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 2;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\YCN.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //YCN_ArcMain_Idle0
		"idle1.tim", //YCN_ArcMain_Idle1
		"left0.tim",  //YCN_ArcMain_Left0
		"left1.tim",  //YCN_ArcMain_Left1
		"down0.tim",  //YCN_ArcMain_Down0
		"down1.tim",  //YCN_ArcMain_Down1
		"up0.tim",    //YCN_ArcMain_Up0
		"up1.tim",    //YCN_ArcMain_Up1
		"right0.tim", //YCN_ArcMain_Right0
		"right1.tim", //YCN_ArcMain_Right1
		"laugh0.tim", //YCN_ArcMain_Laugh0
		"laugh1.tim", //YCN_ArcMain_Laugh1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
