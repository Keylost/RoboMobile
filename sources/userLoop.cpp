#include "userLoop.hpp"

#define ANGLE_CENTER  90 //угол сервомотора, при котором колеса робота смотрят прямо
#define ANGLE_RANGE  35 //максимальное отклонение сервомотора от центрального положения
#define MAX_SPEED  990 //максимальная скорость движения в условных единицах (от 0 до 999)
#define MIN_SPEED  450 //минимальная скорость движения в условных единицах (от 0 до 999)
#define ANGLE_MIN  (ANGLE_CENTER - ANGLE_RANGE)
#define ANGLE_MAX  (ANGLE_CENTER + ANGLE_RANGE)
uint32_t speed_crosswalk = MIN_SPEED; //скорость при обнаружении пешеходного перехода
uint32_t speed_stop      = 0; //скорость при обнаружении знака стоп
uint32_t speed_trafficlight  = 0; //скорость при обнаружении желтого или красного сигнала светофора


/*
 * Функция loop() занимается обработкой данных о черной линии и разметке, хранящихся в поле
 * myline объекта класса Recognition
 * 
 * Функция задает параметры движения робота от данных хранящихся в myline.
 */

double delta=0,old_delta=0;
robotimer timer_line;

void userLoop(line_data &myline, vector<sign_data> &Signs, Engine &engine)
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
				//else if(timer_line.get()>700 && timer_line.get()<9000) sl_passed = true;
				else if(timer_line.get()>=9000)
				{
					timer_line.zero();
					myline.stop_line = false;
				}
			}
		}

		//calculate angle begin
		/* Вычислить отклонение робота от центра линии*/
		delta = myline.center_of_line-myline.robot_center;
		
		if(abs(delta-old_delta)>80) delta = old_delta*0.7 + delta*0.3;
		
		/* Вычислить угол поворота робота согласно его отклонению и заданным коэффициентам ПИД регулятора */
		//engine->angle = 90 - PID(delta,(double)engine.angle); //PID regulator
		engine.angle = ANGLE_CENTER - delta*1/6.7 - (delta- old_delta)*0.2; //простой PD регулятор; 6.7 - пропорциональная компонента, 0.2 - дифференциальная 
		old_delta = delta;
		
		/* Проверить вычисленное значение угла на выход за границы диапазона доступных углов сервопривода*/
		if(engine.angle<ANGLE_MIN) engine.angle =ANGLE_MIN;
		else if(engine.angle>ANGLE_MAX) engine.angle =ANGLE_MAX;
		//calculate angle end
		
		//calculate speed begin
		int ee = abs((int32_t)engine.angle - ANGLE_CENTER);
		engine.speed = MAX_SPEED - ee*((MAX_SPEED-MIN_SPEED)/ANGLE_RANGE); //
		//calculate speed end
	}
	else
	{
		engine.speed = 0;
		LOG("[W]: black line not found");
	}
}
