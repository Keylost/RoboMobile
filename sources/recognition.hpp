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

using namespace std;
using namespace cv;


class Recognition
{
private:
time_t timer; //using for waiting
robotimer timer_line; //using for waiting
Engine *engine; //controll structure
public:
sign_data mysign; //sign 
line_data myline; //line
Recognition(Engine* eng);
void handle_line();
void handle_sign(const cv::Mat& image);
void recognize_sign(const cv::Mat& image);
void recognize_line(const cv::Mat& image);
};

Point TplMatch( Mat &img, Mat &mytemplate, double &maxVal);
