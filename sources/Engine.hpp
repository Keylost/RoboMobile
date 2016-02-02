#pragma once
#include "signs.hpp"

struct Engine
{
    int angle = 90;
    int angle_old =90;
    int direction = 1;
    int speed = 0;
    bool power = true;
    sign_data mysign;
	line_data myline;
};
