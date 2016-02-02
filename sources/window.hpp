#pragma once
#include <opencv2/opencv.hpp>
#include "logger.hpp"

using namespace cv;

class Window
{
	//const char wname[];
	public:
	Window();
	~Window();
	void showimg(const Mat& image);
};
