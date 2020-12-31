/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <psppower.h>
#include <pspiofilemgr.h>

#include "psp_global.h"
#include "psp_sdl.h"
#include "psp_menu.h"
#include "psp_menu_help.h"
#include "psp_kbd.h"
#include "psp_mancala.h"

extern SDL_Surface *back_surface;

# define MENU_GAME_MODE    0
# define MENU_GAME_RULES   1
# define MENU_INIT_STONES  2
# define MENU_LEVEL        3
# define MENU_CLOCK        4
# define MENU_SCREENSHOT   5

# define MENU_HELP         6
# define MENU_NEW_GAME     7
# define MENU_RESIGN_GAME  8
# define MENU_BACK         9
# define MENU_SAVE_CFG    10
# define MENU_EXIT        11

# define MAX_MENU_ITEM (MENU_EXIT + 1)

  static menu_item_t menu_list[] =
  { 
    { "Game mode       : " },
    { "Game rules      : " },
    { "Initial stones  : " },
    { "Level           : " },
    { "Clock frequency : " },
    { "Save Screenshot : " },

    { "Help" },

    { "New game" },
    { "Resign game" },
    { "Back to game" },
    { "Save config" },
    { "Exit" },
  };

  static int cur_menu_id = MENU_BACK;

  static int mancala_game_mode   = MANCALA_HUMAN_VS_COMPUTER;
  static int mancala_game_rules  = MANCALA_AWARI;
  static int mancala_level       = 4;
  static int mancala_init_stones = MANCALA_INIT_STONES;
  static int psp_cpu_clock       = 222;

void
string_fill_with_space(char *buffer, int size)
{
  int length = strlen(buffer);
  int index;

  for (index = length; index < size; index++) {
    buffer[index] = ' ';
  }
  buffer[size] = 0;
}

static void 
psp_display_screen_menu(void)
{
  char buffer[128];
  int menu_id = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  psp_sdl_blit_background();
  
  x      = 10;
  y      = 15;
  y_step = 10;
  
  for (menu_id = 0; menu_id < MAX_MENU_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;
    else 
    if (menu_id == MENU_EXIT) color = PSP_MENU_RED_COLOR;
    else
    if (menu_id == MENU_HELP) color = PSP_MENU_GREEN_COLOR;

    psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);

    if (menu_id == MENU_GAME_MODE) {
      if (mancala_game_mode == MANCALA_HUMAN_VS_COMPUTER) strcpy(buffer, "Human vs PSP");
      else
      if (mancala_game_mode == MANCALA_HUMAN_VS_HUMAN) strcpy(buffer, "Human vs Human");
      else                                             strcpy(buffer, "PSP vs PSP");
      string_fill_with_space(buffer, 15);
      psp_sdl_back2_print(120, y, buffer, color);
    } else
    if (menu_id == MENU_GAME_RULES) {
      if (mancala_game_rules == MANCALA_AWARI) strcpy(buffer, "Awari");
      else                                     strcpy(buffer, "Mancala");
      string_fill_with_space(buffer, 8);
      psp_sdl_back2_print(120, y, buffer, color);
    } else
    if (menu_id == MENU_LEVEL) {
      sprintf(buffer,"%d", mancala_level);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(120, y, buffer, color);
    } else
    if (menu_id == MENU_INIT_STONES) {
      sprintf(buffer,"%d", mancala_init_stones);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(120, y, buffer, color);
    } else
    if (menu_id == MENU_CLOCK) {
      sprintf(buffer,"%d", psp_cpu_clock);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(120, y, buffer, color);
    } else
    if (menu_id == MENU_SCREENSHOT) {
      sprintf(buffer,"%d", MANCALA.psp_screenshot_id);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(120, y, buffer, color);
      y += y_step * 2;
    } else
    if (menu_id == MENU_SAVE_CFG) {
      y += y_step;
    } else
    if (menu_id == MENU_HELP) {
      y += y_step * 2;
    } else
    if (menu_id == MENU_EXIT) {
      y += y_step;
    }

    y += y_step;
  }
}

static void
psp_main_menu_validate(void)
{
  int need_reset = 0;

  MANCALA.psp_cpu_clock = psp_cpu_clock;

  if (mancala_game_mode != MANCALA.mancala_game_mode) {
    MANCALA.mancala_game_mode = mancala_game_mode;
  }

  if (mancala_game_rules != MANCALA.mancala_game_rules) {
    MANCALA.mancala_game_rules = mancala_game_rules;
    need_reset = 1;
  }

  if (mancala_level != MANCALA.mancala_level) {
    MANCALA.mancala_level = mancala_level;
  }

  if (mancala_init_stones != MANCALA.mancala_init_stones) {
    MANCALA.mancala_init_stones = mancala_init_stones;
    need_reset = 1;
  }
  myPowerSetClockFrequency(MANCALA.psp_cpu_clock);

  if (need_reset) {
    psp_mancala_new_game();
  }
}

void
psp_main_menu_new_game(void)
{
  psp_main_menu_validate();
  psp_mancala_new_game();
}

void
psp_main_menu_resign_game(void)
{
  psp_mancala_resign_game();
}


static void
psp_main_menu_save_config()
{
  int error;

  psp_main_menu_validate();
  error = psp_save_configuration();

  if (! error) /* save OK */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(270, 150, "File saved !", PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(270, 150, "Can't save file !", PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_main_menu_screenshot(void)
{
  MANCALA.psp_screenshot_mode = 1;
}

int
psp_main_menu_exit(void)
{
  SceCtrlData c;

  psp_display_screen_menu();
  psp_sdl_back2_print(270, 150, "press X to confirm !", PSP_MENU_WARNING_COLOR);
  psp_sdl_flip();

  psp_kbd_wait_no_button();

  do
  {
    myCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (c.Buttons & PSP_CTRL_CROSS) psp_sdl_exit(0);

  } while (c.Buttons == 0);

  psp_kbd_wait_no_button();

  return 0;
}

static void
psp_main_menu_init(void)
{
  psp_cpu_clock       = MANCALA.psp_cpu_clock;
  mancala_game_mode   = MANCALA.mancala_game_mode;
  mancala_game_rules  = MANCALA.mancala_game_rules;
  mancala_level       = MANCALA.mancala_level;
  mancala_init_stones = MANCALA.mancala_init_stones;
}

void
psp_main_menu_game_mode( int step )
{
  if (step > 0) {
    if (mancala_game_mode < MANCALA_COMPUTER_VS_COMPUTER) mancala_game_mode++;
    else                                         mancala_game_mode = 0;
  } else {
    if (mancala_game_mode > 0) mancala_game_mode--;
    else                  mancala_game_mode = MANCALA_COMPUTER_VS_COMPUTER;
  }
}

void
psp_main_menu_game_rules( int step )
{
  mancala_game_rules = ! mancala_game_rules;
}

void
psp_main_menu_level( int step )
{
  if (step > 0) {
    if (mancala_level < MANCALA_MAX_LEVEL) mancala_level++;
    else                                   mancala_level = 1;
  } else {
    if (mancala_level > 1) mancala_level--;
    else                   mancala_level = MANCALA_MAX_LEVEL;
  }
}

void
psp_main_menu_init_stones( int step )
{
  if (step > 0) {
    if (mancala_init_stones < MANCALA_MAX_STONES) mancala_init_stones++;
    else                                             mancala_init_stones = MANCALA_INIT_STONES;
  } else {
    if (mancala_init_stones > MANCALA_INIT_STONES) mancala_init_stones--;
    else                                                 mancala_init_stones = MANCALA_MAX_STONES;
  }
}

#define MAX_CLOCK_VALUES 5
static int clock_values[MAX_CLOCK_VALUES] = { 133, 222, 266, 300, 333 };

static void
psp_main_menu_clock(int step)
{
  int index;
  for (index = 0; index < MAX_CLOCK_VALUES; index++) {
    if (psp_cpu_clock == clock_values[index]) break;
  }
  if (step > 0) {
    index++;
    if (index >= MAX_CLOCK_VALUES) index = 0;
    psp_cpu_clock = clock_values[index];

  } else {
    index--;

    if (index < 0) index = MAX_CLOCK_VALUES - 1;
    psp_cpu_clock = clock_values[index];
  }
}

int 
psp_main_menu(void)
{
  SceCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  psp_kbd_wait_no_button();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  psp_main_menu_init();

  while (! end_menu)
  {
    psp_display_screen_menu();
    psp_sdl_flip();

    while (1)
    {
      myCtrlPeekBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;

      if (c.Buttons) break;
    }

    new_pad = c.Buttons;

    if ((old_pad != new_pad) || ((c.TimeStamp - last_time) > PSP_MENU_MIN_TIME)) {
      last_time = c.TimeStamp;
      old_pad = new_pad;

    } else continue;

    if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
        (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
      /* Exit ! */
      psp_sdl_exit(0);
    } else
    if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_SELECT)) == (PSP_CTRL_LTRIGGER|PSP_CTRL_SELECT)) {
      /* Ir shell ? */
      sceKernelDelayThread(100);
    } else
    if ((c.Buttons & PSP_CTRL_RTRIGGER) == PSP_CTRL_RTRIGGER) {
      psp_main_menu_new_game();
      end_menu = 1;
    } else
    if ((new_pad & PSP_CTRL_CROSS ) || 
        (new_pad & PSP_CTRL_CIRCLE) || 
        (new_pad & PSP_CTRL_RIGHT ) ||
        (new_pad & PSP_CTRL_LEFT  )) 
    {
      int step;

      if (new_pad & PSP_CTRL_LEFT)  step = -1;
      else 
      if (new_pad & PSP_CTRL_RIGHT) step =  1;
      else                          step =  0;

      switch (cur_menu_id ) 
      {
        case MENU_GAME_MODE    : psp_main_menu_game_mode( step );
        break;
        case MENU_GAME_RULES   : psp_main_menu_game_rules( step );
        break;
        case MENU_LEVEL        : psp_main_menu_level( step );
        break;
        case MENU_INIT_STONES : psp_main_menu_init_stones( step );
        break;
        case MENU_CLOCK        : psp_main_menu_clock( step );
        break;
        case MENU_SCREENSHOT   : psp_main_menu_screenshot();
                                 end_menu = 1;
        break;              

        case MENU_NEW_GAME     : psp_main_menu_new_game();
                                 end_menu = 1;
        break;                 
        case MENU_RESIGN_GAME  : psp_main_menu_resign_game();
                                 end_menu = 1;
        break;                 
        case MENU_SAVE_CFG     : psp_main_menu_save_config();
        break;               
        case MENU_BACK         : end_menu = 1;
        break;                 
        case MENU_EXIT         : psp_main_menu_exit();
        break;                 
        case MENU_HELP         : psp_help_menu();
                                 old_pad = new_pad = 0;
        break;              
      }

    } else
    if(new_pad & PSP_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_ITEM-1;

    } else
    if(new_pad & PSP_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_ITEM-1)) cur_menu_id++;
      else                                 cur_menu_id = 0;

    } else  
    if(new_pad & PSP_CTRL_SQUARE) {
      /* Cancel */
      end_menu = -1;
    } else 
    if((new_pad & PSP_CTRL_SELECT) == PSP_CTRL_SELECT) {
      /* Back to CPC */
      end_menu = 1;
    }
  }

  if (end_menu > 0) {
    psp_main_menu_validate();
  }
 
  psp_kbd_wait_no_button();

  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();
  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();

  return 1;
}

