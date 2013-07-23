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

#define OMXPLAYER_DBUS_NAME "org.mpris.MediaPlayer2.omxplayer"
#define OMXPLAYER_DBUS_PATH_SERVER "/org/mpris/MediaPlayer2"  
#define OMXPLAYER_DBUS_INTERFACE_ROOT "org.mpris.MediaPlayer2"
#define OMXPLAYER_DBUS_INTERFACE_PLAYER "org.mpris.MediaPlayer2.Player"

#include <dbus/dbus.h>

class OMXControl
{
protected:
  struct termios orig_termios;
  int orig_fl;
  DBusConnection *bus;
public:
  OMXControl();
  ~OMXControl();
  int getEvent();
  void restore_term();
  bool IsPipe(const std::string& str);
  void dispatch();
private:
  int dbus_connect();
  void dbus_disconnect();
  static DBusHandlerResult msg_server(DBusConnection *c, DBusMessage *m, void *userdata);
  static DBusHandlerResult dbus_respond_ok(DBusConnection *c, DBusMessage *m);
  static DBusHandlerResult dbus_respond_int64(DBusConnection *c, DBusMessage *m, int64_t i);
  static DBusHandlerResult dbus_respond_boolean(DBusConnection *c, DBusMessage *m, int b);
  static DBusHandlerResult dbus_respond_string(DBusConnection *c, DBusMessage *m, const char *text);
};

