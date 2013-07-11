/*
* XBMC Media Center
* Copyright (c) 2002 d7o3g4q and RUNTiME
* Portions Copyright (c) by the authors of ffmpeg and xvid
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <termios.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <string>
#include <utility>

#include "OMXControl.h"

#define CLASSNAME "OMXControl"

OMXControl::OMXControl() {
  // Make getchar unbuffered
  if (isatty(STDIN_FILENO))
    {
      struct termios new_termios;

      tcgetattr(STDIN_FILENO, &orig_termios);

      new_termios             = orig_termios;
      new_termios.c_lflag     &= ~(ICANON | ECHO | ECHOCTL | ECHONL);
      new_termios.c_cflag     |= HUPCL;
      new_termios.c_cc[VMIN]  = 0;

      tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    }
    else
    {
      orig_fl = fcntl(STDIN_FILENO, F_GETFL);
      fcntl(STDIN_FILENO, F_SETFL, orig_fl | O_NONBLOCK);
    }
}

OMXControl::~OMXControl() {
  this->restore_term();
}

void OMXControl::restore_term() {
  if (isatty(STDIN_FILENO))
  {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
  }
  else
  {
    fcntl(STDIN_FILENO, F_SETFL, orig_fl);
  }
}

OMXEvent OMXControl::getEvent() {
  int num_keys = 0;
  int keys[8];

  while((keys[num_keys] = getchar()) != EOF) num_keys++;

  if (num_keys > 1) keys[0] = keys[num_keys - 1] | (keys[num_keys - 2] << 8);
  // Should be enough to uniquely identify multi-byte key presses

  switch (keys[0]) {
    case 'z':
      return SHOW_INFO;
    case '1':
      return SPEED_DOWN;
    case '2':
      return SPEED_UP;
    case ',': case '<':
      return REWIND;
    case '.': case '>':
      return FFORWARD;
    case 'v':
      return STEP;
    case 'j':
      return PREV_AUDIO;
    case 'k':
      return NEXT_AUDIO;
    case 'i':
      return PREV_CHAPTER;
    case 'o':
      return NEXT_CHAPTER;
    case 'n':
      return PREV_SUBTITLE;
    case 'm':
      return NEXT_SUBTITLE;
    case 's':
      return TOGGLE_SUBTITLES;
    case 'd':
      return DEC_SUB_DELAY;
    case 'f':
      return INC_SUB_DELAY;
    case 'q': case 27:
      return QUIT;
    case 0x5b44:
      return SEEK_LEFT_30;
    case 0x5b43:
      return SEEK_RIGHT_30;
    case 0x5b41:
      return SEEK_LEFT_600;
    case 0x5b42:
      return SEEK_RIGHT_600;
    case ' ': case 'p':
      return PLAY_PAUSE;
    case '-':
      return DEC_VOLUME;
    case '+': case '=':
      return INC_VOLUME;
    default:
      return BLANK;
      break;
  }
}