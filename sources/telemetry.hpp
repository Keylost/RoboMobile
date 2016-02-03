#pragma once
#include <stdint.h>
#include "signs.hpp"

struct Telemetry
{
	uint32_t angle =0;
	uint32_t direction=0;
	uint32_t speed=0;
	sign_data mysign; //sign 
	line_data myline;
};
