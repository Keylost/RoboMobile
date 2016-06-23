#pragma once
#include <stdlib.h>
#include <vector>
#include "features.hpp"
#include <opencv2/opencv.hpp>
#include "Engine.hpp"
#include "logger.hpp"
#include <unistd.h>
#include "signs.hpp"
#include "timer.hpp"
#include "config.hpp"
#include <stdint.h>

using namespace std;
using namespace cv;

void* recognize_ped_fnc(void *ptr);

void recognize_ped(Mat &frame, bool &ped_state);
