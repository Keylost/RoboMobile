#include "CLP.hpp" //Парсер командной строки
#include "logger.hpp" //функция для логгирования
#include <opencv2/opencv.hpp> // OpenCV
#include "server.hpp" //объявляет функции потока взаимодействия с клиентским приложением
#include "recognition.hpp" //объявляет функции потока распознавания знаков и разметки
#include "arduino.hpp" //объявляет функции потока взаимодействия с микроконтроллером
#include "Engine.hpp" //объявляет структуру, хранящую параметры движения робота
#include "videomaker.hpp" //объявляет функции потока записи видео в файл
#include "config.hpp" //конфигурация системы
#include <pthread.h> //объявляет функции создания потоков и управления ими
#include "capture.hpp" //объявляет функции потока захвата изображений с видеокамеры
#include "signs.hpp" //объявляет структуры описывающие знаки и линию
#include "queue.hpp" //объявляет функции взаимодействия с очередями 
#include "userLoop.hpp" //объявляет функции расчета параметров движения робота
#include "barrierRec.hpp" //объявляет функции распознавания препятствий


/*
 * RoboMobile - это многопоточное приложение, предназначеннное для управления робототехническим средством
 * на основании данных, полученных с датчиков.
 * Функция main() настраивает систему в зависимости от параметров, хранящихся в файле конфигурации и переданных
 * в командной стрроке, и запускает систему
 */
int main(int argc, char **argv)
{
	System syst;	
	syst.init(); //Загрузить конфигурацию из файла config.conf
	
	CLP::parse(argc,argv,syst); //Получить параметры командной строки
	
	/*Создает поток распознавания линии на изображении*/
	pthread_t recognize_line_thr;
	pthread_create(&recognize_line_thr, NULL, recognize_line_fnc, &syst);
	
	/*Создает поток распознавания препятствий*/
	//pthread_t barrier_thr;
	//pthread_create(&barrier_thr, NULL, barrier_fnc, &syst);
	
	/*Создает поток распознавания знаков на изображении*/
	pthread_t recognize_sign_thr;
	pthread_create(&recognize_sign_thr, NULL, recognize_sign_fnc, &syst);
	
	/*Создает поток взаимодействия с клиентским приложением*/
	pthread_t server_thr;
	pthread_create(&server_thr, NULL, server_fnc, &syst);
	
	/*Создает поток записи видео с камеры в видеофайл*/
	pthread_t videomaker_thr;
	pthread_create(&videomaker_thr, NULL, videomaker_fnc, &syst);
	
	/*Создает поток взаимодействия с микроконтроллером*/
	pthread_t arduino_thr;
	pthread_create(&arduino_thr, NULL, arduino_fnc, &syst);
	
	/*Создает поток захвата изображений с видеокамеры*/
	pthread_t capture_thr;
	pthread_create(&capture_thr, NULL, capture_fnc, &syst);
	
	Object<line_data> *curLineData = NULL;
	Queue<line_data> &qline = syst.qline;
	Engine engine;
	vector<sign_data> Signs;
	
	/*
	 * Основной поток.
	 * Здесь происходит объдинение данных, полученных с датчиков,
	 * и расчет парметров движения робота
	 */
	while(1)
	{
		curLineData = qline.waitForNewObject(curLineData); //получить информацию о положениии линии
		syst.signs_get(Signs); //получить информацию о знаках дорожного движения, находящихся в поле зрения робота
		syst.engine_get(engine); //получить текущие параметры движения робота
		
		userLoop(*(curLineData->obj),Signs,engine);	//вызвать функцию расчета параметров движения робота
		
		syst.engine_set(engine); //задать новые параметры движения робота
		curLineData->free(); //освободить взятый из очереди объект
	}
	
	/*Не завершать программу. Ожидать завершения потока захвата видео.*/
	pthread_join(capture_thr,NULL);
    
    return 0; //this make us happy:)
}
