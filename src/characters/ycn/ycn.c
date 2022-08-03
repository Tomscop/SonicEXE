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
	YCN_ArcMain_Left2,
	YCN_ArcMain_Down0,
	YCN_ArcMain_Down1,
	YCN_ArcMain_Down2,
	YCN_ArcMain_Up0,
	YCN_ArcMain_Up1,
	YCN_ArcMain_Up2,
	YCN_ArcMain_Right0,
	YCN_ArcMain_Right1,
	YCN_ArcMain_Right2,
	YCN_ArcMain_Pixel,
	YCN_ArcMain_MadIdle0,
	YCN_ArcMain_MadIdle1,
	YCN_ArcMain_MadLeft0,
	YCN_ArcMain_MadLeft2,
	YCN_ArcMain_MadDown0,
	YCN_ArcMain_MadDown2,
	YCN_ArcMain_MadUp0,
	YCN_ArcMain_MadUp2,
	YCN_ArcMain_MadRight0,
	YCN_ArcMain_MadRight2,
	YCN_ArcMain_Laugh0,
	YCN_ArcMain_Laugh1,
	YCN_ArcMain_Laugh2,
	YCN_ArcMain_Scream0,
	YCN_ArcMain_Scream1,
	YCN_ArcMain_Scream2,
	YCN_ArcMain_Scream3,
	YCN_ArcMain_Scream4,
	YCN_ArcMain_Scream5,
	YCN_ArcMain_JustDie0,
	YCN_ArcMain_JustDie1,
	YCN_ArcMain_JustDie2,
	YCN_ArcMain_JustDie3,
	YCN_ArcMain_JustDie4,
	YCN_ArcMain_JustDie5,
	YCN_ArcMain_JustDie6,
	
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
	{YCN_ArcMain_Idle0, {  0,   0, 127, 184}, { 55, 175}}, //0
	{YCN_ArcMain_Idle0, {128,   0, 126, 184}, { 55, 175}}, //1
	{YCN_ArcMain_Idle1, {  0,   0, 126, 184}, { 55, 175}}, //2
	{YCN_ArcMain_Idle1, {127,   0, 127, 184}, { 55, 175}}, //3
	
	{YCN_ArcMain_Left0, {  0,   0,  168, 170}, { 110, 161}}, //4
	{YCN_ArcMain_Left1, {  0,   0,  168, 170}, { 102, 162}}, //5
	{YCN_ArcMain_Left2, {  0,   0,  172, 171}, { 105, 163}}, //6
	
	{YCN_ArcMain_Down0, {  0,   0, 142, 155}, { 74, 153}}, //7
	{YCN_ArcMain_Down1, {  0,   0, 145, 159}, { 73, 157}}, //8
	{YCN_ArcMain_Down2, {  0,   0, 141, 158}, { 72, 156}}, //9
	
	{YCN_ArcMain_Up0, {  0,   0, 127, 192}, { 60, 192}}, //10
	{YCN_ArcMain_Up1, {  0,   0, 132, 191}, { 62, 191}}, //11
	{YCN_ArcMain_Up2, {  0,   0, 132, 185}, { 62, 185}}, //12
	
	{YCN_ArcMain_Right0, {  0,   0, 179, 153}, { 37, 149}}, //13
	{YCN_ArcMain_Right1, {  0,   0, 160, 154}, { 40, 150}}, //14
	{YCN_ArcMain_Right2, {  0,   0, 170, 154}, { 39, 150}}, //15
	
	{YCN_ArcMain_Right0, {  0,   0, 117, 199}, { 43, 189}}, //16
	{YCN_ArcMain_Right1, {  0,   0, 114, 199}, { 42, 189}}, //17
	{YCN_ArcMain_Right2, {  0,   0, 117, 199}, { 43, 189}}, //18
};

static const Animation char_ycn_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5,  6, ASCR_BACK}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, 9, ASCR_BACK}},         //CharAnim_Down
	{2, (const u8[]){ 16,  17, 18, ASCR_BACK}},         //CharAnim_DownAlt
	{2, (const u8[]){ 10,  11, 12, ASCR_BACK}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){13, 14, 15, ASCR_BACK}},         //CharAnim_Right
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
		"left2.tim",  //YCN_ArcMain_Left1
		"down0.tim",  //YCN_ArcMain_Down0
		"down1.tim",  //YCN_ArcMain_Down1
		"down2.tim",  //YCN_ArcMain_Down1
		"up0.tim",    //YCN_ArcMain_Up0
		"up1.tim",    //YCN_ArcMain_Up1
		"up2.tim",    //YCN_ArcMain_Up1
		"right0.tim", //YCN_ArcMain_Right0
		"right1.tim", //YCN_ArcMain_Right1
		"right2.tim", //YCN_ArcMain_Right1
		"pixel.tim", //YCN_ArcMain_Right1
		"midle0.tim", //YCN_ArcMain_Idle0
		"midle1.tim", //YCN_ArcMain_Idle1
		"mleft0.tim",  //YCN_ArcMain_Left0
		"mleft2.tim",  //YCN_ArcMain_Left1
		"mdown0.tim",  //YCN_ArcMain_Down0
		"mdown2.tim",  //YCN_ArcMain_Down1
		"mup0.tim",    //YCN_ArcMain_Up0
		"mup2.tim",    //YCN_ArcMain_Up1
		"mright0.tim", //YCN_ArcMain_Right0
		"mright2.tim", //YCN_ArcMain_Right1
		"laugh0.tim", //YCN_ArcMain_Laugh0
		"laugh1.tim", //YCN_ArcMain_Laugh1
		"laugh2.tim", //YCN_ArcMain_Laugh1
		"scream0.tim", //YCN_ArcMain_Laugh0
		"scream1.tim", //YCN_ArcMain_Laugh1
		"scream2.tim", //YCN_ArcMain_Laugh1
		"scream3.tim", //YCN_ArcMain_Laugh0
		"scream4.tim", //YCN_ArcMain_Laugh1
		"scream5.tim", //YCN_ArcMain_Laugh1
		"justdie0.tim", //YCN_ArcMain_Laugh0
		"justdie1.tim", //YCN_ArcMain_Laugh1
		"justdie2.tim", //YCN_ArcMain_Laugh1
		"justdie3.tim", //YCN_ArcMain_Laugh0
		"justdie4.tim", //YCN_ArcMain_Laugh1
		"justdie5.tim", //YCN_ArcMain_Laugh1
		"justdie6.tim", //YCN_ArcMain_Laugh1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
