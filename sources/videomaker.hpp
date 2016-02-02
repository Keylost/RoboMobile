#pragma once
#include <stdint.h>
#include <opencv2/opencv.hpp>
#include "config.hpp"

using namespace cv;
using namespace std;

class VideoMaker
{
private:
	double fps;
	Size size;	
	VideoWriter output;
	bool iscolor;
public:	
	bool init(System &syst);
	void write(Mat& frame);
	void deinit();
};
