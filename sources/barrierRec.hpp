/*
 * Описывает функции обнаружения препятствий
 */
#pragma once
#include <stdint.h>
#include <opencv2/opencv.hpp>
#include "config.hpp"
#include "logger.hpp"
#include "queue.hpp"

using namespace cv;
using namespace std;

//Mat frame, framePrev;

bool checkBarrier(Mat &cur, Mat &prev);
void* barrier_fnc(void *ptr);
