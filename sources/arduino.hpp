#pragma once
#include "logger.hpp"
#include "Engine.hpp"
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "config.hpp"
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

class ArduinoCtrl
{
protected:
int arduino_fd; //дескриптор устройства
char message[128];
bool connectionStatus;

bool connect(const char* arduinoPort);
bool disconnect();

public:
ArduinoCtrl(System &syst);
int feedback();
void sendCommand(const char *message,size_t size);
void deinit();
bool isConnected();
};

void* arduino_fnc(void *ptr);
