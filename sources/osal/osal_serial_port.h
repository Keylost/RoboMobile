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
 * COPYRIGHT (C) 2010-2015 ELECARD STB    ALL RIGHTS RESERVED.           *
\*************************************************************************/
// Author Mikhail.Karev@elecard.ru

/** Serial port API */

#ifndef OSAL_SERIAL_PORT__H
#define OSAL_SERIAL_PORT__H

#include "osal.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum OsalSerialPortMode {
	OSAL_SERIAL_MODE_5N1,
	OSAL_SERIAL_MODE_5N2,
	OSAL_SERIAL_MODE_5E1,
	OSAL_SERIAL_MODE_5E2,
	OSAL_SERIAL_MODE_5O1,
	OSAL_SERIAL_MODE_5O2,

	OSAL_SERIAL_MODE_6N1,
	OSAL_SERIAL_MODE_6N2,
	OSAL_SERIAL_MODE_6E1,
	OSAL_SERIAL_MODE_6E2,
	OSAL_SERIAL_MODE_6O1,
	OSAL_SERIAL_MODE_6O2,

	OSAL_SERIAL_MODE_7N1,
	OSAL_SERIAL_MODE_7N2,
	OSAL_SERIAL_MODE_7E1,
	OSAL_SERIAL_MODE_7E2,
	OSAL_SERIAL_MODE_7O1,
	OSAL_SERIAL_MODE_7O2,

	OSAL_SERIAL_MODE_8N1,
	OSAL_SERIAL_MODE_8N2,
	OSAL_SERIAL_MODE_8E1,
	OSAL_SERIAL_MODE_8E2,
	OSAL_SERIAL_MODE_8O1,
	OSAL_SERIAL_MODE_8O2,
} OsalSerialPortMode;

	typedef struct OsalSerialPort OsalSerialPort;

	OsalSerialPort* osal_serial_port_open(const char *name, int baudrate, OsalSerialPortMode mode);
	void osal_serial_port_close(OsalSerialPort *port);

	size_t osal_serial_port_read(OsalSerialPort *port, char *buffer, size_t size);
	size_t osal_serial_port_write(OsalSerialPort *port, const char *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif//OSAL_SERIAL_PORT__H
