#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include "Engine.hpp"
#include "logger.hpp"
#include "config.hpp"

using namespace cv;
using namespace std;

enum dataType
{
	Image_t = 0,
	Engine_t = 1,
	Sing_t = 2,
	Line_t = 3
};

/*
 * Данный класс реализует серверные функции системы.
 */
class Server
{
private:
	System *sys;
	bool is_send;
	int sockfd;
	int portno;
	void start();
public:
	int newsockfd;
	bool isconnected;
	Engine *engine;
	
	Server(System &syst);
	void stop();
	void send(dataType type, uint32_t dataSize, void *ptr);
	void receiver(Engine *eng);
};


void* server_fnc(void *ptr);
