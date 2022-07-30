/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "main.h"

#include "timer.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"
#include "network.h"

#include "pause.h"
#include "menu.h"
#include "stage.h"
#include "save.h"

//Game loop
GameLoop gameloop;
SCREEN screen;

//Error handler
char error_msg[0x200];

void ErrorLock(void)
{
	while (1)
	{
		#ifdef PSXF_PC
			MsgPrint(error_msg);
			exit(1);
		#else
			FntPrint("A fatal error has occured\n~c700%s\n", error_msg);
			Gfx_Flip();
		#endif
	}
}

//Memory heap
#define MEM_STAT //This will enable the Mem_GetStat function which returns information about available memory in the heap

#define MEM_IMPLEMENTATION
#include "mem.h"
#undef MEM_IMPLEMENTATION

#ifndef PSXF_STDMEM
static u8 malloc_heap[0x1A0000];
#endif

//Entry point                                                                             
int main(int argc, char **argv)                                                                                                                                                        
{
	//Remember arguments
	my_argc = argc;
	my_argv = argv;

	//Initialize system
	PSX_Init();
	
	Mem_Init((void*)malloc_heap, sizeof(malloc_heap));
	
	ResetGraph(0);
	Pad_Init();
	InitCARD(1);
	StartPAD();
	StartCARD();
	_bu_init();	
	ChangeClearPAD(0);

	IO_Init();
	Audio_Init();
	Gfx_Init();
	Network_Init();
	Timer_Init(false, false);

	if (readSaveFile() == false)
		defaultSettings();
	stage.dascreen = stage.prefs.widescreen;

	//Start game
	gameloop = GameLoop_Menu;
	Menu_Load(MenuPage_Opening);

	//Game loop
	while (PSX_Running())
	{
		if (stage.prefs.widescreen) {
			screen.SCREEN_WIDTH   = 512;
			screen.SCREEN_HEIGHT  = 240;
			screen.SCREEN_WIDTH2  = (screen.SCREEN_WIDTH >> 1);
			screen.SCREEN_HEIGHT2 = (screen.SCREEN_HEIGHT >> 1);

			screen.SCREEN_WIDEADD = (screen.SCREEN_WIDTH - 512);
			screen.SCREEN_TALLADD = (screen.SCREEN_HEIGHT - 240);
			screen.SCREEN_WIDEADD2 = (screen.SCREEN_WIDEADD >> 1);
			screen.SCREEN_TALLADD2 = (screen.SCREEN_TALLADD >> 1);

			screen.SCREEN_WIDEOADD = (screen.SCREEN_WIDEADD > 0 ? screen.SCREEN_WIDEADD : 0);
			screen.SCREEN_TALLOADD = (screen.SCREEN_TALLADD > 0 ? screen.SCREEN_TALLADD : 0);
			screen.SCREEN_WIDEOADD2 = (screen.SCREEN_WIDEOADD >> 1);
			screen.SCREEN_TALLOADD2 = (screen.SCREEN_TALLOADD >> 1);	

			if (stage.dascreen == 1 && stage.screencooldown == 0) {
				Gfx_Init();
			}
			stage.screencooldown ++;
		}
		else {
			stage.dascreen = 0;
			screen.SCREEN_WIDTH   = 320;
			screen.SCREEN_HEIGHT  = 240;
			screen.SCREEN_WIDTH2  = (screen.SCREEN_WIDTH >> 1);
			screen.SCREEN_HEIGHT2 = (screen.SCREEN_HEIGHT >> 1);
			screen.SCREEN_WIDEADD = (screen.SCREEN_WIDTH - 320);

			screen.SCREEN_TALLADD = (screen.SCREEN_HEIGHT - 240);
			screen.SCREEN_WIDEADD2 = (screen.SCREEN_WIDEADD >> 1);
			screen.SCREEN_TALLADD2 = (screen.SCREEN_TALLADD >> 1);
			screen.SCREEN_WIDEOADD = (screen.SCREEN_WIDEADD > 0 ? screen.SCREEN_WIDEADD : 0);

			screen.SCREEN_TALLOADD = (screen.SCREEN_TALLADD > 0 ? screen.SCREEN_TALLADD : 0);
			screen.SCREEN_WIDEOADD2 = (screen.SCREEN_WIDEOADD >> 1);
			screen.SCREEN_TALLOADD2 = (screen.SCREEN_TALLOADD >> 1);	
			
			if (stage.dascreen == 0 && stage.screencooldown == 0) {
				Gfx_Init();
			}
			stage.screencooldown ++;
		}

		//Prepare frame
		Timer_Tick();
		Audio_ProcessXA();
		Pad_Update();
		
		#ifdef MEM_STAT
			//Memory stats
			size_t mem_used, mem_size, mem_max;
			Mem_GetStat(&mem_used, &mem_size, &mem_max);
			#ifndef MEM_BAR
				FntPrint("mem: %08X/%08X (max %08X)\n", mem_used, mem_size, mem_max);
			#endif
		#endif

		//Set video mode
		switch (stage.prefs.palmode)
		{
			case 1:
				SetVideoMode(MODE_PAL);
				SsSetTickMode(SS_TICK50);
				stage.disp[0].screen.y = stage.disp[1].screen.y = 24;

				if (stage.palcooldown == 1)
					Timer_Init(true, true);
				stage.palcooldown ++;
				break;
			default:
				SetVideoMode(MODE_NTSC);
				SsSetTickMode(SS_TICK60);
				stage.disp[0].screen.y = stage.disp[1].screen.y = 0;
				if (stage.palcooldown == 1)
					Timer_Init(false, false);
				stage.palcooldown ++;
				break;
		}

		//Tick and draw game
		Network_Process();
		switch (gameloop)
		{
			case GameLoop_Menu:
				Menu_Tick();
				break;
			case GameLoop_Stage:
				Stage_Tick();
				break;
			case GameLoop_Pause:
				PausedState();
				break;
		}
		
		//Flip gfx buffers
		Gfx_Flip();
	}
	
	//Deinitialize system
	Network_Quit();
	Pad_Quit();
	Gfx_Quit();
	Audio_Quit();
	IO_Quit();
	
	PSX_Quit();
	return 0;
}