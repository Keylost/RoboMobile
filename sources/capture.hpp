#pragma once
#include "logger.hpp" //contains logging functions
#include <opencv2/opencv.hpp>
#include "timer.hpp" //profiling functions
#include "config.hpp"
#include "queue.hpp"

using namespace cv;
using namespace std;

void* capture_fnc(void *ptr);
