#include "arduino.hpp"


/*
 * Функция deinit() закрывает соединение с микроконтроллером.
 */
void Arduino_ctrl::deinit()
{
    if (port) 
    {
    	osal_serial_port_close(port);
    	connect = false;
    }
}

/*
 * Обеспечивает обратную связь с Arduino
 * В частности получает реальную скорость движения робота от arduino
 */
int Arduino_ctrl::feedback()
{
	char buffer[10];
	char current;
	int sz =  osal_serial_port_read(port,&current,1);
	if(sz>0 && current=='F')
	{
		int i=0;
		bool end_flag = false;
		for(; !end_flag;i++)
		{
			while(osal_serial_port_read(port,&current,1)!=1);
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
		printf("buffer %d\n",atoi(buffer));
		return atoi(buffer);
	}
	
	
	return -1;
}

/*
 * Функция send_command() отправляет параметры движения робота на микроконтроллер.
 * @engine - структура содержащая основные параметры движения робота
 */
void Arduino_ctrl::send_command(Engine* engine)
{
	if (port) 
	{		
		snprintf(message, sizeof(message), "SPD %d,%d,%d ",
				engine->angle,
				engine->direction, engine->speed);
		osal_serial_port_write(port, message, strlen(message));
	}
}

/*
 * Функция isconnected() возвращает состояние соединения с микроконтроллером.
 * Возвращаемые значения:
 * "истина", если соединение открыто, "ложь" в ином случае
 */
bool Arduino_ctrl::isconnected()
{
	return connect;
}

/*
 * Конструктор класса Arduino_ctrl.
 * Отвечает за корректное создание объекта класса и открытие соединения с микроконтроллером.
 */
Arduino_ctrl::Arduino_ctrl(System &syst)
{
	
	/*
	bool connect()
	int fd = 0; //дескриптор устройства
	fd = open(syst.arduino_port, O_RDWR | O_NOCTTY | O_NDELAY); // Открывает последовательный порт
	if(fd<0) //произошла ошибка при открытии порта
	{
		return false;
	}
	
	struct termios options; //структура содержащая настройки порта
	tcgetattr(fd, &options); //считать текущие настройки порта
	
	//установить скорость соединения (115200 бодов в секунду)
	cfsetispeed(&options, B115200); 
	cfsetospeed(&options, B115200);
	options.c_cflag &= ~PARENB; //выключить проверку четности
	options.c_cflag &= ~CSTOPB; //1 стопбит
	options.c_cflag &= ~CSIZE; //выключение битовой маски
	options.c_cflag |= CS8; //режим: 8 бит
	options.c_cflag |= ( CLOCAL | CREAD );
	fcntl(port->fd, F_SETFL, FNDELAY); //0
	
	tcsetattr(fd, TCSANOW, &options); //применить новые настройки порта
	tcflush(port->fd, TCIOFLUSH);
	
	return true;
	*/
	
	/*
	bool disconnect()
	close(fd); //закрыть соединение
	*/
	
	/*
	запись данных в порт
	bool send(const char *buffer, size_t size)
	int bytes_written = write(fd, buffer, size);
	ioctl(port->fd, TCSBRK, 1);
	*/
	
	/*
	чтение данных из порта
	bool read(char *buffer, size_t size)
	read(port->fd, buffer, size);
	*/
	
    port = osal_serial_port_open(syst.arduino_port,115200, OSAL_SERIAL_MODE_8N1);
    if (!port)
    {
    	LOG("[E]: Can't open serial port");
    	connect = false;
    }
    else
    {
		connect = true;
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
	Arduino_ctrl controller(syst);
	if(!controller.isconnected())
	{
		LOG("[W]: Arduino isn't attached. The robot willn't move.");
		return NULL;
	}
	
	while(1)
	{
		syst.engine_get(eng);
		controller.send_command(&eng);
		int spd=controller.feedback();
		if(spd!=-1) eng.real_speed = spd;
		syst.engine_set_realSpeed(eng.real_speed);
		usleep(10000); //10 ms. максимальная задержка реакции робоавтомобиля
	}
	controller.deinit();
	return NULL;
}
