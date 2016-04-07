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


void loop();

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
	//pthread_create(&recognize_sign_thr, NULL, recognize_sign_fnc, &syst);
	
	/*Создает поток взаимодействия с клиентским приложением*/
	pthread_t server_thr;
	pthread_create(&server_thr, NULL, server_fnc, &syst);
	
	/*Создает поток записи видео с камеры в видеофайл*/
	pthread_t videomaker_thr;
	pthread_create(&videomaker_thr, NULL, videomaker_fnc, &syst);
	
	/*Создает поток взаимодействия с микроконтроллером*/
	pthread_t arduino_thr;
	//pthread_create(&arduino_thr, NULL, arduino_fnc, &syst);
	
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
		//engine.myline.center_of_line = curLineData->obj->center_of_line;
		/*
		if(!qsign.empty())
		{
			qsign.pop();
		}
		*/
		//user controll function start		
		//loop();		
		//user controll function start
		
		curLineData->free();
	}
	
	/*Не завершать программу. Ожидать завершения видео потока.*/
	pthread_join(capture_thr,NULL);
    
    return 0; //this make us happy:)
}


/*
 * Функция handle_line() занимается обработкой данных о черной линии и разметке, хранящихся в поле
 * myline объекта класса Recognition
 * 
 * Функция задает параметры движения робота от данных хранящихся в myline.
 */
/*
double delta=0,old_delta=0;
void loop()
{
	if(myline.on_line)
	{
		if(myline.stop_line)
		{
			timer_line.stop();
			if(timer_line.get()<=0)
			{
				timer_line.start();
				engine.speed=0;
				return;
			}
			else
			{
				if(timer_line.get()<=700)
				{
					engine.speed=0;
					return;
				}
				else if(timer_line.get()>=9000)
				{
					timer_line.zero();
					myline.stop_line = false;
				}
			}
		}
*/
		//calculate angle begin
		//* Вычислить отклонение робота от центра линии*/
/*
		delta = myline.center_of_line-myline.robot_center;
		
		if(abs(delta-old_delta)>80) delta = old_delta*0.7 + delta*0.3;
*/		
		/* Вычислить угол поворота робота согласно его отклонению и заданным коэффициентам ПИД регулятора */
		//engine->angle = 90 - PID(delta,(double)engine.angle); //PID regulator
/*
		engine.angle = ANGLE_CENTER - delta*1/6.7 - (delta- old_delta)*0.2; //простой PD регулятор; 6.7 - пропорциональная компонента, 0.2 - дифференциальная 
		old_delta = delta;
*/		
		/* Проверить вычисленное значение угла на выход за границы диапазона доступных углов сервопривода*/
/*
		if(engine.angle<ANGLE_MIN) engine.angle =ANGLE_MIN;
		else if(engine.angle>ANGLE_MAX) engine.angle =ANGLE_MAX;
		//calculate angle end
		
		//calculate speed begin								
		if(mysign.sign==sign_none)
		{
			int ee = abs(engine.angle - ANGLE_CENTER);
			engine.speed = MAX_SPEED - ee*((MAX_SPEED-MIN_SPEED)/ANGLE_RANGE); //
		}
		//calculate speed end
	}
	else
	{
		engine.speed = 0;
		LOG("[W]: black line not found");
	}
}
*/
