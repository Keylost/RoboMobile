#pragma once
#include "config.hpp" //gets system configuration from conf file
#include "logger.hpp" //contains logging functions
#include "Engine.hpp" //main structure
#include "timer.hpp" //profiling functions
#include "signs.hpp"
#include <stdint.h>

using namespace std;

void userLoop(line_data &myline, vector<sign_data> &Signs, Engine &engine);
