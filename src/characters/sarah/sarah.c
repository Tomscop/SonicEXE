#include "sarah.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//Sarah character structure
enum
{
	Sarah_ArcMain_Idle0,
	Sarah_ArcMain_Idle1,
	Sarah_ArcMain_Idle2,
	Sarah_ArcMain_Idle3,
	Sarah_ArcMain_Idle4,
	Sarah_ArcMain_Left0,
	Sarah_ArcMain_Left1,
	Sarah_ArcMain_Left2,
	Sarah_ArcMain_Left3,
	Sarah_ArcMain_Left4,
	Sarah_ArcMain_Down0,
	Sarah_ArcMain_Down1,
	Sarah_ArcMain_Down2,
	Sarah_ArcMain_Down3,
	Sarah_ArcMain_Down4,
	Sarah_ArcMain_Up0,
	Sarah_ArcMain_Up1,
	Sarah_ArcMain_Up2,
	Sarah_ArcMain_Up3,
	Sarah_ArcMain_Up4,
	Sarah_ArcMain_Right0,
	Sarah_ArcMain_Right1,
	Sarah_ArcMain_Right2,
	Sarah_ArcMain_Right3,
	Sarah_ArcMain_Right4,
	
	Sarah_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Sarah_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Sarah;

//Sarah character definitions
static const CharFrame char_sarah_frame[] = {
	{Sarah_ArcMain_Idle0, {  0,  0, 89,160}, { 63,163}}, //0 idle 1
	{Sarah_ArcMain_Idle0, { 90,  0, 89,160}, { 63,163}}, //1 idle 2
	{Sarah_ArcMain_Idle1, {  0,  0, 88,156}, { 63,163}}, //2 idle 3
	{Sarah_ArcMain_Idle1, { 90,  0, 91,156}, { 63,163}}, //3 idle 4
	{Sarah_ArcMain_Idle2, {  0,  0, 92,157}, { 63,163}}, //4 idle 5
	{Sarah_ArcMain_Idle2, { 93,  0, 95,157}, { 63,163}}, //5 idle 6
	{Sarah_ArcMain_Idle3, {  0,  0, 94,156}, { 63,163}}, //6 idle 7
	{Sarah_ArcMain_Idle3, { 95,  0, 92,156}, { 63,163}}, //7 idle 8
	{Sarah_ArcMain_Idle4, {  0,  0, 92,153}, { 63,163}}, //8 idle 9
	{Sarah_ArcMain_Idle4, { 93,  0, 93,153}, { 63,163}}, //9 idle 10
	
	{Sarah_ArcMain_Left0, {  0,  0,148,121}, { 63,163}}, //10 left 1
	{Sarah_ArcMain_Left1, {  0,  0,150,121}, { 63,163}}, //11 left 2
	{Sarah_ArcMain_Left2, {  0,  0,143,122}, { 63,163}}, //12 left 3
	{Sarah_ArcMain_Left3, {  0,  0,135,125}, { 63,163}}, //13 left 4
	{Sarah_ArcMain_Left4, {  0,  0,137,125}, { 63,163}}, //14 left 5
	
	{Sarah_ArcMain_Down0, {  0,  0,167,178}, { 63,163}}, //15 down 1
	{Sarah_ArcMain_Down1, {  0,  0,167,178}, { 63,163}}, //16 down 2
	{Sarah_ArcMain_Down2, {  0,  0,167,178}, { 63,163}}, //17 down 3
	{Sarah_ArcMain_Down3, {  0,  0,167,178}, { 63,163}}, //18 down 4
	{Sarah_ArcMain_Down4, {  0,  0,167,178}, { 63,163}}, //19 down 5
	
	{Sarah_ArcMain_Up0, {  0,  0,167,178}, { 63,163}}, //20 up 1
	{Sarah_ArcMain_Up1, {  0,  0,167,178}, { 63,163}}, //21 up 2
	{Sarah_ArcMain_Up2, {  0,  0,167,178}, { 63,163}}, //22 up 3
	{Sarah_ArcMain_Up3, {  0,  0,167,178}, { 63,163}}, //23 up 4
	{Sarah_ArcMain_Up4, {  0,  0,167,178}, { 63,163}}, //24 up 5
	
	{Sarah_ArcMain_Right0, {  0,  0,123,118}, { 63,163}}, //25 right 1
	{Sarah_ArcMain_Right1, {  0,  0,121,118}, { 63,163}}, //26 right 2
	{Sarah_ArcMain_Right2, {  0,  0,129,125}, { 63,163}}, //27 right 3
	{Sarah_ArcMain_Right3, {  0,  0,137,130}, { 63,163}}, //28 right 4
	{Sarah_ArcMain_Right4, {  0,  0,136,130}, { 63,163}}, //29 right 5
};

static const Animation char_sarah_anim[CharAnim_Max] = {
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

//Sarah character functions
void Char_Sarah_SetFrame(void *user, u8 frame)
{
	Char_Sarah *this = (Char_Sarah*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_sarah_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Sarah_Tick(Character *character)
{
	Char_Sarah *this = (Char_Sarah*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Sarah_SetFrame);
	Character_Draw(character, &this->tex, &char_sarah_frame[this->frame]);
}

void Char_Sarah_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Sarah_Free(Character *character)
{
	Char_Sarah *this = (Char_Sarah*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Sarah_New(fixed_t x, fixed_t y)
{
	//Allocate sarah object
	Char_Sarah *this = Mem_Alloc(sizeof(Char_Sarah));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Sarah_New] Failed to allocate sarah object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Sarah_Tick;
	this->character.set_anim = Char_Sarah_SetAnim;
	this->character.free = Char_Sarah_Free;
	
	Animatable_Init(&this->character.animatable, char_sarah_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(12,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SARAH.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Sarah_ArcMain_Idle0
		"idle1.tim", //Sarah_ArcMain_Idle1
		"idle2.tim", //Sarah_ArcMain_Idle2
		"idle3.tim", //Sarah_ArcMain_Idle3
		"idle4.tim", //Sarah_ArcMain_Idle4
		
		"left0.tim", //Sarah_ArcMain_Left0
		"left1.tim", //Sarah_ArcMain_Left1
		"left2.tim", //Sarah_ArcMain_Left2
		"left3.tim", //Sarah_ArcMain_Left3
		"left4.tim", //Sarah_ArcMain_Left4
		
		"down0.tim", //Sarah_ArcMain_Down0
		"down1.tim", //Sarah_ArcMain_Down1
		"down2.tim", //Sarah_ArcMain_Down2
		"down3.tim", //Sarah_ArcMain_Down3
		"down4.tim", //Sarah_ArcMain_Down4
		
		"up0.tim", //Sarah_ArcMain_Up0
		"up1.tim", //Sarah_ArcMain_Up1
		"up2.tim", //Sarah_ArcMain_Up2
		"up3.tim", //Sarah_ArcMain_Up3
		"up4.tim", //Sarah_ArcMain_Up4
		
		"right0.tim", //Sarah_ArcMain_Right0
		"right1.tim", //Sarah_ArcMain_Right1
		"right2.tim", //Sarah_ArcMain_Right2
		"right3.tim", //Sarah_ArcMain_Right3
		"right4.tim", //Sarah_ArcMain_Right4
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}