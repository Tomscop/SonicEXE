#include "lordx.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//LordX character structure
enum
{
	LordX_ArcMain_Idle0,
	LordX_ArcMain_Idle1,
	LordX_ArcMain_Idle2,
	LordX_ArcMain_Idle3,
	LordX_ArcMain_Idle4,
	LordX_ArcMain_Idle5,
	LordX_ArcMain_Left0,
	LordX_ArcMain_Left1,
	LordX_ArcMain_Left2,
	LordX_ArcMain_Left3,
	LordX_ArcMain_Down0,
	LordX_ArcMain_Down1,
	LordX_ArcMain_Down2,
	LordX_ArcMain_Down3,
	LordX_ArcMain_Up0,
	LordX_ArcMain_Up1,
	LordX_ArcMain_Up2,
	LordX_ArcMain_Up3,
	LordX_ArcMain_Right0,
	LordX_ArcMain_Right1,
	LordX_ArcMain_Right2,
	LordX_ArcMain_Right3,
	
	LordX_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[LordX_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_LordX;

//LordX character definitions
static const CharFrame char_lordx_frame[] = {
	{LordX_ArcMain_Idle0, {  0,   0, 134, 168}, { 63, 163}}, //0
	{LordX_ArcMain_Idle1, {  0,   0, 134, 168}, { 63, 163}}, //1
	{LordX_ArcMain_Idle2, {  0,   0, 134, 168}, { 63, 163}}, //2
	{LordX_ArcMain_Idle3, {  0,   0, 134, 168}, { 63, 163}}, //3
	{LordX_ArcMain_Idle4, {  0,   0, 134, 168}, { 63, 163}}, //4
	{LordX_ArcMain_Idle5, {  0,   0, 134, 168}, { 63, 163}}, //5
	
	{LordX_ArcMain_Left0, {  0,   0, 134, 168}, { 63, 163}}, //6
	{LordX_ArcMain_Left1, {  0,   0, 134, 168}, { 63, 163}}, //7
	{LordX_ArcMain_Left2, {  0,   0, 134, 168}, { 63, 163}}, //8
	{LordX_ArcMain_Left3, {  0,   0, 134, 168}, { 63, 163}}, //9
	
	{LordX_ArcMain_Down0, {  0,   0, 134, 168}, { 63, 163}}, //10
	{LordX_ArcMain_Down1, {  0,   0, 134, 168}, { 63, 163}}, //11
	{LordX_ArcMain_Down2, {  0,   0, 134, 168}, { 63, 163}}, //12
	{LordX_ArcMain_Down3, {  0,   0, 134, 168}, { 63, 163}}, //13
	
	{LordX_ArcMain_Up0, {  0,   0, 134, 168}, { 63, 163}}, //14
	{LordX_ArcMain_Up1, {  0,   0, 134, 168}, { 63, 163}}, //15
	{LordX_ArcMain_Up2, {  0,   0, 134, 168}, { 63, 163}}, //16
	{LordX_ArcMain_Up3, {  0,   0, 134, 168}, { 63, 163}}, //17
	
	{LordX_ArcMain_Right0, {  0,   0, 134, 168}, { 63, 163}}, //18
	{LordX_ArcMain_Right1, {  0,   0, 134, 168}, { 63, 163}}, //19
	{LordX_ArcMain_Right2, {  0,   0, 134, 168}, { 63, 163}}, //20
	{LordX_ArcMain_Right3, {  0,   0, 134, 168}, { 63, 163}}, //21
};

static const Animation char_lordx_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 6,  7,  8,  9, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 10,  11, 12, 13, ASCR_BACK, 0}},         //CharAnim_Down
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_DownAlt
	{2, (const u8[]){ 14,  15, 16, 17, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){18, 19, 20, 21, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//LordX character functions
void Char_LordX_SetFrame(void *user, u8 frame)
{
	Char_LordX *this = (Char_LordX*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_lordx_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_LordX_Tick(Character *character)
{
	Char_LordX *this = (Char_LordX*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_LordX_SetFrame);
	Character_Draw(character, &this->tex, &char_lordx_frame[this->frame]);
}

void Char_LordX_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_LordX_Free(Character *character)
{
	Char_LordX *this = (Char_LordX*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_LordX_New(fixed_t x, fixed_t y)
{
	//Allocate lordx object
	Char_LordX *this = Mem_Alloc(sizeof(Char_LordX));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_LordX_New] Failed to allocate lordx object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_LordX_Tick;
	this->character.set_anim = Char_LordX_SetAnim;
	this->character.free = Char_LordX_Free;
	
	Animatable_Init(&this->character.animatable, char_lordx_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(11,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\LORDX.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",
		"idle1.tim",
		"idle2.tim",
		"idle3.tim",
		"idle4.tim",
		"idle5.tim",
		
		"left0.tim",
		"left1.tim",
		"left2.tim",
		"left3.tim",
		
		"down0.tim",
		"down1.tim",
		"down2.tim",
		"down3.tim",
		
		"up0.tim",
		"up1.tim",
		"up2.tim",
		"up3.tim",
		
		"right0.tim",
		"right1.tim",
		"right2.tim",
		"right3.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
