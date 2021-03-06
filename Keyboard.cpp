#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dbus/dbus.h>

#include "utils/log.h"
#include "Keyboard.h"

Keyboard::Keyboard() {
	if (isatty(STDIN_FILENO)) {
		struct termios new_termios;

		tcgetattr(STDIN_FILENO, &orig_termios);

		new_termios = orig_termios;
		new_termios.c_lflag &= ~(ICANON | ECHO | ECHOCTL | ECHONL);
		new_termios.c_cflag |= HUPCL;
		new_termios.c_cc[VMIN] = 0;

		tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
	} else {		
		orig_fl = fcntl(STDIN_FILENO, F_GETFL);
		fcntl(STDIN_FILENO, F_SETFL, orig_fl | O_NONBLOCK);
	}

	if (dbus_connect() < 0) {
		CLog::Log(LOGWARNING, "DBus connection failed");
	} else {
		CLog::Log(LOGDEBUG, "DBus connection succeeded");
	}

	Create();
}

Keyboard::~Keyboard() {
	restore_term();
	dbus_disconnect();
	if (ThreadHandle()) {
		StopThread();
	}
}

void Keyboard::Close() {
	if (ThreadHandle())
		StopThread();
}

void Keyboard::restore_term() {
	if (isatty(STDIN_FILENO)) {
		tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
	} else {
		fcntl(STDIN_FILENO, F_SETFL, orig_fl);
	}
}

void Keyboard::Process() {
	while(!m_bStop && dbus_connection_read_write_dispatch(conn, 0)) {
		// sleep(10);
		int ch[8];
		int chnum = 0;

		while ((ch[chnum] = getchar()) != EOF) chnum++;

		if (chnum > 1) ch[0] = ch[chnum - 1] | (ch[chnum - 2] << 8);

		if (m_keymap[ch[0]] != 0)
					send_action(m_keymap[ch[0]]);
	}
}

void Keyboard::send_action(int action) {
	DBusMessage *message = NULL, *reply = NULL;
	DBusError error;

	dbus_error_init(&error);

	if (!(message = dbus_message_new_method_call(OMXPLAYER_DBUS_NAME, 
																							OMXPLAYER_DBUS_PATH_SERVER, 
																							OMXPLAYER_DBUS_INTERFACE_PLAYER,
																							"Action"))) {
		CLog::Log(LOGWARNING, "Keyboard: DBus error 1");
		goto fail;
	}

	dbus_message_append_args(message, DBUS_TYPE_INT32, &action, DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(conn, message, -1, &error);

	if (!reply || dbus_error_is_set(&error))
		goto fail;

	dbus_message_unref(message);
	dbus_message_unref(reply);

	return;

fail:
	if (dbus_error_is_set(&error)) {
		printf("%s", error.message);
		dbus_error_free(&error);
	}

	if (message)
		dbus_message_unref(message);

	if (reply)
		dbus_message_unref(reply);
}

void Keyboard::setKeymap(std::map<int,int> keymap) {
	m_keymap = keymap;
}

int Keyboard::dbus_connect() {
	DBusError error;

	dbus_error_init(&error);
	if (!(conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error))) {
		CLog::Log(LOGWARNING, "dbus_bus_get_private(): %s", error.message);
        goto fail;
	}

	dbus_connection_set_exit_on_disconnect(conn, FALSE);

  return 0;

fail:
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        conn = NULL;
    }

    return -1;

}

void Keyboard::dbus_disconnect() {
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        conn = NULL;
    }
}