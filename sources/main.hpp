#pragma once
#include "logger.hpp" //contains logging functions
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "server.hpp" //contains sending telemetry functions
#include "recognition.hpp" //signs and line recognition
#include "features.hpp" //contains features like color and fps counters
#include "arduino.hpp" //functions to work with arduino board 
#include "Engine.hpp" //main structure
#include "timer.hpp" //profiling functions
#include "telemetry.hpp"
#include <iostream>
#include <stdio.h>
#include "videomaker.hpp"
#include "config.hpp"
#include <pthread.h>
//#include "capture.hpp" //frames capture functions

using namespace cv;
using namespace std;

void main_thr(System &syst);

void* capture_fnc(void *ptr);
void* recognize_fnc(void *ptr);
void* server_fnc(void *ptr);
void* arduino_fnc(void *ptr);
void* videomaker_fnc(void *ptr);

void capture_from_cam_fnc(System &syst);
void capture_from_img_fnc(System &syst);
void capture_from_vid_fnc(System &syst);
