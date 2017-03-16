#pragma once
#include <opencv2/opencv.hpp>

enum signs
{
	sign_none          = 0,
	sign_stop          = 1,	
	sign_crosswalk 	   = 2,
	sign_mainroad      = 3,
	sign_giveway       = 4,
	sign_starttrafficlight_green 	= 5,
	sign_starttrafficlight_red 		= 6,
	sign_trafficlight_green  		= 7,
	sign_trafficlight_yellow 		= 8,
	sign_trafficlight_red 			= 9,
	sign_trafficlight_off			= 10,
	sign_trafficlight_yelRed  		= 11,
	sign_starttrafficlight_off 		= 12
};

enum trafficlight_states
{
	redlight    = 0,
	yellowlight = 1,
	greenlight  = 2
};

struct sign_data
{
	cv::Rect area;
	signs sign;
	uint32_t detect_time; //время, прошедшее с поледней регистрации знака системой
};

struct line_data
{
	int32_t robot_center = 323; //точка на изображении, на которую будет выравниваться робот
	int32_t center_of_line = 323; //точка на изображении, в которой находится центр направляющей линии
	bool on_line = true;
	bool stop_line = false;
};
