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
    port = osal_serial_port_open(syst.arduino_port,115200, OSAL_SERIAL_MODE_8N1);
    if (!port)
    {
    	LOG("[E]: Can't open serial port");
    	connect = false;
    }
    else
    {
		//usleep(2000000); // Wait hardware initialization
		connect = true;
	}
}
