#include "ycr.h"

#include "../../psx/archive.h"
#include "../../psx/mem.h"
#include "../../stage/stage.h"

//Week 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0;
	Gfx_Tex tex_back1;
	Gfx_Tex tex_back2;
	Gfx_Tex tex_back3;
} Back_YCR;

boolean YCRpixel;

//Week 1 background functions
void Back_YCR_DrawBG(StageBack *back)
{
	Back_YCR *this = (Back_YCR*)back;
	
	fixed_t fx, fy;
	
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	if (YCRpixel == true)
	{
		stage.camera.force = true;
		stage.hidegf = true;
		if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
			stage.camera.tx = FIXED_DEC(-30,1);
		else
			stage.camera.tx = FIXED_DEC(30,1);
		stage.camera.ty = 30;
		stage.camera.zoom = FIXED_DEC(2,1);
		RECT backp_src = {0, 0, 256, 184};
		RECT_FIXED backp_dst = {
			FIXED_DEC(-128,1) - fx,
			FIXED_DEC(-92,1) - fy,
			FIXED_DEC(256,1),
			FIXED_DEC(184,1)
		};
		
		Stage_DrawTex(&this->tex_back2, &backp_src, &backp_dst, stage.camera.bzoom);
	}
	else
	{
		stage.camera.force = false;
		stage.hidegf = false;
		stage.camera.zoom -= FIXED_DEC(5,1000);
		RECT backl_src = {0, 0, 256, 256};
		RECT_FIXED backl_dst = {
			FIXED_DEC(-298,1) - fx,
			FIXED_DEC(-120,1) - fy,
			FIXED_DEC(300,1),
			FIXED_DEC(300,1)
		};
		RECT backr_src = {0, 0, 256, 256};
		RECT_FIXED backr_dst = {
			FIXED_DEC(-1,1) - fx,
			FIXED_DEC(-120,1) - fy,
			FIXED_DEC(300,1),
			FIXED_DEC(300,1)
		};
		
		Stage_DrawTex(&this->tex_back0, &backl_src, &backl_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back1, &backr_src, &backr_dst, stage.camera.bzoom);
	}
}

void Back_YCR_Free(StageBack *back)
{
	Back_YCR *this = (Back_YCR*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_YCR_New(void)
{
	//Allocate background structure
	Back_YCR *this = (Back_YCR*)Mem_Alloc(sizeof(Back_YCR));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_YCR_DrawBG;
	this->back.free = Back_YCR_Free;
	YCRpixel = false;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\STAGES\\YCR.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "2.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}

void StagePixel()
{
	YCRpixel = true;
	stage.camera.x = FIXED_DEC(0,1);
	stage.camera.y = FIXED_DEC(0,1);
	stage.camera.zoom = FIXED_DEC(2,1);
}

void StageNormal()
{
	YCRpixel = false;
	stage.camera.x = FIXED_DEC(0,1);
	stage.camera.y = FIXED_DEC(0,1);
	stage.camera.zoom = FIXED_DEC(1,1);
}
