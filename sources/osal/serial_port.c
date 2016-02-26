/*************************************************************************\
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY *
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE   *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR *
 * PURPOSE.                                                              *
 *                                                                       *
 * NO PART OF THIS PUBLICATION MAY BE REPRODUCED OR UTILIZED IN ANY FORM *
 * OR BY ANY MEANS, ELECTRONIC OR MECHANICAL, WITHOUT PERMISSION IN      *
 * WRITING FORM FROM THE AUTHOR.                                         *
 *                                                                       *
 * COPYRIGHT (C) 2010-2014 ELECARD STB    ALL RIGHTS RESERVED.           *
\*************************************************************************/
// Author Mikhail.Karev@elecard.ru

/** Docs: https://www.cmrr.umn.edu/~strupp/serial.html */

#define OSAL_LOG_TAG "serial-port"
#include "osal.h"
#include "osal_serial_port.h"
#include "osal_log.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct OsalSerialPort
{
	int baudrate;
	OsalSerialPortMode mode;
	struct termios settings;
	int fd;
};

static bool init_baudrate(OsalSerialPort *port)
{
	int baudrate;
	switch (port->baudrate) {
		case 0: baudrate = B0; break;
		case 50: baudrate = B50; break;
		case 75: baudrate = B75; break;
		case 110: baudrate = B110; break;
		case 134: baudrate = B134; break;
		case 150: baudrate = B150; break;
		case 200: baudrate = B200; break;
		case 300: baudrate = B300; break;
		case 600: baudrate = B600; break;
		case 1200: baudrate = B1200; break;
		case 1800: baudrate = B1800; break;
		case 2400: baudrate = B2400; break;
		case 4800: baudrate = B4800; break;
		case 9600: baudrate = B9600; break;
		case 19200: baudrate = B19200; break;
		case 38400: baudrate = B38400; break;
		case 57600: baudrate = B57600; break;
		//case 76800: baudrate = B76800; break; // Android missing
		case 115200: baudrate = B115200; break;
		default:
			LOGE("baudrate %d is invalid\n", port->baudrate);
			return false;
	}
	if (cfsetispeed(&port->settings, baudrate) < 0) {
		LOGE("cfsetispeed failed\n");
		return false;
	}
	if (cfsetospeed(&port->settings, baudrate) < 0) {
		LOGE("cfsetospeed failed\n");
		return false;
	}
	return true;
}

static bool init_data_bits(OsalSerialPort *port)
{
	port->settings.c_cflag &= ~CSIZE;
	switch (port->mode) {
		case OSAL_SERIAL_MODE_5N1:
		case OSAL_SERIAL_MODE_5N2:
		case OSAL_SERIAL_MODE_5E1:
		case OSAL_SERIAL_MODE_5E2:
		case OSAL_SERIAL_MODE_5O1:
		case OSAL_SERIAL_MODE_5O2:
			port->settings.c_cflag |= CS5;
			break;

		case OSAL_SERIAL_MODE_6N1:
		case OSAL_SERIAL_MODE_6N2:
		case OSAL_SERIAL_MODE_6E1:
		case OSAL_SERIAL_MODE_6E2:
		case OSAL_SERIAL_MODE_6O1:
		case OSAL_SERIAL_MODE_6O2:
			port->settings.c_cflag |= CS6;
			break;

		case OSAL_SERIAL_MODE_7N1:
		case OSAL_SERIAL_MODE_7N2:
		case OSAL_SERIAL_MODE_7E1:
		case OSAL_SERIAL_MODE_7E2:
		case OSAL_SERIAL_MODE_7O1:
		case OSAL_SERIAL_MODE_7O2:
			port->settings.c_cflag |= CS7;
			break;

		case OSAL_SERIAL_MODE_8N1:
		case OSAL_SERIAL_MODE_8N2:
		case OSAL_SERIAL_MODE_8E1:
		case OSAL_SERIAL_MODE_8E2:
		case OSAL_SERIAL_MODE_8O1:
		case OSAL_SERIAL_MODE_8O2:
			port->settings.c_cflag |= CS8;
			break;

		default:
			LOGE("mode is invalid\n");
			return false;
	}
	return true;
}

static bool init_stop_bits(OsalSerialPort *port)
{
	switch (port->mode) {
		case OSAL_SERIAL_MODE_5N1:
		case OSAL_SERIAL_MODE_5E1:
		case OSAL_SERIAL_MODE_5O1:
		case OSAL_SERIAL_MODE_6N1:
		case OSAL_SERIAL_MODE_6E1:
		case OSAL_SERIAL_MODE_6O1:
		case OSAL_SERIAL_MODE_7N1:
		case OSAL_SERIAL_MODE_7E1:
		case OSAL_SERIAL_MODE_7O1:
		case OSAL_SERIAL_MODE_8N1:
		case OSAL_SERIAL_MODE_8E1:
		case OSAL_SERIAL_MODE_8O1:
			port->settings.c_cflag &= ~CSTOPB;
			break;

		case OSAL_SERIAL_MODE_5N2:
		case OSAL_SERIAL_MODE_5E2:
		case OSAL_SERIAL_MODE_5O2:
		case OSAL_SERIAL_MODE_6N2:
		case OSAL_SERIAL_MODE_6E2:
		case OSAL_SERIAL_MODE_6O2:
		case OSAL_SERIAL_MODE_7N2:
		case OSAL_SERIAL_MODE_7E2:
		case OSAL_SERIAL_MODE_7O2:
		case OSAL_SERIAL_MODE_8N2:
		case OSAL_SERIAL_MODE_8E2:
		case OSAL_SERIAL_MODE_8O2:
			port->settings.c_cflag |= CSTOPB;
			break;

		default:
			LOGE("mode is invalid\n");
			return false;
	}
	return true;
}

static bool init_parity(OsalSerialPort *port)
{
	port->settings.c_cflag |= PARENB;
	switch (port->mode) {
		case OSAL_SERIAL_MODE_5N1:
		case OSAL_SERIAL_MODE_5N2:
		case OSAL_SERIAL_MODE_6N1:
		case OSAL_SERIAL_MODE_6N2:
		case OSAL_SERIAL_MODE_7N1:
		case OSAL_SERIAL_MODE_7N2:
		case OSAL_SERIAL_MODE_8N1:
		case OSAL_SERIAL_MODE_8N2:
			port->settings.c_cflag &= ~PARENB;
			break;

		case OSAL_SERIAL_MODE_5E1:
		case OSAL_SERIAL_MODE_5E2:
		case OSAL_SERIAL_MODE_6E1:
		case OSAL_SERIAL_MODE_6E2:
		case OSAL_SERIAL_MODE_7E1:
		case OSAL_SERIAL_MODE_7E2:
		case OSAL_SERIAL_MODE_8E1:
		case OSAL_SERIAL_MODE_8E2:
			port->settings.c_cflag &= ~PARODD;
			break;

		case OSAL_SERIAL_MODE_5O1:
		case OSAL_SERIAL_MODE_5O2:
		case OSAL_SERIAL_MODE_6O1:
		case OSAL_SERIAL_MODE_6O2:
		case OSAL_SERIAL_MODE_7O1:
		case OSAL_SERIAL_MODE_7O2:
		case OSAL_SERIAL_MODE_8O1:
		case OSAL_SERIAL_MODE_8O2:
			port->settings.c_cflag |= PARODD;
			break;

		default:
			LOGE("mode is invalid\n");
			return false;
	}
	/**
	   You should enable input parity checking when you have enabled parity in the c_cflag member (PARENB).
	   The revelant constants for input parity checking are INPCK, IGNPAR, PARMRK , and ISTRIP.
	   Generally you will select INPCK and ISTRIP to enable checking and stripping of the parity bit
	 */
	if (port->settings.c_cflag & PARENB) {
		port->settings.c_iflag |= (INPCK | ISTRIP);
	}
	return true;
}

OsalSerialPort* osal_serial_port_open(const char *name, int baudrate, OsalSerialPortMode mode)
{
	OsalSerialPort *port = (OsalSerialPort *)calloc(1, sizeof(*port));
	if (!port) {
		LOGE("Can't alloc serial port\n");
		goto fail;
	}
	port->baudrate = baudrate;
	port->mode = mode;
	port->fd = -1;
	
	if (!init_data_bits(port)) {
		LOGE("init_data_bits failed\n");
		goto fail;
	}
	if (!init_stop_bits(port)) {
		LOGE("init_stop_bits failed\n");
		goto fail;
	}
	if (!init_parity(port)) {
		LOGE("init_parity failed\n");
		goto fail;
	}
	if (!init_baudrate(port)) {
		LOGE("init_baudrate failed\n");
		goto fail;
	}

	/**
	  The c_cflag member contains two options that should always be enabled, CLOCAL and CREAD.
	  These will ensure that your program does not become the 'owner' of the port
	  subject to sporatic job control and hangup signals,
	  and also that the serial interface driver will read incoming data bytes. 
	 */
	port->settings.c_cflag |= ( CLOCAL | CREAD );

	// initialize control characters 
	port->settings.c_cc[VMIN] = 1; /* blocking read until 1 character arrives */
	port->settings.c_cc[VTIME] = 0;

	/**
		The O_NOCTTY flag tells UNIX that this program doesn't want to be the "controlling terminal" for that port.
		If you don't specify this then any input (such as keyboard abort signals and so forth) will affect your process.
		Programs like getty(1M/8) use this feature when starting the login process,
		but normally a user program does not want this behavior.

		The O_NDELAY flag tells UNIX that this program doesn't care what state the DCD signal
		line is in - whether the other end of the port is up and running.
		If you do not specify this flag, your process will be put to sleep until the DCD signal line is the space voltage.
	*/
	port->fd = open(name, O_RDWR | O_NOCTTY | O_NDELAY);
	if (port->fd == -1) {
		LOGE("Can't open serial port \"%s\"\n", name);
		goto fail;
	}
	fcntl(port->fd, F_SETFL, FNDELAY); //0

	// flush serial port and activate settings
	if (tcsetattr(port->fd, TCSANOW, &port->settings) < 0 ) {
		LOGE("tcsetattr(\"%s\") failed\n", name);
		goto fail;
	}
	tcflush(port->fd, TCIOFLUSH);
	return port;

fail:
	osal_serial_port_close(port);
	return NULL;
}

void osal_serial_port_close(OsalSerialPort *port)
{
	if (port) {
		if (port->fd != -1) {
			close(port->fd);
			port->fd = -1;
		}
		free(port);
	}
}
#include <sys/ioctl.h>
size_t osal_serial_port_write(OsalSerialPort *port, const char *buffer, size_t size)
{
	/**
		Writing data to the port is easy - just use the write(2) system call to send data it
	 */
	int bytes_written = write(port->fd, buffer, size);

	ioctl(port->fd, TCSBRK, 1);//tcdrain(engine.port);
	return bytes_written == -1 ? 0 : bytes_written;
}

size_t osal_serial_port_read(OsalSerialPort *port, char *buffer, size_t size)
{
	/**
		Reading data from a port is a little trickier.
		When you operate the port in raw data mode,
		each read(2) system call will return however many characters
		are actually available in the serial input buffers.
		If no characters are available, the call will block (wait) until characters come in,
		an interval timer expires, or an error occurs.
		The read function can be made to return immediately by doing the following:

    		fcntl(fd, F_SETFL, FNDELAY);

		The FNDELAY option causes the read function to return 0 if no characters are available on the port.
		To restore normal (blocking) behavior, call fcntl() without the FNDELAY option:

    		fcntl(fd, F_SETFL, 0);

		This is also used after opening a serial port with the O_NDELAY option. 
	 */
	return read(port->fd, buffer, size);
}
