#include "seggm.h"

#include "../../psx/mem.h"
#include "../../psx/archive.h"
#include "../../stage/stage.h"
#include "../../main.h"

//seggm character structure
enum
{
	seggm_ArcMain_idle,
	seggm_ArcMain_left,
	seggm_ArcMain_down,
	seggm_ArcMain_up0,
	seggm_ArcMain_up1,
	seggm_ArcMain_right,
	
	seggm_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[seggm_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_seggm;

//seggm character definitions
static const CharFrame char_seggm_frame[] = {
 	{seggm_ArcMain_idle,  {  0,   0,  70, 134}, {0, 134}},
	{seggm_ArcMain_idle,  { 70,   0,  70, 134}, {0, 134}},
	{seggm_ArcMain_left,  {  0,   0, 109, 118}, {0, 118}},
	{seggm_ArcMain_left,  {109,   0, 107, 118}, {0, 118}},
	{seggm_ArcMain_left,  {  0, 118, 106, 119}, {0, 119}},
	{seggm_ArcMain_left,  {106, 118, 104, 118}, {0, 118}},
	{seggm_ArcMain_down,  {  0,   0, 102, 110}, {0, 110}},
	{seggm_ArcMain_down,  {102,   0, 102, 114}, {0, 114}},
	{seggm_ArcMain_down,  {  0, 116, 101, 114}, {0, 114}},
	{seggm_ArcMain_down,  {101, 116, 101, 114}, {0, 114}},
	{seggm_ArcMain_up0,   {  0,   0, 100, 165}, {0, 165}},
	{seggm_ArcMain_up0,   {100,   0,  97, 165}, {0, 165}},
	{seggm_ArcMain_up1,   {  0,   0,  98, 159}, {0, 159}},
	{seggm_ArcMain_up1,   { 98,   0, 100, 156}, {0, 156}},
	{seggm_ArcMain_right, {  0,   0,  92, 111}, {0, 111}},
	{seggm_ArcMain_right, { 92,   0,  91, 112}, {0, 112}},
	{seggm_ArcMain_right, {  0, 112,  90, 112}, {0, 112}},
	{seggm_ArcMain_right, { 90, 112,  88, 111}, {0, 111}}
};

static const Animation char_seggm_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 2, 3, 4, 5, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6, 7, 8, 9, ASCR_BACK, 0}},         //CharAnim_Down
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_DownAlt
	{2, (const u8[]){ 10, 11, 12, 13, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){14, 15, 16, 17, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//seggm character functions
void Char_seggm_SetFrame(void *user, u8 frame)
{
	Char_seggm *this = (Char_seggm*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_seggm_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_seggm_Tick(Character *character)
{
	Char_seggm *this = (Char_seggm*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_seggm_SetFrame);
	Character_Draw(character, &this->tex, &char_seggm_frame[this->frame]);
}

void Char_seggm_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_seggm_Free(Character *character)
{
	Char_seggm *this = (Char_seggm*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_seggm_New(fixed_t x, fixed_t y)
{
	//Allocate seggm object
	Char_seggm *this = Mem_Alloc(sizeof(Char_seggm));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_seggm_New] Failed to allocate seggm object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_seggm_Tick;
	this->character.set_anim = Char_seggm_SetAnim;
	this->character.free = Char_seggm_Free;
	
	Animatable_Init(&this->character.animatable, char_seggm_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(11,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SEGGM.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim",
		"left.tim",
		"down.tim",
		"up0.tim",
		"up1.tim",
		"right.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
