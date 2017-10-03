#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <pthread.h>
#include "queue.hpp"
#include "Engine.hpp"
#include "signs.hpp"

using namespace cv;

/*
 * Определяет типы данных, которые умеет различать парсер
 * конфигурационных файлов.
 */
enum VALUE_TYPE
{
	INT_VAL = 0, //целое
	STRING_VAL =1 //строковое
};

/*
 * Определяет доступные режимы работы системы захвата видео.
 */
enum modes
{
	CAMERA = 0, //получать видео с камеры
	VIDEO  = 2  //получать видео из файла
};

/*
 * Определяет доступные режимы взаимодействия с Arduino.
 */
enum heads
{
	ARDUINO_HEAD=0, //передавать коды распознанных системой объектов на Arduino (данные о направлении и скорости движения не передаются)
	ORANGE_HEAD=1 //передавать на Arduino только данные о направлении и скорости движения 
};

/*
 * Класс System.
 * Класс, содержащий основные статические и динамические параметры системы. 
 */
class System
{
	private:
	
	/*
	 * Cодержит основные параметры движения робота, такие как скорость и
	 * направление.
	 */
	Engine engine;
	
	/*
	 * Вектор содержащий коды знаков, находящихся в поле зрения робота.
	 */
	vector<sign_data> Signs;
	
	pthread_mutex_t engine_mutex; //мутекс для контроля доступа к engine
	pthread_mutex_t signs_mutex; //мутекс для контроля доступа к Signs
	pthread_mutex_t barrier_mutex; //мутекс для контроля доступа к barrier
	pthread_mutex_t pedestrian_mutex; //мутекс для контроля доступа к pedestrian
	pthread_mutex_t autoModel_mutex; //мутекс для контроля доступа к autoModel
	
	/*
	 * Флаг, показывающий есть или нет препятствие на пути робота.
	 */
	bool barrier;
	
	/*
	 * Флаг, показывающий есть или нет модель пешехода на пути робота.
	 */	
	bool pedestrian;
	
	/*
	 * Флаг, показывающий есть или нет другая модель автомобиля на пути робота.
	 */		
	bool autoModel;
	
	public:
	
	Queue<Mat> queue; //очередь для кадров с видеокамеры
	Queue<line_data> qline; //очередь для данных линии
	
	/*
	 * Включить/выключить удаленное управление роботом
	 */
	bool remoteControl;
	
	char arduino_port[30];
	modes MODE;
	
	heads headDevice;
	
	bool videomaker;
	char videoname[80];
	
	Rect linearea;
	Rect signarea;
	
	int capture_width;
	int capture_height;
	int image_quality;
	uint32_t robot_center; //Точка на кадре, на которую робот будет выравниваться.
	
	////////////////////////////////////////////////////////////
	
	/*
	 * Конструктор класса.
	 * Устанавливает настройки по умолчанию.
	 */
	System()
	{
		engine_mutex = PTHREAD_MUTEX_INITIALIZER;
		signs_mutex = PTHREAD_MUTEX_INITIALIZER;
		barrier_mutex = PTHREAD_MUTEX_INITIALIZER;
		pedestrian_mutex = PTHREAD_MUTEX_INITIALIZER;
		autoModel_mutex = PTHREAD_MUTEX_INITIALIZER;
		
		MODE = CAMERA;
		capture_width = 640;
		capture_height = 360;
		snprintf(arduino_port,30,"/dev/ttyACM0");
		headDevice = ORANGE_HEAD;
		image_quality = 80;
		videomaker = false;
		robot_center = 320;
		signarea = Rect( Point( capture_width/2, 0 ), Point( capture_width-1, capture_height/2 ) );
		linearea = Rect( Point( 0, capture_height - capture_height/4 ), Point( capture_width-1, capture_height-1) );
		
		barrier = false;
		pedestrian = false;
		autoModel = false;
		
		remoteControl = false; //выключить удаленнное управление по умолчанию.
	}
	
	/*
	* Функция init() отвечает за инициализацию System с использованием данных из конфигурационного файла.
	*/	
	void init(const char *configFileName);
	
	void engine_get(Engine &destination);
	void engine_set_realSpeed(uint32_t realSpeed);
	void engine_set(Engine &source);
	void signs_get(vector<sign_data> &destination);
	void signs_set(vector<sign_data> &source);
	
	bool barrier_get();
	void barrier_set(bool statusFlag);
	bool pedestrian_get();
	void pedestrian_set(bool statusFlag);
	bool autoModel_get();
	void autoModel_set(bool statusFlag);		
};

struct config
{
	char name[180];
	VALUE_TYPE type;
	void *value;
};
