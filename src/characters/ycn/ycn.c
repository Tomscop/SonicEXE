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
	
	YCN_ArcMain_Laugh0,
	YCN_ArcMain_Laugh1,
	YCN_ArcMain_Laugh2,
	
	YCN_ArcMain_MIdle0,
	YCN_ArcMain_MIdle1,
	
	YCN_ArcMain_MLeft0,
	YCN_ArcMain_MLeft1,
	YCN_ArcMain_MLeft2,
	
	YCN_ArcMain_MDown0,
	YCN_ArcMain_MDown1,
	YCN_ArcMain_MDown2,
	
	YCN_ArcMain_MUp0,
	YCN_ArcMain_MUp1,
	YCN_ArcMain_MUp2,
	
	YCN_ArcMain_MRight0,
	YCN_ArcMain_MRight1,
	YCN_ArcMain_MRight2,
	
	YCN_ArcMain_Scream0,
	YCN_ArcMain_Scream1,
	YCN_ArcMain_Scream2,
	YCN_ArcMain_Scream3,
	
	YCN_ArcMain_JustDie0,
	YCN_ArcMain_JustDie1,
	YCN_ArcMain_JustDie2,
	YCN_ArcMain_JustDie3,
	YCN_ArcMain_JustDie4,
	YCN_ArcMain_JustDie5,
	
	YCN_ArcMain_Pixel,
	
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

u8 phases;

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
	
	{YCN_ArcMain_Laugh0, {  0,   0, 142, 149}, { 69, 145}}, //16
	{YCN_ArcMain_Laugh1, {  0,   0, 139, 147}, { 66, 142}}, //17
	{YCN_ArcMain_Laugh2, {  0,   0, 135, 148}, { 64, 143}}, //18
};

static const Animation char_ycn_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 0}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5,  6, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, 9, ASCR_BACK, 0}},         //CharAnim_Down
	{2, (const u8[]){ 16,  17, 18, ASCR_BACK, 0}},         //CharAnim_DownAlt
	{2, (const u8[]){ 10,  11, 12, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){13, 14, 15, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//YCN character definitions
static const CharFrame char_ycnpixel_frame[] = {
	{YCN_ArcMain_Pixel, {  0,  39,  29,  39}, { -50+17, 40+75}}, //0
	{YCN_ArcMain_Pixel, { 29,  39,  28,  37}, { -50+16, 37+75}}, //1
	{YCN_ArcMain_Pixel, { 57,  39,  28,  38}, { -50+16, 38+75}}, //2
	{YCN_ArcMain_Pixel, { 86,  39,  29,  39}, { -50+17, 39+75}}, //3
	{YCN_ArcMain_Pixel, {115,  39,  29,  40}, { -50+17, 40+75}}, //4
	
	{YCN_ArcMain_Pixel, {  0,  78,  29,  38}, { -50+16, 38+75}}, //5
	{YCN_ArcMain_Pixel, { 29,  78,  27,  39}, { -50+15, 39+75}}, //6
	
	{YCN_ArcMain_Pixel, {  0,  0,  29,  38}, { -50+15, 38+75}}, //7
	{YCN_ArcMain_Pixel, { 29,  0,  28,  39}, { -50+15, 39+75}}, //8
	
	{YCN_ArcMain_Pixel, {  0, 155,  27 , 40}, { -50+16, 40+75}}, //9
	{YCN_ArcMain_Pixel, { 28, 155,  28 , 39}, { -50+16, 39+75}}, //10
	
	{YCN_ArcMain_Pixel, {  0, 117,  27,  37}, { -50+15, 37+75}}, //11
	{YCN_ArcMain_Pixel, { 27, 117,  27,  38}, { -50+15, 38+75}}, //12
};

static const Animation char_ycnpixel_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 5,  6, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, ASCR_BACK, 0}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_DownAlt
	{2, (const u8[]){ 9,  10, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){11, 12, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//YCN character definitions
static const CharFrame char_ycnmad_frame[] = {
	{YCN_ArcMain_MIdle0, {  0,   0, 127, 184}, { 55, 175}}, //0
	{YCN_ArcMain_MIdle0, {128,   0, 126, 184}, { 55, 175}}, //1
	{YCN_ArcMain_MIdle1, {  0,   0, 126, 184}, { 55, 175}}, //2
	{YCN_ArcMain_MIdle1, {127,   0, 127, 184}, { 55, 175}}, //3
	
	{YCN_ArcMain_MLeft0, {  0,   0,  168, 170}, { 110, 161}}, //4
	{YCN_ArcMain_MLeft1, {  0,   0,  168, 170}, { 102, 162}}, //5
	{YCN_ArcMain_MLeft2, {  0,   0,  172, 171}, { 105, 163}}, //6
	
	{YCN_ArcMain_MDown0, {  0,   0, 142, 155}, { 74, 153}}, //7
	{YCN_ArcMain_MDown1, {  0,   0, 145, 159}, { 73, 157}}, //8
	{YCN_ArcMain_MDown2, {  0,   0, 141, 158}, { 72, 156}}, //9
	
	{YCN_ArcMain_MUp0, {  0,   0, 127, 192}, { 60, 192}}, //10
	{YCN_ArcMain_MUp1, {  0,   0, 132, 191}, { 62, 191}}, //11
	{YCN_ArcMain_MUp2, {  0,   0, 132, 185}, { 62, 185}}, //12
	
	{YCN_ArcMain_MRight0, {  0,   0, 179, 153}, { 37, 149}}, //13
	{YCN_ArcMain_MRight1, {  0,   0, 160, 154}, { 40, 150}}, //14
	{YCN_ArcMain_MRight2, {  0,   0, 170, 154}, { 39, 150}}, //15
	
	{YCN_ArcMain_Laugh0, {  0,   0, 142, 149}, { 69, 145}}, //16
	{YCN_ArcMain_Laugh1, {  0,   0, 139, 147}, { 66, 142}}, //17
	{YCN_ArcMain_Laugh2, {  0,   0, 135, 148}, { 64, 143}}, //18
};

static const Animation char_ycnmad_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 0}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5,  6, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, 9, ASCR_BACK, 0}},         //CharAnim_Down
	{2, (const u8[]){ 16,  17, 18, ASCR_BACK, 0}},         //CharAnim_DownAlt
	{2, (const u8[]){ 10,  11, 12, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){13, 14, 15, ASCR_BACK, 0}},         //CharAnim_Right
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
		const CharFrame *cframe1 = &char_ycnpixel_frame[this->frame = frame];
		const CharFrame *cframe2 = &char_ycnmad_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
		{
			if(phases == 0)
				Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
			else if(phases == 1)
				Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe1->tex], 0);
			else
				Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe2->tex], 0);
		}
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
	if(phases == 0)
	{
		Character_Draw(character, &this->tex, &char_ycn_frame[this->frame]);
		Animatable_Init(&this->character.animatable, char_ycn_anim);
	}
	else if(phases == 1)
	{
		Character_Draw(character, &this->tex, &char_ycnpixel_frame[this->frame]);
		Animatable_Init(&this->character.animatable, char_ycnpixel_anim);
	}
	else
	{
		Character_Draw(character, &this->tex, &char_ycnmad_frame[this->frame]);
		Animatable_Init(&this->character.animatable, char_ycnmad_anim);
	}
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
	
	phases = 0;
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\YCN.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",    //YCN_ArcMain_Idle0
		"idle1.tim",    //YCN_ArcMain_Idle1
		
		"left0.tim",    //YCN_ArcMain_Left0
		"left1.tim",    //YCN_ArcMain_Left1
		"left2.tim",    //YCN_ArcMain_Left2
		
		"down0.tim",    //YCN_ArcMain_Down0
		"down1.tim",    //YCN_ArcMain_Down1
		"down2.tim",    //YCN_ArcMain_Down2
		
		"up0.tim",      //YCN_ArcMain_Up0
		"up1.tim",      //YCN_ArcMain_Up1
		"up2.tim",      //YCN_ArcMain_Up2
		
		"right0.tim",   //YCN_ArcMain_Right0
		"right1.tim",   //YCN_ArcMain_Right1
		"right2.tim",   //YCN_ArcMain_Right2
		
		"laugh0.tim",   //YCN_ArcMain_Laugh0
		"laugh1.tim",   //YCN_ArcMain_Laugh1
		"laugh2.tim",   //YCN_ArcMain_Laugh1
		
		"midle0.tim",   //YCN_ArcMain_MIdle0
		"midle1.tim",   //YCN_ArcMain_MIdle1
		
		"mleft0.tim",   //YCN_ArcMain_MLeft0
		"mleft1.tim",   //YCN_ArcMain_MLeft1
		"mleft2.tim",   //YCN_ArcMain_MLeft2
		
		"mdown0.tim",   //YCN_ArcMain_MDown0
		"mdown1.tim",   //YCN_ArcMain_MDown1
		"mdown2.tim",   //YCN_ArcMain_MDown2
		
		"mup0.tim",     //YCN_ArcMain_MUp0
		"mup1.tim",     //YCN_ArcMain_MUp1
		"mup2.tim",     //YCN_ArcMain_MUp2
		
		"mright0.tim",  //YCN_ArcMain_MRight0
		"mright1.tim",  //YCN_ArcMain_MRight1
		"mright2.tim",  //YCN_ArcMain_MRight2
		
		"scream0.tim",  //YCN_ArcMain_Scream0
		"scream1.tim",  //YCN_ArcMain_Scream1
		"scream2.tim",  //YCN_ArcMain_Scream2
		"scream3.tim",  //YCN_ArcMain_Scream3
		
		"justdie0.tim", //YCN_ArcMain_JustDie0
		"justdie1.tim", //YCN_ArcMain_JustDie1
		"justdie2.tim", //YCN_ArcMain_JustDie2
		"justdie3.tim", //YCN_ArcMain_JustDie3
		"justdie4.tim", //YCN_ArcMain_JustDie4
		"justdie5.tim", //YCN_ArcMain_JustDie5
		
		"pixel.tim",    //YCN_ArcMain_Pixel
		
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}

void YCNChangePhase(u8 phase)
{
	phases = phase;
}