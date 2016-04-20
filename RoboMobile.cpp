#include "CLP.hpp" //commandline parser
#include "config.hpp" //gets system configuration from conf file
#include "logger.hpp" //contains logging functions
#include <opencv2/opencv.hpp>
#include "server.hpp" //contains sending telemetry functions
#include "recognition.hpp" //signs and line recognition
#include "features.hpp" //contains features like color and fps counters
#include "arduino.hpp" //functions to work with arduino board 
#include "Engine.hpp" //main structure
#include "timer.hpp" //profiling functions
#include "videomaker.hpp"
#include "config.hpp"
#include <pthread.h>
#include "capture.hpp"
#include "signs.hpp"
#include "queue.hpp"
#include "userLoop.hpp"


/*
 * RoboMobile - это многопоточное приложение предназначеннное для управления робототехническим средством
 * на основании данных полученных с датчиков
 * Функция main() настраивает систему в зависимости от параметров, хранящихся в файле конфигурации и переданных
 * в командной стрроке и затем вызывает функцию, запускающую систему
 */
int main(int argc, char **argv)
{
	System syst;	
	syst.init(); //Загрузить конфигурацию из файла config.conf
	
	CLP::parse(argc,argv,syst); //get command line parameters
	
	/*Создает поток распознования линии на изображении*/
	pthread_t recognize_line_thr;
	pthread_create(&recognize_line_thr, NULL, recognize_line_fnc, &syst);
	
	/*Создает поток распознования знаков на изображении*/
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
	
	while(1)
	{
		curLineData = qline.waitForNewObject(curLineData);
		syst.signs_get(Signs);
		syst.engine_get(engine);
		
		//user controll function start		
		userLoop(*(curLineData->obj),Signs,engine);		
		//user controll function start
		
		syst.engine_set(engine);
		curLineData->free();
	}
	
	/*Не завершать программу. Ожидать завершения видео потока.*/
	pthread_join(capture_thr,NULL);
    
    return 0; //this make us happy:)
}
