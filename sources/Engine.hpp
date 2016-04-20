#pragma once
#include "signs.hpp"
#include "stdint.h"

struct Engine
{
    uint32_t angle = 90;
    uint32_t direction = 1;
    uint32_t speed = 0;
    uint32_t real_speed =0; //Сорость в реальных единицах полученная с arduino
};
