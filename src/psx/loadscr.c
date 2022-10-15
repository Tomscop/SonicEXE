#include "loadscr.h"

#include "gfx.h"
#include "timer.h"
#include "io.h"
#include "audio.h"
#include "trans.h"

//Loading screen functions
void LoadScr_Start(void)
{
	//Stop music and make sure frame has been drawn
	Gfx_Flip();
	
	//Load loading screen texture
	RECT loading_src = {0, 0, 255, 255};
	RECT loading_dst = {(SCREEN_WIDTH - 300) >> 1, (SCREEN_HEIGHT - 224) >> 1, 300, 224};
	Gfx_Tex loading_tex;
	Gfx_SetClear(0, 0, 0);
	Timer_Reset();
	
	//Draw loading screen and run transition
	Trans_Set();
	
	while (!Trans_Idle())
	{
		//Draw loading screen and end frame
		Timer_Tick();
		Trans_Tick();
		Gfx_DrawTex(&loading_tex, &loading_src, &loading_dst);
		Gfx_Flip();
	}
	
	//Draw an extra frame to avoid double buffering issues
	Gfx_DrawTex(&loading_tex, &loading_src, &loading_dst);
	Gfx_Flip();
}

void LoadScr_End(void)
{

}
