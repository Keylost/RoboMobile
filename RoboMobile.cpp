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
#include "recognitionAdd.hpp" //объявляет функции распознавания доп. объектов, таких как другие авто и пешеходы

#include <pigpio.h>

#define SERVO	  26
#define ESC		  17
#define ENCODER_A 3
#define ENCODER_B 4

void* read_speed_fnc(void *ptr);
static volatile int encoderPos = 0; //текущая позиция энкодера
void encoderPulse(int gpio, int lev, uint32_t tick);

/*
 * RoboMobile - это многопоточное приложение, предназначеннное для управления робототехническим средством
 * на основании данных, полученных с датчиков.
 * Функция main() настраивает систему в зависимости от параметров, хранящихся в файле конфигурации и переданных
 * в командной стрроке, и запускает систему
 */
int main(int argc, char **argv)
{
   if (gpioInitialise() < 0)
   {
      fprintf(stderr, "pigpio initialisation failed\n");
      return 1;
   }
	
	/* Set GPIO modes */
	gpioSetMode(SERVO, PI_OUTPUT);
	gpioSetMode(ENCODER_A, PI_INPUT);
	//gpioSetMode(ENCODER_B, PI_INPUT);
	gpioSetMode(ESC, PI_OUTPUT);
	//gpioSetMode(18, PI_OUTPUT);
	//gpioSetMode(23, PI_INPUT);
	//gpioSetMode(24, PI_OUTPUT);
	//включить подтягивающие резисторы
	gpioSetPullUpDown(ENCODER_A, PI_PUD_UP);
	//gpioSetPullUpDown(ENCODER_B, PI_PUD_UP);
	//Назначить функцию для обработки прерываний	
	gpioSetAlertFunc(ENCODER_A, encoderPulse);
	//gpioSetAlertFunc(ENCODER_B, encoderPulse);	
	
	System syst;
	syst.init("../configs/config.conf"); //Загрузить конфигурацию из файла config.conf
	
	//initialize servo and esc
	gpioServo(ESC, 1500);
	gpioServo(SERVO, 1500);
	usleep(1000);
	//main_tst(argc, argv);
	
	CLP::parse(argc,argv,syst); //Получить параметры командной строки
	
	/*Создает поток распознавания линии на изображении*/
	pthread_t recognize_line_thr;
	pthread_create(&recognize_line_thr, NULL, recognize_line_fnc, &syst);
	
	pthread_t read_speed_thr;
	pthread_create(&read_speed_thr, NULL, read_speed_fnc, &syst);
	
	/*Создает поток распознавания пешеходов*/
	pthread_t recognize_ped_thr;
	pthread_create(&recognize_ped_thr, NULL, recognize_ped_fnc, &syst);
	
	/*Создает поток распознавания моделей автомобилей*/
	pthread_t recognize_auto_thr;
	pthread_create(&recognize_auto_thr, NULL, recognize_auto_fnc, &syst);	
	
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
	//pthread_t arduino_thr;
	//pthread_create(&arduino_thr, NULL, arduino_fnc, &syst);
	
	/*Создает поток захвата изображений с видеокамеры*/
	pthread_t capture_thr;
	pthread_create(&capture_thr, NULL, capture_fnc, &syst);
	
	
	//if (wiringPiSetup () == -1)
	{
		//fprintf (stdout, "oops: %s\n", strerror (errno)) ;
		//return 1 ;
	}
	//softServoSetup (8, -1, -1, -1, -1, -1, -1, -1) ;
	Engine engine; //Хранит парметры движения робота
	
	if(syst.headDevice == ORANGE_HEAD)
	{
		Object<line_data> *curLineData = NULL;
		Queue<line_data> &qline = syst.qline;
		
		/*
		 * Основной поток.
		 * Здесь происходит объдинение данных, полученных с датчиков,
		 * и расчет парметров движения робота
		 */
		while(1)
		{
			curLineData = qline.waitForNewObject(curLineData); //получить информацию о положениии линии
			
			userLoop(*(curLineData->obj), syst);	//вызвать функцию расчета параметров движения робота
			
			curLineData->free(); //освободить взятый из очереди объект
			
			syst.engine_get(engine);
			gpioServo(2, 750+engine.angle/0.12);

		}
	}
	gpioTerminate();
	/*Не завершать программу. Ожидать завершения потока захвата видео.*/
	pthread_join(capture_thr,NULL);
    
    return 0; //this make us happy:)
}

void encoderPulse(int gpio, int level, uint32_t tick)
{
	encoderPos++;
	return;
}

//Поток чтения текущей скорости с энкодера
void* read_speed_fnc(void *ptr)
{
	if(!ptr)
	{
		printf("[E]: Unitiolazed system!\n");
		exit(1);
	}
	System &syst = *((System *)ptr);
	
	uint32_t realSpd = 0;
	Engine engine; //Хранит парметры движения робота
	float error = 0;
	const float kp = 0.5;
	const float ki = 0.1;
	float integral = 0;
	int regulator = 0;
	int regulatorOld = 0;
	
	while(true)
	{
		realSpd = encoderPos*1.3/0.3;
		encoderPos = 0;
		syst.engine_set_realSpeed(realSpd);
		syst.engine_get(engine);	
		regulatorOld = regulator;
		error = engine.speed - realSpd;
		integral += error*0.3;
		if(integral>200) integral = 200;
		regulator = (int)(error*2.5 + integral);
		regulator = regulator>255 ? 255 : (regulator<0 ? 0 : regulator); 
		regulator = (regulator + regulatorOld)*0.5;
		if(engine.direction == 1)
		{
			gpioServo(ESC, 1500+regulator/0.34);
		}
		else
		{
			gpioServo(ESC, 1500-regulator/0.34);
		}
		
		usleep(300);
	}
}
