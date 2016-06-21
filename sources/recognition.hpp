#pragma once
#include <stdlib.h>
#include <vector>
#include "features.hpp"
#include <opencv2/opencv.hpp>
#include "Engine.hpp"
#include "logger.hpp"
#include <unistd.h>
#include "signs.hpp"
#include "timer.hpp"
#include "config.hpp"
#include <stdint.h>

using namespace std;
using namespace cv;


void* recognize_sign_fnc(void *ptr);
void* recognize_line_fnc(void *ptr);

/*
 * Функция recognize_sign() занимается распознаванием знаков на изображении.
 * Все найденные знаки добавляются в @Signs.
 * 
 * Входные данные:
 * @frame - ссылка на текущее обрабатываемое цветное(BGR) изображение
 * 
 */
void recognize_sign(const Mat& orig, vector<sign_data> &Signs);

/*
 * Функция recognize_line() занимается распознаванием черной линии и разметки на изображении.
 * Результат записывается в @myline
 * Входные данные:
 * @orig - указатель на текущее обрабатываемое цветное(BGR) изображение;
 * @scan_row - номер строки матрицы изображения, по которой будет вестись поиск линии.
 */
void recognize_line(const Mat& orig, line_data &myline, int scan_row);

/*
 * Функция get_light() определяет активный сигнал светофора
 * Входные данные:
 * Mat& roi - указатель на изображение светофора
 * Возвращаемые значения: см. перечисление trafficlight_states в signs.hpp
 */
uint8_t get_light(Mat& roi);
