/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#define OMXPLAYER_DBUS_NAME "org.mpris.MediaPlayer2.omxplayer"
#define OMXPLAYER_DBUS_PATH_SERVER "/org/mpris/MediaPlayer2"  
#define OMXPLAYER_DBUS_INTERFACE_ROOT "org.mpris.MediaPlayer2"
#define OMXPLAYER_DBUS_INTERFACE_PLAYER "org.mpris.MediaPlayer2.Player"

#include "OMXThread.h"
#include <map>

 class Keyboard : public OMXThread
 {
 protected:
 	struct termios orig_termios;
 	int orig_fl;
 	DBusConnection *conn;
 	std::map<int,int> m_keymap;
 public:
 	Keyboard();
 	~Keyboard();
 	void Close();
 	void Process();
 	void setKeymap(std::map<int,int> keymap);
 private:
 	void restore_term();
 	void send_action(int action);
 	int dbus_connect();
 	void dbus_disconnect();
 };