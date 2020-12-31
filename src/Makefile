#
# Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
TARGET = pspmancala
PSPSDK = $(shell psp-config --pspsdk-path)
PSPBIN = $(shell psp-config --psp-prefix)/bin
SDL_CONFIG = $(PSPBIN)/sdl-config
OBJS = \
\
psp_main.o \
psp_global.o \
psp_mancala.o \
psp_font.o \
psp_sdl.o \
psp_kbd.o \
psp_play.o \
psp_menu.o \
psp_menu_help.o \
\
mancala.o \
minimax.o



DEFAULT_CFLAGS = $(shell $(SDL_CONFIG) --cflags) 

MORE_CFLAGS = -DHAVE_CONFIG_H -I. -O2 -Wall \
 -DNO_STDIO_REDIRECT \
 -DPSPMANCALA_VERSION=\"$(PSPMANCALA_VERSION)\" \
 -DPSPFW30X

PSP_FW_VERSION=303
PSPMANCALA_VERSION=1.1.0

CFLAGS = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
CXXFLAGS = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -fno-exceptions -fno-rtti

LIBS += -lpspwlan -lpspnet_inet -lSDL_image -lSDL_ttf -lSDL -lGL -lfreetype \
        -lpng -ljpeg -lpspvfpu -lpspirkeyb -lpsprtc -lpspaudio -lpspgu -lpsppower -lpsphprm -lz -lm 

BUILD_PRX=1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPMANCALA-v$(PSPMANCALA_VERSION)
PSP_EBOOT_ICON= pspmancala.png
PSP_EBOOT_SND0  = pspmancala.at3
PSP_EBOOT_PIC1 = pspmancala-pic.png

include $(PSPSDK)/lib/build.mak

all: EBOOT.PBP

install: all
	@mkdir -p /media/usbdisk/psp/game5XX/pspmancala/
	cp EBOOT.PBP /media/usbdisk/psp/game5XX/pspmancala/

realclean:
	/bin/rm -f $(OBJS) EBOOT.PBP PARAM.SFO

