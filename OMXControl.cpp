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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dbus/dbus.h>

#include <string>
#include <utility>

#include "utils/log.h"
#include "OMXControl.h"
#include "KeyConfig.h"

#define CLASSNAME "OMXControl"

OMXControl::OMXControl() {
	if (dbus_connect() < 0) {
		CLog::Log(LOGWARNING, "DBus connection failed");
	} else {
		CLog::Log(LOGDEBUG, "DBus connection succeeded");
	}
    	
}

OMXControl::~OMXControl() {
    dbus_disconnect();
}

void OMXControl::dispatch() {
	dbus_connection_read_write_dispatch(bus, 0);
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

int OMXControl::dbus_connect() {
	DBusError error;

	dbus_error_init(&error);
	if (!(bus = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error))) {
		printf("get bus private: %s\n", error.message);
		CLog::Log(LOGWARNING, "dbus_bus_get_private(): %s", error.message);
        goto fail;
	} else {
		printf("Got bus\n");
	}

	dbus_connection_set_exit_on_disconnect(bus, FALSE);

	if (dbus_bus_request_name(
        bus,
        OMXPLAYER_DBUS_NAME,
        DBUS_NAME_FLAG_DO_NOT_QUEUE,
        &error) != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        if (dbus_error_is_set(&error)) {
            CLog::Log(LOGWARNING, "dbus_bus_request_name(): %s", error.message);
            goto fail;
        }

        CLog::Log(LOGWARNING, "Failed to acquire D-Bus name '%s'", OMXPLAYER_DBUS_NAME);
        goto fail;
    } else {
    	printf("Got bus name\n");
    }

    return 0;

fail:
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    if (bus) {
        dbus_connection_close(bus);
        dbus_connection_unref(bus);
        bus = NULL;
    }

    return -1;
}

void OMXControl::dbus_disconnect() {
    if (bus) {
        dbus_connection_close(bus);
        dbus_connection_unref(bus);
        bus = NULL;
    }
}

DBusHandlerResult OMXControl::msg_server(DBusConnection *c, DBusMessage *m, void *userdata) {
	DBusError error;

	dbus_error_init(&error);

	CLog::Log(LOGDEBUG, "DBus interface=%s, path=%s, member=%s",
						dbus_message_get_interface(m),
						dbus_message_get_path(m),
						dbus_message_get_member(m));

	printf("DBus interface=%s, path=%s, member=%s\n",
						dbus_message_get_interface(m),
						dbus_message_get_path(m),
						dbus_message_get_member(m));

	if (dbus_message_is_method_call(m, OMXPLAYER_DBUS_INTERFACE_ROOT, "Quit")) {
		printf("Quit method called\n");
		return OMXControl::dbus_respond_ok(c, m);
	}

//fail:
	if (dbus_error_is_set(&error))
		dbus_error_free(&error);

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

int OMXControl::getEvent() {
	dispatch();

	DBusMessage *m = dbus_connection_pop_message(bus);

	if (m == NULL) 
		return KeyConfig::ACTION_BLANK;

	if (dbus_message_is_method_call(m, OMXPLAYER_DBUS_INTERFACE_ROOT, "Quit")) {
		OMXControl::dbus_respond_ok(bus, m);
		return KeyConfig::ACTION_EXIT;
	} else if (dbus_message_is_method_call(m, DBUS_INTERFACE_PROPERTIES, "CanQuit")) {
		OMXControl::dbus_respond_boolean(bus, m, 1);
		return KeyConfig::ACTION_BLANK;
	} else if (dbus_message_is_method_call(m, DBUS_INTERFACE_PROPERTIES, "Fullscreen")) {
		OMXControl::dbus_respond_boolean(bus, m, 1);
		return KeyConfig::ACTION_BLANK;
	} else if (dbus_message_is_method_call(m, DBUS_INTERFACE_PROPERTIES, "CanSetFullscreen")) {
		OMXControl::dbus_respond_boolean(bus, m, 0);
		return KeyConfig::ACTION_BLANK;
	} else if (dbus_message_is_method_call(m, DBUS_INTERFACE_PROPERTIES, "CanRaise")) {
		OMXControl::dbus_respond_boolean(bus, m, 0);
		return KeyConfig::ACTION_BLANK;
	} else if (dbus_message_is_method_call(m, DBUS_INTERFACE_PROPERTIES, "HasTrackList")) {
		OMXControl::dbus_respond_boolean(bus, m, 0);
		return KeyConfig::ACTION_BLANK;
	} else if (dbus_message_is_method_call(m, DBUS_INTERFACE_PROPERTIES, "Identity")) {
		OMXControl::dbus_respond_string(bus, m, "OMXPlayer");
		return KeyConfig::ACTION_BLANK;
	}

	/*
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
			return KeyConfig::ACTION_BLANK;
			break;
	}
*/
	return KeyConfig::ACTION_BLANK;
}

DBusHandlerResult OMXControl::dbus_respond_ok(DBusConnection *c, DBusMessage *m) {
	DBusMessage *reply;

	reply = dbus_message_new_method_return(m);

	if (!reply) {
		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	}

	dbus_connection_send(c, reply, NULL);
	dbus_message_unref(reply);

	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult OMXControl::dbus_respond_string(DBusConnection *c, DBusMessage *m, const char *text) {
	DBusMessage *reply;

	reply = dbus_message_new_method_return(m);

	if (!reply) {
		CLog::Log(LOGWARNING, "Failed to allocate message");
		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	}

	dbus_message_append_args(reply, DBUS_TYPE_STRING, &text, DBUS_TYPE_INVALID);
	dbus_connection_send(c, reply, NULL);
	dbus_message_unref(reply);

	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult OMXControl::dbus_respond_int64(DBusConnection *c, DBusMessage *m, int64_t i) {
	DBusMessage *reply;

	reply = dbus_message_new_method_return(m);

	if (!reply) {
		CLog::Log(LOGWARNING, "Failed to allocate message");
		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	}

	dbus_message_append_args(reply, DBUS_TYPE_INT64, &i, DBUS_TYPE_INVALID);
	dbus_connection_send(c, reply, NULL);
	dbus_message_unref(reply);

	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult OMXControl::dbus_respond_boolean(DBusConnection *c, DBusMessage *m, int b) {
	DBusMessage *reply;

	reply = dbus_message_new_method_return(m);

	if (!reply) {
		CLog::Log(LOGWARNING, "Failed to allocate message");
		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	}

	dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &b, DBUS_TYPE_INVALID);
	dbus_connection_send(c, reply, NULL);
	dbus_message_unref(reply);

	return DBUS_HANDLER_RESULT_HANDLED;
}