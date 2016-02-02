#pragma once
#include "osal/osal_serial_port.h"
#include "logger.hpp"
#include "Engine.hpp"
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "config.hpp"

void* arduino_control_thr(void* ard_data_ptr);

class Arduino_ctrl
{
protected:	
OsalSerialPort *port;
char message[128];
bool connect;
public:
Arduino_ctrl(System &syst);
void send_command(Engine* engine);
void deinit();
bool isconnected();
};
