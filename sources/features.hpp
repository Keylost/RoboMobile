#pragma once
#include <limits.h> // to get INT_MAX, to protect against overflow
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <opencv2/opencv.hpp>
#include "string.h"
#include <stdlib.h>
#include "timer.hpp"

using namespace cv;
using namespace std;

class fps_counter
{
private:
    robotimer t;
    int counter;
    double sec;
    double fps;
public:
    void init();
    double get_fps();
};

struct simple_hist
{
	uint32_t red = 0;
	uint32_t blue = 0;
	uint32_t black = 0;
	uint32_t yellow =0;
	uint32_t white = 0;
};

void color_counter(Mat& roi, simple_hist &color);
