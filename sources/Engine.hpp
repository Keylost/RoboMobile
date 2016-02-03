#pragma once
#include "signs.hpp"
#include "stdint.h"

struct Engine
{
    int32_t angle = 90;
    int32_t angle_old =90;
    int32_t direction = 1;
    int32_t speed = 0;
    bool power = true;
    sign_data mysign;
	line_data myline;
};
