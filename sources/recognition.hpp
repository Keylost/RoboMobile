#pragma once
#include <stdlib.h>
#include <vector>
#include "features.hpp" //contains features like color and fps counters
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

void* recognize_sign_fnc(void *ptr);
void* recognize_line_fnc(void *ptr);
void recognize_sign(const Mat& orig);
void recognize_line(const Mat& orig);
