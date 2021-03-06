#pragma once
#include "config.hpp" //конфигурация системы
#include "logger.hpp" //функция для логгирования
#include "Engine.hpp" //объявляет структуру, хранящую параметры движения робота
#include "timer.hpp" //таймер
#include "signs.hpp" //объявляет структуры описывающие знаки и линию
#include <stdint.h>

using namespace std;

/*
 * Функция расчитывает параметры движения робота, основываясь на данных датчиков.
 * @myline - данные о положении линии
 * @syst - список знаков, находящихся в поле зрения робота
 */
void userLoop(line_data &myline, System &syst);
