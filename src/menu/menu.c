#include "menu.h"

#include "../main.h"
#include "../psx/mem.h"
#include "../psx/timer.h"
#include "../psx/io.h"
#include "../psx/gfx.h"
#include "../psx/audio.h"
#include "../psx/pad.h"
#include "../psx/archive.h"
#include "../psx/mutil.h"
#include "../psx/save.h"

#include "../fonts/font.h"
#include "../psx/trans.h"
#include "../psx/loadscr.h"

#include "../stage/stage.h"
#include "../characters/gf/gf.h"

//Menu string type
#define MENUSTR_CHARS 0x20
typedef char MenuStr[MENUSTR_CHARS + 1];

//Menu state
static struct
{
	//Menu state
	u32 sound[3];
	u8 page, next_page;
	boolean page_swap;
	u8 select, select2, next_select;
	u8 diff;
	s16 animcounter;
	
	fixed_t scroll;
	fixed_t trans_time;
	
	//Page specific state
	union
	{
		struct
		{
			fixed_t logo_bump;
			fixed_t fade, fadespd;
		} title;
		struct
		{
			fixed_t fade, fadespd;
		} story;
		struct
		{
			fixed_t back_r, back_g, back_b;
		} freeplay;
	} page_state;
	
	union
	{
		struct
		{
			u8 id, diff;
			boolean story;
		} stage;
	} page_param;
	
	//Menu assets
	Gfx_Tex tex_back, tex_title, tex_menu, tex_backopt;
	Gfx_Tex tex_story;
	FontData font_bold, font_arial;
	
	IO_Data back_arc;
	u8 backanim, backspeed;
	boolean music2;
	u8 options;
	
	Character *gf; //Title Girlfriend
} menu;


//Internal menu functions
char menu_text_buffer[0x100];

static const char *Menu_LowerIf(const char *text, boolean lower)
{
	//Copy text
	char *dstp = menu_text_buffer;
	if (lower)
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'A' && *srcp <= 'Z')
				*dstp++ = *srcp | 0x20;
			else
				*dstp++ = *srcp;
		}
	}
	else
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'a' && *srcp <= 'z')
				*dstp++ = *srcp & ~0x20;
			else
				*dstp++ = *srcp;
		}
	}
	
	//Terminate text
	*dstp++ = '\0';
	return menu_text_buffer;
}

//Menu functions
void Menu_Load(MenuPage page)
{
	//Load menu assets
	IO_Data menu_arc = IO_Read("\\MENU\\MENU.ARC;1");
	Gfx_LoadTex(&menu.tex_title, Archive_Find(menu_arc, "title.tim"), 0);
	Gfx_LoadTex(&menu.tex_story, Archive_Find(menu_arc, "story.tim"), 0);
	Gfx_LoadTex(&menu.tex_menu, Archive_Find(menu_arc, "menu.tim"), 0);
	Gfx_LoadTex(&menu.tex_backopt, Archive_Find(menu_arc, "optionsbg.tim"), 0);
	Mem_Free(menu_arc);
	
	menu.back_arc = IO_Read("\\MENU\\BACK.ARC;1");
	
	FontData_Load(&menu.font_bold, Font_Bold);
	FontData_Load(&menu.font_arial, Font_Arial);
	
	//Initialize menu state
	menu.select = menu.next_select = 0;
	
	switch (menu.page = menu.next_page = page)
	{
		case MenuPage_Opening:
			//Do this here so timing is less reliant on VSync
			menu.music2 = false;
			break;
		default:
			break;
	}
	menu.page_swap = true;
	
	menu.trans_time = 0;
	Trans_Clear();
	
	stage.song_step = 0;
	
	//Load sfx
	Audio_ClearAlloc();
	CdlFILE file;

	IO_FindFile(&file, "\\SOUNDS\\SCROLL.VAG;1");
   	u32 *data = IO_ReadFile(&file);
    menu.sound[0] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);
	
	IO_FindFile(&file, "\\SOUNDS\\SELECT.VAG;1");
   	data = IO_ReadFile(&file);
    menu.sound[1] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);
	
	IO_FindFile(&file, "\\SOUNDS\\MENULAUG.VAG;1");
   	data = IO_ReadFile(&file);
    menu.sound[2] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);
	
	//Play menu music
	if(menu.music2)
		Audio_PlayXA_Track(XA_StoryMode, 0x40, 2, 1);
	else
		Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
	Audio_WaitPlayXA();
	
	//Set background colour
	Gfx_SetClear(0, 0, 0);
}

void Menu_Unload(void)
{
	//Free title Girlfriend
	Character_Free(menu.gf);
	
	Mem_Free(menu.back_arc);
}

void Menu_ToStage(StageId id, StageDiff diff, boolean story)
{
	menu.next_page = MenuPage_Stage;
	menu.page_param.stage.id = id;
	menu.page_param.stage.story = story;
	menu.page_param.stage.diff = diff;
	Trans_Start();
}

void Menu_Tick(void)
{
	//Clear per-frame flags
	stage.flag &= ~STAGE_FLAG_JUST_STEP;
	
	//Get song position
	u16 next_step = Audio_TellXA_Milli() / 147; //100 BPM
	if (next_step != stage.song_step)
	{
		if (next_step >= stage.song_step)
			stage.flag |= STAGE_FLAG_JUST_STEP;
		stage.song_step = next_step;
	}
	
	//Handle transition out
	if (Trans_Tick())
	{
		//Change to set next page
		menu.page_swap = true;
		menu.page = menu.next_page;
		menu.select = menu.next_select;
	}
	
	//Tick menu page
	MenuPage exec_page;
	switch (exec_page = menu.page)
	{
		case MenuPage_Opening:
		{
			menu.page = menu.next_page = MenuPage_Title;
			menu.page_swap = true;
		}
	//Fallthrough
		case MenuPage_Title:
		{
			if(menu.music2)
			{
				Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
				menu.music2 = false;
			}
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(90,1);
			}
			
			//Draw white fade
			if (menu.page_state.title.fade > 0)
			{
				static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
				u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
			}
			
			//Go to main menu when start is pressed
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if ((pad_state.press & PAD_START) && menu.next_page == menu.page && Trans_Idle())
			{
				Audio_PlaySound(menu.sound[2], 0x3fff);
				menu.trans_time = FIXED_UNIT;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(300,1);
				menu.next_page = MenuPage_Main;
				menu.next_select = 0;
			}
			
			//Draw Friday Night Funkin' logo
			RECT logo_src = {0, 0, 255, 150};
			RECT logo_dst = {
				SCREEN_WIDTH2,
				120,
				255,
				150
			};
			
			Gfx_DrawTexRotate(&menu.tex_title, &logo_src, &logo_dst, 0, logo_dst.w / 2, logo_dst.h / 2);
			
			if (menu.page_state.title.logo_bump > 0)
				if ((menu.page_state.title.logo_bump -= timer_dt) < 0)
					menu.page_state.title.logo_bump = 0;
			
			//Draw "Press Start to Begin"
			if (menu.next_page == menu.page)
			{
				//Blinking blue
				s16 press_lerp = (MUtil_Cos(animf_count << 3) + 0x100) >> 1;
				u8 press_r = 51 >> 1;
				u8 press_g = (58  + ((press_lerp * (255 - 58))  >> 8)) >> 1;
				u8 press_b = (206 + ((press_lerp * (255 - 206)) >> 8)) >> 1;
				
				RECT press_src = {0, 151, 207, 18};
				Gfx_BlitTexCol(&menu.tex_title, &press_src, SCREEN_WIDTH2 - press_src.w / 2, SCREEN_HEIGHT - 48, press_r, press_g, press_b);
			}
			else
			{
				//Flash white
				RECT press_src = {0, (animf_count & 1) ? 151 : 169, 207, 18};
				Gfx_BlitTex(&menu.tex_title, &press_src, SCREEN_WIDTH2 - press_src.w / 2, SCREEN_HEIGHT - 48);
			}
			break;
		}
		case MenuPage_Main:
		{
			static const char *menu_options[] = {
				"STORY MODE",
				"ENCORE",
				"FREEPLAY",
				"SOUND TEST",
				"OPTIONS",
				"EXTRAS",
			};
			
			if (!menu.music2)
			{
				menu.music2 = true;
				Audio_PlayXA_Track(XA_StoryMode, 0x40, 2, 1);
			}
			
			//Handle option and selection
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					Audio_PlaySound(menu.sound[1], 0x3fff);
					switch (menu.select)
					{
						case 0: //Story Mode
							menu.next_page = MenuPage_Story;
							break;
						case 1: //Encore
							menu.next_page = MenuPage_Encore;
							break;
						case 2: //Freeplay
							menu.next_page = MenuPage_Freeplay;
							break;
						case 3: //Sound Test
							menu.next_page = MenuPage_SoundTest;
							break;
						case 4: //Options
							menu.next_page = MenuPage_Options;
							break;
						case 5: //Extras
							menu.next_page = MenuPage_Extras;
							break;
					}
					menu.next_select = 0;
					menu.trans_time = FIXED_UNIT;
				}
				
				//Return to title screen if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Title;
					Trans_Start();
				}
			}
			
			//Draw options
			u8 menu_buttons = 0;
			while(menu_buttons < 6)
			{
				if ((menu.next_page == menu.page) || (menu.select == menu_buttons))
				{
					RECT story_src = {(menu.select == menu_buttons) ? 101 : 0, 0 + (23 * menu_buttons), 101, 23};
					Gfx_BlitTex(&menu.tex_menu, &story_src, (SCREEN_WIDTH - 156) + (10 * menu_buttons), 40 + (28 * menu_buttons));
				}
				menu_buttons += 1;
			}
			
			static const char *back_files[] = {
				"m0.tim",
				"m1.tim",
				"m2.tim",
				"m3.tim",
			};
			
			//Draw Background
			Gfx_LoadTex(&menu.tex_back, Archive_Find(menu.back_arc, back_files[menu.backanim]), 0);
			RECT back_src = {0, 0, 256, 180};
			RECT back_dst = {-50, 0, 341, 240};
			Gfx_DrawTex(&menu.tex_back, &back_src, &back_dst);
			if(menu.backspeed > 0)
				menu.backspeed -= 1;
			else
			{
				menu.backspeed = 2;
				if(menu.backanim < 3)
					menu.backanim += 1;
				else
					menu.backanim = 0;
			}
			break;
		}
		case MenuPage_Story:
		{
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					if (menu.select2 > 0)
						menu.select2--;
					else
						menu.select2 = 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					if (menu.select2 < 1)
						menu.select2++;
					else
						menu.select2 = 0;
				}
				
				if(menu.select2 == 0)
				{
					//Change option
					if (pad_state.press & PAD_LEFT)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select > 0)
							menu.select--;
						else
							menu.select = 2;
					}
					if (pad_state.press & PAD_RIGHT)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select < 2)
							menu.select++;
						else
							menu.select = 0;
					}
				}
				if(menu.select2 == 1)
				{
					//Change option
					if (pad_state.press & PAD_LEFT)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.diff > 0)
							menu.diff--;
						else
							menu.diff = 2;
					}
					if (pad_state.press & PAD_RIGHT)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.diff < 2)
							menu.diff++;
						else
							menu.diff = 0;
					}
				}
			}
			
			//Draw Arrows
			u8 drawarrows = 2;
			while(drawarrows > 0)
			{
				RECT arrows_src[3] = 
				{
					{ 82, 191, 14, 23},
					{ 56, 191, 12, 23},
					{ 68, 191, 14, 23}
				};
				
				//Song Selection
				u8 frames = 2;
				
				if(drawarrows == 1)
				{
					if(menu.select2 == 0)
					{
						frames = 0;
					}
				}
				else
				{
					if(menu.select2 == 0)
					{
						frames = 0;
					}
				}
				
				RECT arrow_dst = {72, 61, arrows_src[frames].w, arrows_src[frames].h};
				
				if(drawarrows == 1)
				{
					arrow_dst.x = 248;
					arrow_dst.w = -arrow_dst.w;
				}
				
				Gfx_DrawTex(&menu.tex_story, &arrows_src[frames], &arrow_dst);
				
				//Difficulty Selection
				frames = 2;
				
				if(drawarrows == 1)
				{
					if(menu.select2 == 1)
					{
						frames = 0;
					}
				}
				else
				{
					if(menu.select2 == 1)
					{
						frames = 0;
					}
				}
				
				RECT arrow2_dst = {97, 210, arrows_src[frames].w, arrows_src[frames].h};
				
				if(drawarrows == 1)
				{
					arrow2_dst.x = 226;
					arrow2_dst.w = -arrow2_dst.w;
				}
				
				Gfx_DrawTex(&menu.tex_story, &arrows_src[frames], &arrow2_dst);
				
				drawarrows -= 1;
			}
			
			//Draw difficulties
			RECT diffs_src[3] = 
			{
				{  0, 172, 51, 18},
				{ 52, 172, 79, 18},
				{  0, 191, 55, 18}
			};
			
			Gfx_BlitTex(&menu.tex_story, &diffs_src[menu.diff], SCREEN_WIDTH2 - diffs_src[menu.diff].w / 2 + 4, 213);
			
			//Draw Art
			RECT art_src = {143, menu.select * 59, 113, 58};
			Gfx_BlitTex(&menu.tex_story, &art_src, 104, 45);
			
			//Draw Selection Background
			RECT story_src = {0, 0, 142, 171};
			Gfx_BlitTex(&menu.tex_story, &story_src, SCREEN_WIDTH2 - story_src.w / 2, 34);
			
			//Draw Background
			static const char *back_files[] = {
				"s0.tim",
				"s05.tim",
				"s1.tim",
				"s15.tim",
			};
			
			Gfx_LoadTex(&menu.tex_back, Archive_Find(menu.back_arc, back_files[menu.backanim]), 0);
			RECT back_src = {0, 0, 256, 256};
			RECT back_dst = {0, 0, 320, 240};
			Gfx_DrawTex(&menu.tex_back, &back_src, &back_dst);
			if(menu.backspeed > 0)
				menu.backspeed -= 1;
			else
			{
				menu.backspeed = 2;
				if(menu.backanim < 3)
					menu.backanim += 1;
				else
					menu.backanim = 0;
			}
			break;
		}
		case MenuPage_Freeplay:
		{
			static const struct
			{
				StageId stage;
				u32 col;
				const char *text;
			} menu_options[] = {
				//{StageId_4_4, 0xFFFC96D7, "TEST"},
				{StageId_TooSlow, 0xFF9271FD, "TOO SLOW"},
				{StageId_YouCantRun, 0xFF9271FD, "YOU CAN'T RUN"},
				{StageId_TripleTrouble, 0xFF9271FD, "TRIPLE TROUBLE"},
				
				{StageId_TooSlowEncore, 0xFF9271FD, "TOO SLOW ENCORE"},
				
				{StageId_Endless, 0xFF9271FD, "ENDLESS"},
				{StageId_EndlessOG, 0xFF9271FD, "ENDLESS OG"},
				
				{StageId_Cycles, 0xFF9271FD, "CYCLES"},
				{StageId_Execution, 0xFF9271FD, "EXECUTION"},
				
				{StageId_Sunshine, 0xFF9271FD, "SUNSHINE"},
				{StageId_Soulless, 0xFF9271FD, "SOULLESS"},
				
				{StageId_Chaos, 0xFF9271FD, "CHAOS"},
				
				{StageId_Prey, 0xFF9271FD, "PREY"},
				{StageId_Fight_Or_Flight, 0xFF9271FD, "FIGHT OR FLIGHT"},
				
				{StageId_Round_A_Bout, 0xFF9271FD, "ROUND-A-BOUT"}
			};
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
				menu.page_param.stage.diff = StageDiff_Normal;
				menu.page_state.freeplay.back_r = FIXED_DEC(255,1);
				menu.page_state.freeplay.back_g = FIXED_DEC(255,1);
				menu.page_state.freeplay.back_b = FIXED_DEC(255,1);
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"FREEPLAY",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					Audio_PlaySound(menu.sound[1], 0x3fff);
					menu.next_page = MenuPage_Stage;
					menu.page_param.stage.id = menu_options[menu.select].stage;
					menu.page_param.stage.story = false;
					Trans_Start();
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					menu.next_page = MenuPage_Main;
					menu.next_select = 2; //Freeplay
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(menu_options[i].text, menu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			RECT back_src = {0, 0, 256, 256};
			RECT back_dst = {0, 0, 320, 240};
			Gfx_DrawTex(&menu.tex_backopt, &back_src, &back_dst);
			break;
		}
		case MenuPage_Options:
		{
			//Initialize page
			if (menu.page_swap)
				menu.options = 0;
			
			if (menu.options == 0)
			{
				static const char *main_options[] = {
					"CONTROLS",
					"PREFERENCES",
					"MEMORY CARD",
					"MOD OPTIONS",
				};
				
				//Handle option and selection
				if (menu.next_page == menu.page && Trans_Idle())
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select > 0)
							menu.select--;
						else
							menu.select = COUNT_OF(main_options) - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select < COUNT_OF(main_options) - 1)
							menu.select++;
						else
							menu.select = 0;
					}
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					switch (menu.select)
					{
						case 0:
							menu.options = 9;
							menu.select = 0;
							break;
						case 1:
							menu.options = 1;
							menu.select = 0;
							break;
						case 2:
							menu.options = 2;
							menu.select = 0;
							break;
					}
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					menu.next_page = MenuPage_Main;
					menu.next_select = 4; //Options
					Trans_Start();
				}
				
				//Draw options
				for (u8 i = 0; i < COUNT_OF(main_options); i++)
				{
					//Draw text
					char text[0x80];
					sprintf(text, "%s", main_options[i]);
					menu.font_bold.draw(&menu.font_bold,
						Menu_LowerIf(text, menu.select != i),
						SCREEN_WIDTH2,
						SCREEN_HEIGHT2 + (i * 24) - (COUNT_OF(main_options) * 12),
						FontAlign_Center
					);
				}
			}
			
			if (menu.options == 9)
			{
				RECT keys_src = {  0, 197, 173, 28};
			
				Gfx_BlitTex(&menu.tex_menu, &keys_src, SCREEN_WIDTH2 - keys_src.w / 2, 20);
			}
			
			if (menu.options == 1)
			{
				static const char *main_options[] = {
					"NOTE COLORS",
					"ADJUST COMBO",
					"GRAPHICS",
					"VISUALS AND UI",
					"GAMEPLAY",
				};
				
				//Handle option and selection
				if (menu.next_page == menu.page && Trans_Idle())
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select > 0)
							menu.select--;
						else
							menu.select = COUNT_OF(main_options) - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select < COUNT_OF(main_options) - 1)
							menu.select++;
						else
							menu.select = 0;
					}
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					switch (menu.select)
					{
						case 0:
							break;
						case 3:
							menu.options = 13;
							menu.select = 0;
							break;
						case 4:
							menu.options = 14;
							menu.select = 0;
							break;
					}
				}
				
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.options = 0;
					menu.select = 1;
					Audio_PlaySound(menu.sound[0], 0x3fff);
				}
				
				//Draw options
				for (u8 i = 0; i < COUNT_OF(main_options); i++)
				{
					//Draw text
					char text[0x80];
					sprintf(text, "%s", main_options[i]);
					menu.font_bold.draw(&menu.font_bold,
						Menu_LowerIf(text, menu.select != i),
						SCREEN_WIDTH2,
						SCREEN_HEIGHT2 + (i * 24) - (COUNT_OF(main_options) * 12),
						FontAlign_Center
					);
				}
			}
			
			if (menu.options == 13)
			{
				static const struct
				{
					enum
					{
						OptType_Boolean,
						OptType_Scroll,
						OptType_Enum,
					} type;
					const char *text;
					void *value;
					union
					{
						struct
						{
							int a;
						} spec_boolean;
						struct
						{
							s32 max;
							const char **strs;
						} spec_enum;
					} spec;
				} menu_options[] = {
					{OptType_Boolean, "NOTE SPLASHES", &stage.prefs.splash, {.spec_boolean = {0}}},
					{OptType_Boolean, "HIDE HUD", &stage.prefs.hidehud, {.spec_boolean = {0}}},
					{OptType_Boolean, "TIME BAR", &stage.prefs.timebar, {.spec_boolean = {0}}},
					{OptType_Boolean, "FLASHING LIGHTS", &stage.prefs.flashing, {.spec_boolean = {0}}},
					{OptType_Boolean, "CAMERA ZOOMS", &stage.prefs.camerazoom, {.spec_boolean = {0}}},
					{OptType_Boolean, "COMBO STACKING", &stage.prefs.combostack, {.spec_boolean = {0}}}
				};
			
				//Handle option and selection
				if (menu.next_page == menu.page && Trans_Idle())
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select > 0)
							menu.select--;
						else
							menu.select = COUNT_OF(menu_options) - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select < COUNT_OF(menu_options) - 1)
							menu.select++;
						else
							menu.select = 0;
					}
					
					//Handle option changing
					switch (menu_options[menu.select].type)
					{
						case OptType_Boolean:
							if (pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
							{
								Audio_PlaySound(menu.sound[0], 0x3fff);
								*((boolean*)menu_options[menu.select].value) ^= 1;
							}
							break;
						case OptType_Enum:
							if (pad_state.press & PAD_LEFT)
							{
								Audio_PlaySound(menu.sound[0], 0x3fff);
								if (--*((s32*)menu_options[menu.select].value) < 0)
									*((s32*)menu_options[menu.select].value) = menu_options[menu.select].spec.spec_enum.max - 1;
							}
							if (pad_state.press & PAD_RIGHT)
							{
								Audio_PlaySound(menu.sound[0], 0x3fff);
								if (++*((s32*)menu_options[menu.select].value) >= menu_options[menu.select].spec.spec_enum.max)
									*((s32*)menu_options[menu.select].value) = 0;
							}
							break;
						case OptType_Scroll:
							if (pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
							{
								Audio_PlaySound(menu.sound[0], 0x3fff);
								*((boolean*)menu_options[menu.select].value) ^= 1;
							}
							break;
					}
					//Return to main menu if circle is pressed
					if (pad_state.press & PAD_CIRCLE)
					{
						menu.options = 1;
						menu.select = 3;
						Audio_PlaySound(menu.sound[0], 0x3fff);
					}
				}
				
				//Draw options
				for (u8 i = 0; i < COUNT_OF(menu_options); i++)
				{
					//Draw text
					char text[0x80];
					switch (menu_options[i].type)
					{
						case OptType_Boolean:
							sprintf(text, "%s %s", menu_options[i].text, *((boolean*)menu_options[i].value) ? "ON" : "OFF");
							break;
						case OptType_Scroll:
							sprintf(text, "%s %s", menu_options[i].text, *((boolean*)menu_options[i].value) ? "DOWN" : "UP");
							break;
						case OptType_Enum:
							sprintf(text, "%s %s", menu_options[i].text, menu_options[i].spec.spec_enum.strs[*((s32*)menu_options[i].value)]);
							break;
					}
					menu.font_bold.draw(&menu.font_bold,
						Menu_LowerIf(text, menu.select != i),
						SCREEN_WIDTH2,
						SCREEN_HEIGHT2 + (i * 24) - (COUNT_OF(menu_options) * 12),
						FontAlign_Center
					);
				}
			}
			
			if (menu.options == 14)
			{
				static const char *gamemode_strs[] = {"NORMAL", "SWAP", "MULTIPLAYER"};
				static const struct
				{
					enum
					{
						OptType_Boolean,
						OptType_Scroll,
						OptType_Enum,
					} type;
					const char *text;
					void *value;
					union
					{
						struct
						{
							int a;
						} spec_boolean;
						struct
						{
							s32 max;
							const char **strs;
						} spec_enum;
					} spec;
				} menu_options[] = {
					{OptType_Enum,    "GAMEMODE", &stage.prefs.mode, {.spec_enum = {COUNT_OF(gamemode_strs), gamemode_strs}}},
					{OptType_Scroll,  "SCROLL", &stage.prefs.downscroll, {.spec_boolean = {0}}},
					{OptType_Boolean, "MIDDLESCROLL", &stage.prefs.middlescroll, {.spec_boolean = {0}}},
					{OptType_Boolean, "GHOST TAP", &stage.prefs.ghost, {.spec_boolean = {0}}},
					{OptType_Boolean, "VIBRATION", &stage.prefs.vibrate, {.spec_boolean = {0}}},
					{OptType_Boolean, "BOTPLAY", &stage.prefs.botplay, {.spec_boolean = {0}}}
				};
			
				//Handle option and selection
				if (menu.next_page == menu.page && Trans_Idle())
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select > 0)
							menu.select--;
						else
							menu.select = COUNT_OF(menu_options) - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select < COUNT_OF(menu_options) - 1)
							menu.select++;
						else
							menu.select = 0;
					}
					
					//Handle option changing
					switch (menu_options[menu.select].type)
					{
						case OptType_Boolean:
							if (pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
							{
								Audio_PlaySound(menu.sound[0], 0x3fff);
								*((boolean*)menu_options[menu.select].value) ^= 1;
							}
							break;
						case OptType_Enum:
							if (pad_state.press & PAD_LEFT)
							{
								Audio_PlaySound(menu.sound[0], 0x3fff);
								if (--*((s32*)menu_options[menu.select].value) < 0)
									*((s32*)menu_options[menu.select].value) = menu_options[menu.select].spec.spec_enum.max - 1;
							}
							if (pad_state.press & PAD_RIGHT)
							{
								Audio_PlaySound(menu.sound[0], 0x3fff);
								if (++*((s32*)menu_options[menu.select].value) >= menu_options[menu.select].spec.spec_enum.max)
									*((s32*)menu_options[menu.select].value) = 0;
							}
							break;
						case OptType_Scroll:
							if (pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
							{
								Audio_PlaySound(menu.sound[0], 0x3fff);
								*((boolean*)menu_options[menu.select].value) ^= 1;
							}
							break;
					}
					//Return to main menu if circle is pressed
					if (pad_state.press & PAD_CIRCLE)
					{
						menu.options = 1;
						menu.select = 4;
						Audio_PlaySound(menu.sound[0], 0x3fff);
					}
				}
				
				//Draw options
				for (u8 i = 0; i < COUNT_OF(menu_options); i++)
				{
					//Draw text
					char text[0x80];
					switch (menu_options[i].type)
					{
						case OptType_Boolean:
							sprintf(text, "%s %s", menu_options[i].text, *((boolean*)menu_options[i].value) ? "ON" : "OFF");
							break;
						case OptType_Scroll:
							sprintf(text, "%s %s", menu_options[i].text, *((boolean*)menu_options[i].value) ? "DOWN" : "UP");
							break;
						case OptType_Enum:
							sprintf(text, "%s %s", menu_options[i].text, menu_options[i].spec.spec_enum.strs[*((s32*)menu_options[i].value)]);
							break;
					}
					menu.font_bold.draw(&menu.font_bold,
						Menu_LowerIf(text, menu.select != i),
						SCREEN_WIDTH2,
						SCREEN_HEIGHT2 + (i * 24) - (COUNT_OF(menu_options) * 12),
						FontAlign_Center
					);
				}
			}
			
			if (menu.options == 2)
			{
				static const char *main_options[] = {
					"CARD LOAD",
					"CARD SAVE",
					"RESET ALL DEFAULT"
				};
				
				//Handle option and selection
				if (menu.next_page == menu.page && Trans_Idle())
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select > 0)
							menu.select--;
						else
							menu.select = COUNT_OF(main_options) - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						Audio_PlaySound(menu.sound[0], 0x3fff);
						if (menu.select < COUNT_OF(main_options) - 1)
							menu.select++;
						else
							menu.select = 0;
					}
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					Audio_PlaySound(menu.sound[0], 0x3fff);
					switch (menu.select)
					{
						case 0:
							break;
						case 1:
							writeSaveFile();
							Audio_PlaySound(menu.sound[1], 0x3fff);
							break;
						case 2:
							defaultSettings();
							writeSaveFile();
							Audio_PlaySound(menu.sound[1], 0x3fff);
							break;
					}
				}
				
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.options = 0;
					menu.select = 2;
					Audio_PlaySound(menu.sound[0], 0x3fff);
				}
				
				//Draw options
				for (u8 i = 0; i < COUNT_OF(main_options); i++)
				{
					//Draw text
					char text[0x80];
					sprintf(text, "%s", main_options[i]);
					menu.font_bold.draw(&menu.font_bold,
						Menu_LowerIf(text, menu.select != i),
						SCREEN_WIDTH2,
						SCREEN_HEIGHT2 + (i * 24) - (COUNT_OF(main_options) * 12),
						FontAlign_Center
					);
				}
			}
			
			//Draw background
			RECT back_src = {0, 0, 256, 256};
			RECT back_dst = {0, 0, 320, 240};
			Gfx_DrawTex(&menu.tex_backopt, &back_src, &back_dst);
			break;
		}
		case MenuPage_Stage:
		{
			//Unload menu state
			Menu_Unload();
			
			//Load new stage
			LoadScr_Start();
			Stage_Load(menu.page_param.stage.id, menu.page_param.stage.diff, menu.page_param.stage.story);
			gameloop = GameLoop_Stage;
			LoadScr_End();
			break;
		}
		default:
			break;
	}
	
	//Clear page swap flag
	menu.page_swap = menu.page != exec_page;
}
