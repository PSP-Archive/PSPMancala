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
#include <zlib.h>
#include "SDL.h"

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspkernel.h>
#include <psppower.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "psp_global.h"
#include "psp_sdl.h"

  char psp_homedir[128];
  MANCALA_t MANCALA;

  int psp_exit_now  = 0;

void
myCtrlPeekBufferPositive( SceCtrlData* pc, int count )
{
  static long last_time = 0L;
  if (psp_exit_now) psp_sdl_exit(0);
  sceCtrlPeekBufferPositive( pc, count );
  /* too fast ? */
  if ((pc->TimeStamp - last_time) < 16000) {
    sceDisplayWaitVblankStart();
  }
  last_time = pc->TimeStamp;
}

void
myPowerSetClockFrequency(int cpu_clock)
{
  if (MANCALA.psp_current_clock != cpu_clock) {
    scePowerSetClockFrequency(cpu_clock, cpu_clock, cpu_clock/2);
    MANCALA.psp_current_clock = cpu_clock;
  }
}

void
myCtrlFastPeekBufferPositive( SceCtrlData* pc, int count )
{
  if (psp_exit_now) psp_sdl_exit(0);
  sceCtrlPeekBufferPositive( pc, count );
}


int
psp_global_reset()
{
  int Index;

  MANCALA.current_x[MANCALA_NORTH] = 0;
  MANCALA.current_x[MANCALA_SOUTH] = 0;
  MANCALA.score[MANCALA_NORTH] = 0;
  MANCALA.score[MANCALA_SOUTH] = 0;

  MANCALA.mancala_game_over = 0;
  MANCALA.computer_player = MANCALA_NORTH;
  MANCALA.to_move = MANCALA_SOUTH;
 
  for (Index = 0; Index < MANCALA_NUM_PITS; Index++) {
    MANCALA.board_array[Index] = MANCALA.mancala_init_stones;
  }

  return 0;
}

int
psp_global_init()
{
  memset(&MANCALA, sizeof(MANCALA_t), 0);

  MANCALA.xor_color     = psp_sdl_rgb(0xff, 0xff, 0xff);
  MANCALA.board_color    = 0x0;

  MANCALA.mancala_game_mode   = MANCALA_HUMAN_VS_COMPUTER;
  MANCALA.mancala_game_rules  = MANCALA_AWARI;
  MANCALA.mancala_level       = 1;

  MANCALA.psp_screenshot_mode = 0;
  MANCALA.mancala_init_stones = MANCALA_INIT_STONES;
  MANCALA.psp_cpu_clock       = 222;

  psp_parse_configuration();
 
  myPowerSetClockFrequency(MANCALA.psp_cpu_clock);

  psp_global_reset();

  return 0;
}

int
psp_save_configuration(void)
{
  char  FileName[MAX_PATH];
  FILE* FileDesc;
  int   error = 0;

  strcpy(FileName, psp_homedir);
  strcat(FileName, "/pspmancala.cfg");

  FileDesc = fopen(FileName, "w");
  if (FileDesc != (FILE *)0 ) {

    fprintf(FileDesc, "psp_cpu_clock=%d\n", MANCALA.psp_cpu_clock);
    fprintf(FileDesc, "mancala_game_mode=%d\n", MANCALA.mancala_game_mode);
    fprintf(FileDesc, "mancala_game_rules=%d\n", MANCALA.mancala_game_rules);
    fprintf(FileDesc, "mancala_level=%d\n", MANCALA.mancala_level);
    fprintf(FileDesc, "mancala_init_stones=%d\n", MANCALA.mancala_init_stones);

    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

int
psp_parse_configuration(void)
{
  char  FileName[MAX_PATH + 1];
  char  Buffer[512];
  char *Scan;
  unsigned int Value;
  FILE* FileDesc;

  sprintf(FileName, psp_homedir);
  strcat(FileName, "/pspmancala.cfg");

  FileDesc = fopen(FileName, "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    Scan = strchr(Buffer,'=');
    if (! Scan) continue;

    *Scan = '\0';
    Value = atoi(Scan+1);

    if (!strcasecmp(Buffer,"psp_cpu_clock")) MANCALA.psp_cpu_clock = Value;
    else
    if (!strcasecmp(Buffer,"mancala_game_mode")) MANCALA.mancala_game_mode = Value;
    else
    if (!strcasecmp(Buffer,"mancala_game_rules")) MANCALA.mancala_game_rules = Value;
    else
    if (!strcasecmp(Buffer,"mancala_level")) MANCALA.mancala_level = Value;
    else
    if (!strcasecmp(Buffer,"mancala_init_stones")) MANCALA.mancala_init_stones = Value;
  }

  fclose(FileDesc);

  return 0;
}


