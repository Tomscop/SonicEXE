/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "pause.h"
#include "pad.h"
#include "stage.h"

#include "main.h"
#include "trans.h"

int selection;
int bfangle;
int bfcounter;

void PausedState()
{	
	RECT blue_src = {71,  0, 45, 45};
	RECT yellow_src = {117,   0, 45, 46};
	RECT cancel_src = {163,   0, 51, 10};
	RECT replay_src = {163,  11, 55, 11};
	RECT bf_src = {  1,   0, 69, 46};
	RECT blue_dst = {96,  166, 45, 45};
	RECT yellow_dst = {96,  165, 45, 46};
	RECT cancel_dst = {192, 183, 51, 10};	
	RECT replay_dst = { 92, 183, 55, 11};
	RECT bf_dst = {161,  49, 69, 46};
	if (stage.prefs.widescreen)
	{
		blue_dst.x += 98;
		yellow_dst.x += 98;
		cancel_dst.x += 98;
		replay_dst.x += 98;
		bf_dst.x += 98;
	}
	bfcounter ++;
	if (pad_state.press & (PAD_LEFT | PAD_RIGHT))
		selection ++;

	//Reset selection
	if (selection > 1)
		selection = 0;
	else if (selection < 0)
		selection = 1;

	//Control BF angle
	if (bfcounter >= 50)
		bfangle = 23;
	else 
		bfangle = 0;

	if (bfcounter >= 100)
		bfcounter = 0;

	Gfx_DrawTexRotate(&pause.tex_pause, &bf_src, &bf_dst, bfangle); //Draw BF
	
	//Highlight selection
	if (selection == 0) {
		replay_src.y = 34;
		Gfx_DrawTex(&pause.tex_pause, &replay_src, &replay_dst); //Replay
		Gfx_DrawTex(&pause.tex_pause, &yellow_src, &yellow_dst); 
	}
	else if (selection == 1)	
	{
		if (stage.prefs.widescreen)
			yellow_dst.x = 195 + 98;
		else
			yellow_dst.x = 195;
		cancel_src.y = 23;
		Gfx_DrawTex(&pause.tex_pause, &cancel_src, &cancel_dst); //Cancel
		Gfx_DrawTex(&pause.tex_pause, &yellow_src, &yellow_dst);
	}
	
	//Draw Black text
	Gfx_DrawTex(&pause.tex_pause, &cancel_src, &cancel_dst); //Cancel
	Gfx_DrawTex(&pause.tex_pause, &replay_src, &replay_dst); //Replay
	//Draw Blue buttons
	Gfx_DrawTex(&pause.tex_pause, &blue_src, &blue_dst);
	if (stage.prefs.widescreen)
		blue_dst.x = 195 + 98;
	else
		blue_dst.x = 195;
	Gfx_DrawTex(&pause.tex_pause, &blue_src, &blue_dst);

	//Draw Bg
	RECT back_src = {1, 47, 254, 208};
	RECT back_dst = {0,  0, screen.SCREEN_WIDTH, screen.SCREEN_HEIGHT};
	Gfx_DrawTex(&pause.tex_pause, &back_src, &back_dst);

	

	if (pad_state.press & (PAD_CROSS | PAD_START) && selection == 1)
	{
		stage.trans = StageTrans_Menu;
		gameloop = GameLoop_Stage;
		Trans_Start();
	}
	if (pad_state.press & (PAD_CROSS | PAD_START) && selection == 0)
	{
		stage.trans = StageTrans_Reload;
		gameloop = GameLoop_Stage;
		Trans_Start();
	}
}
void Pause_load()
{
	bfangle = 0;
	selection = 0;
	bfcounter = 0;
	Gfx_LoadTex(&pause.tex_pause, IO_Read("\\STAGE\\PAUSE.TIM;1"), GFX_LOADTEX_FREE);
	PausedState();
}