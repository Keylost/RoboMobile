/*
 * Данный класс реализует серверные функции системы.
 * Формат общения с клиентом после установки связи:
 * 1. Отправляет сведения о конфигурации:
 * ширина кадра - 2 байта (uint16_t)
 * высота кадра - 2 байта (uint16_t)
 * 
 * область поиска знака - cv::Rect (sizeof(cv::Rect))
 * область поиска линии - cv::Rect (sizeof(cv::Rect))
 */
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
#include "telemetry.hpp"
#include "config.hpp"

using namespace cv;
using namespace std;

class Server
{
private:
	System *sys;
	bool is_send;
	int sockfd;
	int portno;
	std::vector<int> parameters; //parameters vector for image compressor(jpeg 20%)
	std::vector<uchar> buffer; //compressed image vector
	Telemetry telemetry; //telemetry data	
	void start();
public:
	int newsockfd;
	bool isconnected;
	Engine *engine;
	
	Server(System &syst);
	void stop();
	void send(Mat &img,Telemetry *tl);
	void receiver(Engine *eng);
};
