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

# ifndef _PSP_GLOBAL_H_
# define _PSP_GLOBAL_H_


# define MAX_PATH   128
  extern char psp_homedir[MAX_PATH];

# define MANCALA_EMPTY  0
# define MANCALA_BLACK  1
# define MANCALA_WHITE  2

# define MANCALA_AWARI       0
# define MANCALA_MANCALA     1

# define MANCALA_HUMAN_VS_COMPUTER     0
# define MANCALA_HUMAN_VS_HUMAN        1
# define MANCALA_COMPUTER_VS_COMPUTER  2

# define MANCALA_MAX_STONES_IMG  10

# define MANCALA_NUM_PITS    12 
# define MANCALA_WIDTH           6
# define MANCALA_INIT_STONES     4
# define MANCALA_MAX_STONES      8

# define MANCALA_NORTH     1
# define MANCALA_SOUTH     0

# define MANCALA_MAX_LEVEL      10

  typedef struct MANCALA_t {

    int      psp_cpu_clock;
    int      psp_current_clock;
    int      psp_screenshot_mode;
    int      psp_screenshot_id;

    int      mancala_game_mode;
    int      mancala_game_rules;
    int      mancala_level;

    int      mancala_game_over;
    int      mancala_init_stones;

    int      xor_color;
    int      board_color;

    int      computer_player; /* SOUTH / NORTH */
    int      to_move;         /* SOUTH / NORTH */

    int      current_x[2]; /* SOUTH / NORTH */
    int      score[2];     /* SOUTH / NORTH */

    int      board_array[MANCALA_NUM_PITS];

  } MANCALA_t;

  extern MANCALA_t MANCALA;

  extern int psp_global_init();
  extern int psp_parse_configuration(void);
  extern int psp_save_configuration(void);

  extern int psp_exit_now;

# endif
