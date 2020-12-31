/*
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
#include <string.h>
#include <math.h>
#include <SDL/SDL.h>
#include <pspctrl.h>
#include <psptypes.h>
#include <png.h>

#include "psp_sdl.h"
#include "psp_global.h"
#include "psp_kbd.h"
#include "psp_play.h"
#include "psp_mancala.h"
#include "psp_menu.h"

  static int psp_mancala_board_dirty = 1;

int
psp_mancala_is_computer_move()
{
  if (MANCALA.mancala_game_over) return 0;

  if (MANCALA.mancala_game_mode == MANCALA_HUMAN_VS_COMPUTER) {
    return (MANCALA.computer_player == MANCALA.to_move);
  } else 
  if (MANCALA.mancala_game_mode == MANCALA_COMPUTER_VS_COMPUTER) {
    return 1;
  }
  return 0;
}

void
psp_mancala_display_board()
{
  int  x_orig;
  int  y_orig;
  int  x_north;
  int  x_south;
  int  x_dst;
  int  y_dst;
  int  x;
  int  num_stones;

  int  square_width_pix = 480 / MANCALA_WIDTH;
  int  board_width_pix = MANCALA_WIDTH * square_width_pix;

  x_orig = 0;
  y_orig = 56;

  psp_sdl_blit_board();
  psp_sdl_draw_rectangle(x_orig, y_orig-1, board_width_pix, (square_width_pix * 2)+1, MANCALA.board_color, 0);

  /* Draw board up : north */
  y_dst = y_orig;
  for (x = 0; x < MANCALA_WIDTH; x++) {
    x_dst = x_orig + (x * square_width_pix);
    psp_sdl_blit_stone_xy(x_dst, y_dst, 0);
    num_stones = MANCALA.board_array[(MANCALA_NUM_PITS - 1) - x];
    psp_sdl_blit_stone_xy(x_dst, y_dst, num_stones);
  }
  /* Draw board bottom : south */
  y_dst = y_dst + square_width_pix;
  for (x = 0; x < MANCALA_WIDTH; x++) {
    x_dst = x_orig + (x * square_width_pix);
    psp_sdl_blit_stone_xy(x_dst, y_dst, 0);
    num_stones = MANCALA.board_array[x];
    psp_sdl_blit_stone_xy(x_dst, y_dst, num_stones);
  }

  x_north = MANCALA.current_x[MANCALA_NORTH];
  x_south = MANCALA.current_x[MANCALA_SOUTH];

  psp_sdl_blit_arrow_xy( (x_north * square_width_pix) + square_width_pix/2, y_orig, 0);
  psp_sdl_blit_arrow_xy( (x_south * square_width_pix) + square_width_pix/2, y_orig + 2*square_width_pix, 1);
  psp_sdl_draw_rectangle( 0, 56, 0, (square_width_pix * 2), MANCALA.board_color, 0);
}

void
psp_mancala_display_status()
{
  char  buffer[128];

  int   x_orig = 10;
  int   y_orig = 10;

  /* Game mode */
  if (MANCALA.mancala_game_mode == MANCALA_HUMAN_VS_COMPUTER) strcpy(buffer, "Human vs PSP");
  else
  if (MANCALA.mancala_game_mode == MANCALA_HUMAN_VS_HUMAN)    strcpy(buffer, "Human vs Human");
  else                                                        strcpy(buffer, "PSP vs PSP");

  psp_sdl_print(x_orig, y_orig, buffer, PSP_MENU_YELLOW_COLOR);
  y_orig += 10;

  if (MANCALA.mancala_game_over) {
    psp_sdl_print(x_orig, y_orig, "GAME OVER !", PSP_MENU_YELLOW_COLOR);
  } else
  if (psp_mancala_is_computer_move()) {
    psp_sdl_print(x_orig, y_orig, "CPU Thinking", PSP_MENU_WHITE_COLOR);
  }
  y_orig += 10;

  if (MANCALA.to_move == MANCALA_NORTH) {
    psp_sdl_print(x_orig, y_orig, "North to play", PSP_MENU_NORTH_COLOR);
  } else {
    psp_sdl_print(x_orig, y_orig, "South to play", PSP_MENU_SOUTH_COLOR);
  }

  y_orig = 240;
  sprintf(buffer, "North %02d South %02d", MANCALA.score[MANCALA_NORTH], MANCALA.score[MANCALA_SOUTH]);
  psp_sdl_print(x_orig, y_orig, buffer, PSP_MENU_BLACK_COLOR);
}

void
psp_mancala_refresh_screen()
{
  psp_mancala_display_board();
  psp_mancala_display_status();
}

void
psp_mancala_mancala_left()
{
  int *p_current = &MANCALA.current_x[MANCALA.to_move];
  (*p_current)--;
  if ( *p_current < 0) *p_current = MANCALA_WIDTH - 1;
  psp_mancala_board_dirty = 1;
}

void
psp_mancala_mancala_right()
{
  int *p_current = &MANCALA.current_x[MANCALA.to_move];
  (*p_current)++;
  if ( *p_current >= MANCALA_WIDTH) *p_current = 0;
  psp_mancala_board_dirty = 1;
}

void
psp_mancala_new_game()
{
  psp_global_reset();
  psp_play_reset();
}

void
psp_mancala_resign_game()
{
  if (MANCALA.mancala_game_over) return;
  MANCALA.mancala_game_over = 2;
}

void
psp_mancala_game_over()
{
  if (MANCALA.mancala_game_over != 1) {
    psp_play_compute_final_score();
  }
  psp_mancala_refresh_screen();

  if (MANCALA.mancala_game_over == 1) return;
  MANCALA.mancala_game_over = 1;

  psp_sdl_print(200,  240, "Game over press X to continue ...", PSP_MENU_RED_COLOR);
  psp_sdl_flip();

  psp_kbd_wait_no_button();
  while (1)
  {
    SceCtrlData c;
    myCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;
    if (c.Buttons & PSP_CTRL_CROSS) break;
  }
  psp_kbd_wait_no_button();
  psp_mancala_board_dirty = 1;
}

void
psp_mancala_play()
{
  if (MANCALA.mancala_game_over) return;

  psp_play_player_move();
  psp_mancala_board_dirty = 1;
}

void
psp_mancala_main_loop()
{
  SceCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;

  old_pad   = 0;
  last_time = 0;

  psp_main_menu();

  while (1) {

    do {

      if (MANCALA.psp_screenshot_mode) {
        MANCALA.psp_screenshot_mode = 0;
        psp_mancala_refresh_screen();
        psp_sdl_flip();
        psp_mancala_refresh_screen();
        psp_sdl_flip();
        psp_sdl_save_screenshot();
      }

      if (psp_mancala_board_dirty) {
        psp_mancala_board_dirty = 0;
        psp_mancala_refresh_screen();
        psp_sdl_flip();
      }

      if (psp_mancala_is_computer_move()) {

        if (psp_play_computer_move()) {
          psp_mancala_refresh_screen();
          psp_sdl_flip();

          if (MANCALA.mancala_game_mode == MANCALA_COMPUTER_VS_COMPUTER) {
            MANCALA.computer_player = ! MANCALA.computer_player;
          }
        }
      }

      if (psp_play_is_game_over()) {
        /* Game OVER ! */
        psp_mancala_game_over();
      }

    } while (psp_mancala_board_dirty);

    myCtrlFastPeekBufferPositive(&c, 1);
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
    if((new_pad & PSP_CTRL_SELECT) == PSP_CTRL_SELECT) {
      psp_main_menu();
      psp_mancala_board_dirty = 1;
    } else {

      if (new_pad & PSP_CTRL_CROSS) {
        psp_mancala_play();
      } else
      if (new_pad & PSP_CTRL_LEFT) {
        psp_mancala_mancala_left();
      }
      if (new_pad & PSP_CTRL_RIGHT) {
        psp_mancala_mancala_right();
      }
    }
  }
}

int 
SDL_main(int argc,char *argv[])
{
  psp_sdl_init();
  psp_global_init();
  psp_play_init();

  psp_mancala_main_loop();

  psp_sdl_exit(0);

  return 0;
}

