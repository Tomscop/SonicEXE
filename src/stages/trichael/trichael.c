#include "trichael.h"

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
} Back_Trichael;

//Week 1 background functions
void Back_Trichael_DrawBG(StageBack *back)
{
	Back_Trichael *this = (Back_Trichael*)back;
	
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
}

void Back_Trichael_Free(StageBack *back)
{
	Back_Trichael *this = (Back_Trichael*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Trichael_New(void)
{
	//Allocate background structure
	Back_Trichael *this = (Back_Trichael*)Mem_Alloc(sizeof(Back_Trichael));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Trichael_DrawBG;
	this->back.free = Back_Trichael_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\STAGES\\TRICHAEL.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "1.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
