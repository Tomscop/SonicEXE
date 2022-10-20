#include "majinog.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//Majin OG character structure
enum
{
	MajinOG_ArcMain_Idle0,
	MajinOG_ArcMain_Idle1,
	MajinOG_ArcMain_Idle2,
	MajinOG_ArcMain_Idle3,
	MajinOG_ArcMain_Idle4,
	MajinOG_ArcMain_Idle5,
	MajinOG_ArcMain_Left0,
	MajinOG_ArcMain_Left1,
	MajinOG_ArcMain_Left2,
	MajinOG_ArcMain_Left3,
	MajinOG_ArcMain_Left4,
	MajinOG_ArcMain_Down0,
	MajinOG_ArcMain_Down1,
	MajinOG_ArcMain_Down2,
	MajinOG_ArcMain_Down3,
	MajinOG_ArcMain_Down4,
	MajinOG_ArcMain_Up0,
	MajinOG_ArcMain_Up1,
	MajinOG_ArcMain_Up2,
	MajinOG_ArcMain_Up3,
	MajinOG_ArcMain_Up4,
	MajinOG_ArcMain_Right0,
	MajinOG_ArcMain_Right1,
	MajinOG_ArcMain_Right2,
	MajinOG_ArcMain_Right3,
	MajinOG_ArcMain_Right4,
	
	MajinOG_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[MajinOG_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_MajinOG;

//Majin OG character definitions
static const CharFrame char_majinog_frame[] = {
	{MajinOG_ArcMain_Idle0, {  0,  0,255,187}, { 63,163}}, //0 idle 1
    {MajinOG_ArcMain_Idle1, {  0,  0,255,187}, { 63,163}}, //1 idle 2
    {MajinOG_ArcMain_Idle2, {  0,  0,255,187}, { 63,163}}, //2 idle 3
    {MajinOG_ArcMain_Idle3, {  0,  0,255,187}, { 63,163}}, //3 idle 4
    {MajinOG_ArcMain_Idle4, {  0,  0,255,187}, { 63,163}}, //4 idle 5
    {MajinOG_ArcMain_Idle5, {  0,  0,255,187}, { 63,163}}, //5 idle 6
    
    {MajinOG_ArcMain_Left0, {  0,  0,255,187}, { 63,163}}, //6 left 1
    {MajinOG_ArcMain_Left1, {  0,  0,255,187}, { 63,163}}, //7 left 2
    {MajinOG_ArcMain_Left2, {  0,  0,255,187}, { 63,163}}, //8 left 3
    {MajinOG_ArcMain_Left3, {  0,  0,255,187}, { 63,163}}, //9 left 4
    {MajinOG_ArcMain_Left4, {  0,  0,255,187}, { 63,163}}, //10 left 5
    
    {MajinOG_ArcMain_Down0, {  0,  0,255,187}, { 63,163}}, //11 down 1
    {MajinOG_ArcMain_Down1, {  0,  0,255,187}, { 63,163}}, //12 down 2
    {MajinOG_ArcMain_Down2, {  0,  0,255,187}, { 63,163}}, //13 down 3
    {MajinOG_ArcMain_Down3, {  0,  0,255,187}, { 63,163}}, //14 down 4
    {MajinOG_ArcMain_Down4, {  0,  0,255,187}, { 63,163}}, //15 down 5
    
    {MajinOG_ArcMain_Up0, {  0,  0,255,187}, { 63,163}}, //16 up 1
    {MajinOG_ArcMain_Up1, {  0,  0,255,187}, { 63,163}}, //17 up 2
    {MajinOG_ArcMain_Up2, {  0,  0,255,187}, { 63,163}}, //18 up 3
    {MajinOG_ArcMain_Up3, {  0,  0,255,187}, { 63,163}}, //19 up 4
    {MajinOG_ArcMain_Up4, {  0,  0,255,187}, { 63,163}}, //20 up 5
    
    {MajinOG_ArcMain_Right0, {  0,  0,255,187}, { 63,163}}, //21 up 1
    {MajinOG_ArcMain_Right1, {  0,  0,255,187}, { 60,163}}, //22 up 2
    {MajinOG_ArcMain_Right2, {  0,  0,255,187}, { 63,163}}, //23 up 3
    {MajinOG_ArcMain_Right3, {  0,  0,255,187}, { 63,163}}, //24 up 4
    {MajinOG_ArcMain_Right4, {  0,  0,255,187}, { 63,163}}, //25 up 5
};

static const Animation char_majinog_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 6, 7, 8, 9, 10, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 11, 12, 13, 14, 15, ASCR_BACK, 0}},         //CharAnim_Down
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_DownAlt
	{2, (const u8[]){ 16, 17, 18, 19, 20, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){21, 22, 23, 24, 25, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Majin OG character functions
void Char_MajinOG_SetFrame(void *user, u8 frame)
{
	Char_MajinOG *this = (Char_MajinOG*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_majinog_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_MajinOG_Tick(Character *character)
{
	Char_MajinOG *this = (Char_MajinOG*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_MajinOG_SetFrame);
	Character_Draw(character, &this->tex, &char_majinog_frame[this->frame]);
}

void Char_MajinOG_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_MajinOG_Free(Character *character)
{
	Char_MajinOG *this = (Char_MajinOG*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_MajinOG_New(fixed_t x, fixed_t y)
{
	//Allocate majinog object
	Char_MajinOG *this = Mem_Alloc(sizeof(Char_MajinOG));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_MajinOG_New] Failed to allocate majinog object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_MajinOG_Tick;
	this->character.set_anim = Char_MajinOG_SetAnim;
	this->character.free = Char_MajinOG_Free;
	
	Animatable_Init(&this->character.animatable, char_majinog_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MAJINOG.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //MajinOG_ArcMain_Idle0
		"idle1.tim", //MajinOG_ArcMain_Idle1
		"idle2.tim", //MajinOG_ArcMain_Idle2
		"idle3.tim", //MajinOG_ArcMain_Idle3
		"idle4.tim", //MajinOG_ArcMain_Idle4
		"idle5.tim", //MajinOG_ArcMain_Idle5
		
		"left0.tim", //MajinOG_ArcMain_Left0
		"left1.tim", //MajinOG_ArcMain_Left1
		"left2.tim", //MajinOG_ArcMain_Left2
		"left3.tim", //MajinOG_ArcMain_Left3
		"left4.tim", //MajinOG_ArcMain_Left4
		
		"down0.tim", //MajinOG_ArcMain_Down0
		"down1.tim", //MajinOG_ArcMain_Down1
		"down2.tim", //MajinOG_ArcMain_Down2
		"down3.tim", //MajinOG_ArcMain_Down3
		"down4.tim", //MajinOG_ArcMain_Down4
		
		"up0.tim", //MajinOG_ArcMain_Up0
		"up1.tim", //MajinOG_ArcMain_Up1
		"up2.tim", //MajinOG_ArcMain_Up2
		"up3.tim", //MajinOG_ArcMain_Up3
		"up4.tim", //MajinOG_ArcMain_Up4
		
		"right0.tim", //MajinOG_ArcMain_Right0
		"right1.tim", //MajinOG_ArcMain_Right1
		"right2.tim", //MajinOG_ArcMain_Right2
		"right3.tim", //MajinOG_ArcMain_Right3
		"right4.tim", //MajinOG_ArcMain_Right4
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
