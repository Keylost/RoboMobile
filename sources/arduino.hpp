#pragma once
#include "osal/osal_serial_port.h"
#include "logger.hpp"
#include "Engine.hpp"
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "config.hpp"

class Arduino_ctrl
{
protected:	
OsalSerialPort *port;
char message[128];
bool connect;
public:
Arduino_ctrl(System &syst);
int feedback();
void send_command(Engine* engine);
void deinit();
bool isconnected();
};
