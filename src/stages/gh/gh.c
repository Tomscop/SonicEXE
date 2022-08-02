#include "gh.h"

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
} Back_GH;

//Week 1 background functions
void Back_GH_DrawBG(StageBack *back)
{
	Back_GH *this = (Back_GH*)back;
	
	stage.camera.zoom -= FIXED_DEC(5,1000);
	
	fixed_t fx, fy;
	
	fx = stage.camera.x;
	fy = stage.camera.y;
	
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
	
	fx = stage.camera.x / 2;
	fy = stage.camera.y / 2;
	
	RECT back2l_src = {0, 0, 256, 256};
	RECT_FIXED back2l_dst = {
		FIXED_DEC(-298,1) - fx,
		FIXED_DEC(-135,1) - fy,
		FIXED_DEC(300,1),
		FIXED_DEC(300,1)
	};
	RECT back2r_src = {0, 0, 256, 256};
	RECT_FIXED back2r_dst = {
		FIXED_DEC(-1,1) - fx,
		FIXED_DEC(-135,1) - fy,
		FIXED_DEC(300,1),
		FIXED_DEC(300,1)
	};
	
	Stage_DrawTex(&this->tex_back2, &back2l_src, &back2l_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back3, &back2r_src, &back2r_dst, stage.camera.bzoom);
}

void Back_GH_Free(StageBack *back)
{
	Back_GH *this = (Back_GH*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_GH_New(void)
{
	//Allocate background structure
	Back_GH *this = (Back_GH*)Mem_Alloc(sizeof(Back_GH));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_GH_DrawBG;
	this->back.free = Back_GH_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\STAGES\\GH.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "3.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
