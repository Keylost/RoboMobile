#pragma once
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <opencv2/opencv.hpp>
#include "string.h"
#include <stdlib.h>
#include "timer.hpp"

using namespace cv;
using namespace std;

struct simple_hist
{
	uint32_t red = 0;
	uint32_t blue = 0;
	uint32_t black = 0;
	uint32_t yellow =0;
	uint32_t white = 0;
};

void color_counter(Mat& roi, simple_hist &color);
