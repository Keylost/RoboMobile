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
#include "config.hpp"

using namespace std;
using namespace cv;


class Recognition
{
private:
/*
 * Таймеры используемые для установки задержек при реакции на знаки и стоп линию
 */
time_t timer;
robotimer timer_line;

/*
 * Структуры signarea и linearea указывают на области на кадре
 * в которых будет происходить поиск знака и линии соответственно.
 * (настраиваются в конструкторе класса в зависимости от разрешения 
 * видео заданного в конфигурации)
 */
Rect linearea;
Rect signarea;

public:

/*
 * Основная структура,
 * содержащая поля необходимые для
 * контроля скорости, угла поворота и 
 * направления движения робота
 */
Engine engine;

/*
 * Структуры mysign и myline хранят информацию о найденных знаках и 
 * линии
 */
sign_data &mysign = engine.mysign;
line_data &myline = engine.myline;

Recognition(System &syst);
void handle_line();
void handle_sign(const cv::Mat& image);
void recognize_sign(const cv::Mat& image);
void recognize_line(const cv::Mat& image);
};

Point TplMatch( Mat &img, Mat &mytemplate, double &maxVal);
