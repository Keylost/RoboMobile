#include "arduino.hpp"

bool ArduinoCtrl::connect(const char* arduinoPort)
{	
	arduino_fd = open(arduinoPort, O_RDWR | O_NOCTTY | O_NDELAY); // Открывает последовательный порт
	if(arduino_fd<0) //произошла ошибка при открытии порта
	{
		return false;
	}
	
	struct termios options; //структура содержащая настройки порта
	tcgetattr(arduino_fd, &options); //считать текущие настройки порта
	
	//установить скорость соединения (115200 бодов в секунду)
	cfsetispeed(&options, B115200); 
	cfsetospeed(&options, B115200);
	options.c_cflag &= ~PARENB; //выключить проверку четности
	options.c_cflag &= ~CSTOPB; //1 стопбит
	options.c_cflag &= ~CSIZE; //выключение битовой маски
	options.c_cflag |= CS8; //режим: 8 бит
	options.c_cflag |= ( CLOCAL | CREAD );
	//fcntl(arduino_fd, F_SETFL, FNDELAY); //0
	
	tcsetattr(arduino_fd, TCSANOW, &options); //применить новые настройки порта
	tcflush(arduino_fd, TCIOFLUSH);
	
	return true;
}

bool ArduinoCtrl::disconnect()
{
	if(close(arduino_fd)==0) //закрыть соединение
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * Функция deinit() закрывает соединение с микроконтроллером.
 */
void ArduinoCtrl::deinit()
{
	if(connectionStatus)
	{
		if(disconnect())
		{
			connectionStatus = false;
		}
	}
	
	return;
}

/*
 * Обеспечивает обратную связь с Arduino
 * В частности получает реальную скорость движения робота от arduino
 */
int ArduinoCtrl::feedback()
{
	char buffer[10];
	char current;	
	int sz =  read(arduino_fd, &current,1);
	if(sz>0 && current=='F')
	{
		int i=0;
		bool end_flag = false;
		for(; !end_flag;i++)
		{
			while(read(arduino_fd, &current,1)!=1);
			if(current!='E')
			{
				buffer[i] = current;
			}
			else
			{
				end_flag = true;
			}
		}
		buffer[i+1] = '\0';
		//printf("buffer %f %d\n",atof(buffer),atoi(buffer));
		return atoi(buffer);
	}
	
	return -1;
}

/*
 * Функция send_command() отправляет параметры движения робота на микроконтроллер.
 * @engine - структура содержащая основные параметры движения робота
 */
void ArduinoCtrl::sendCommand(Engine* engine)
{
	snprintf(message, sizeof(message), "SPD %d,%d,%d ", engine->angle, engine->direction, engine->speed);
	int sz = strlen(message);
	int bytes_written = write(arduino_fd, message, sz);
	if(bytes_written<sz)
	{
		//LOG("[E]: Arduino: Sending data error");
	}
	ioctl(arduino_fd, TCSBRK, 1);
	return;
}

/*
 * Функция isconnected() возвращает состояние соединения с микроконтроллером.
 * Возвращаемые значения:
 * "истина", если соединение открыто, "ложь" в ином случае
 */
bool ArduinoCtrl::isConnected()
{
	return connectionStatus;
}

/*
 * Конструктор класса ArduinoCtrl.
 * Отвечает за корректное создание объекта класса и открытие соединения с микроконтроллером.
 */
ArduinoCtrl::ArduinoCtrl(System &syst)
{
	arduino_fd = -1;
	
	if(connect(syst.arduino_port))
	{
		connectionStatus = true;
	}
	else
	{
		LOG("[E]: Can't open serial port");
		connectionStatus = false;
	}
}

/*
 * Функция arduino_fnc() реализует поток взаимодействия с микроконтроллером.
 * @ptr - указатель на структуру System.
 * Устанавливает соединение с микроконтроллером и начинает процесс передачи параметров движения.
 */
void* arduino_fnc(void *ptr)
{
	System &syst = *((System *)ptr);
	Engine eng; //локальная копия Engine
	ArduinoCtrl controller(syst);
	if(!controller.isConnected())
	{
		LOG("[W]: Arduino isn't attached.");
		return NULL;
	}
	
	while(1)
	{
		syst.engine_get(eng);
		controller.sendCommand(&eng);
		int spd=controller.feedback();
		if(spd!=-1) eng.real_speed = spd;
		syst.engine_set_realSpeed(eng.real_speed);
		usleep(10000); //10 ms. максимальная задержка реакции робоавтомобиля
	}
	controller.deinit();
	return NULL;
}
