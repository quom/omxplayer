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

//////////////////////////////////////////////////////////////////////

enum OMXEvent { 
  SPEED_UP, SPEED_DOWN,
  REWIND, FFORWARD,
  SHOW_INFO, EXIT_OMX,
  PREV_AUDIO, NEXT_AUDIO,
  PREV_CHAPTER, NEXT_CHAPTER,
  PREV_SUBTITLE, NEXT_SUBTITLE,
  TOGGLE_SUBTITLES,
  DEC_SUB_DELAY, INC_SUB_DELAY,
  PLAY_PAUSE, STEP, QUIT,
  DEC_VOLUME, INC_VOLUME,
  SEEK_LEFT_30, SEEK_RIGHT_30,
  SEEK_LEFT_600, SEEK_RIGHT_600,
  BLANK
};

class OMXControl
{
protected:
  struct termios orig_termios;
  int orig_fl;
public:
  OMXControl();
  ~OMXControl();
  OMXEvent getEvent();
  static void getMsg();
  void restore_term();
  bool IsPipe(const std::string& str);
private:



};

