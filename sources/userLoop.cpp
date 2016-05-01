/*
 * В данном файле находятся функции,
 * устанавливающие параметры движения робота
 * в зависимости от данных анализа изображения
 */
#include "userLoop.hpp"

#define ANGLE_CENTER  			90 //угол сервомотора, при котором колеса робота смотрят прямо
#define ANGLE_RANGE  			35 //максимальное отклонение сервомотора от центрального положения
#define MAX_SPEED  				700 //максимальная скорость движения в условных единицах (от 0 до 999)
#define MIN_SPEED  				450 //минимальная скорость движения в условных единицах (от 0 до 999)
#define ANGLE_MIN  				(ANGLE_CENTER - ANGLE_RANGE)
#define ANGLE_MAX  				(ANGLE_CENTER + ANGLE_RANGE)
#define speed_crosswalk 		MIN_SPEED //скорость при обнаружении пешеходного перехода
#define speed_stop      		0 //скорость при обнаружении знака стоп
#define speed_trafficlight		0 //скорость при обнаружении желтого или красного сигнала светофора

double delta=0,old_delta=0;
robotimer timer_line;
signs signPrev = sign_none;
double signWeightDefault = 0.1;
double signWeight = 0.1; //текущее влияние знака на скорость

robotimer holder;
int32_t holdFor = 0;
bool hold = false;
int32_t holdSpeed =0;
void startHolding(int32_t holdForMs, int32_t speed)
{
	holder.start();
	holdFor = holdForMs;
	holdSpeed = speed;
	hold = true;
}

void calcAngleAndSpeed(line_data &myline, Engine &engine);
signs getMaxPrioritySign(vector<sign_data> &Signs);

signs in_handle = sign_none;
bool in_signs(signs sign,vector<sign_data> &Signs)
{
	for(unsigned i=0;i<Signs.size();i++)
	{
		if(sign==Signs[i].sign) return true;
	}
	return false;
}
int get_signNum(signs sign,vector<sign_data> &Signs)
{
	for(unsigned i=0;i<Signs.size();i++)
	{
		if(sign==Signs[i].sign) return i;
	}
	return -1;
}
/*
 * Функция userLoop() занимается обработкой данных о линии, разметке и знаках
 * Функция  должна задавать параметры движения робота в engine.
 * Никогда не используйте sleep или usleep в этой функции
 */
void userLoop(line_data &myline, vector<sign_data> &Signs, Engine &engine)
{
	/*
	 * Функция вычисляет скорость движения и угол поворота на основании данных
	 * о линии, и записывает эти параметры в engine
	 */
	
	calcAngleAndSpeed(myline,engine);
	
	if(hold)
	{
		holder.stop();
		if(holder.get()>=holdFor)
		{
			hold = false;
			in_handle = sign_none;
		}
		else
		{
			engine.speed = holdSpeed;
		}
	}
	
	if(!hold)
	{
		if(in_handle==sign_none)
		{
			signs current = getMaxPrioritySign(Signs);
			if(current!=sign_none)
			{
				if(current == sign_trafficlight || current == sign_starttrafficlight)
				{
					if(myline.stop_line)
					{
						in_handle = current;
					}
				}
				else
				{
					in_handle = current;
				}
			}			
		}
		switch(in_handle)
		{
			case sign_none:
			{
				break;
			};
			case sign_stop:
			{
				if(in_signs(sign_stop,Signs))
				{
					engine.speed = engine.speed - (engine.speed - MIN_SPEED)*signWeight;				
					double tmpw = signWeight+0.03;
					if(tmpw<1.0)
					{
						signWeight = tmpw;
					}
				}
				else
				{
					signWeight = signWeightDefault;
					engine.speed = speed_stop;
					startHolding(4000,speed_stop); //сохранять указанную скорость движения 2000 миллисекунд
				}
				break;
			};
			case sign_starttrafficlight:
			{
				int n = get_signNum(sign_starttrafficlight,Signs);
				if(n==-1 || Signs[n].state != greenlight)
				{
					engine.speed = 0;
				}
				else
				{
					in_handle = sign_none;
				}
				
				break;
			};
			case sign_trafficlight:
			{
				int n = get_signNum(sign_trafficlight,Signs);
				if(n==-1 || Signs[n].state != greenlight)
				{
					engine.speed = 0;
				}
				else
				{
					in_handle = sign_none;
				}
				
				break;
			};
			default:
			{
				break;
			};
		}
	}
	
	/*
	calcAngleAndSpeed(myline,engine);
	if(hold)
	{
		holder.stop();
		if(holder.get()>=holdFor)
		{
			hold = false;
		}
		else
		{
			engine.speed = holdSpeed;
		}
	}
	
	if(!hold)
	{
		signs current = getMaxPrioritySign(Signs);
		
		switch(current)
		{
			case sign_stop:
			{
				engine.speed = engine.speed - (engine.speed - MIN_SPEED)*signWeight;				
				double tmpw = signWeight+0.03;
				if(tmpw<1.0)
				{
					signWeight = tmpw;
				}
				break;
			};
			case sign_crosswalk:
			{
				engine.speed = engine.speed - (engine.speed - speed_crosswalk)*signWeight;
				double tmpw = signWeight+0.03;
				if(tmpw<1.0)
				{
					signWeight = tmpw;
				}
				break;
			};
			case sign_trafficlight:
			{
				
				break;
			};
			default: break;
		}
	
		if(signPrev!=sign_none && current!=signPrev)
		{
			switch(signPrev)
			{
				case sign_stop:
				{
					engine.speed = speed_stop;
					signWeight = signWeightDefault;
					startHolding(4000,speed_stop); //сохранять указанную скорость движения 2000 миллисекунд
					break;
				}
				case sign_crosswalk:
				{
					engine.speed = speed_crosswalk;
					signWeight = signWeightDefault;
					startHolding(3000,speed_crosswalk);
					break;
				}
				default:
				{
					break;
				}		
			}
		}
		signPrev = current;
	}
	*/
}




void calcAngleAndSpeed(line_data &myline, Engine &engine)
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
		engine.speed = MAX_SPEED - ee*((MAX_SPEED-MIN_SPEED)/ANGLE_RANGE);
		//calculate speed end
	}
	else
	{
		engine.speed = 0;
		LOG("[W]: black line not found");
	}
}

bool field[7];

signs getMaxPrioritySign(vector<sign_data> &Signs)
{
	if(Signs.size()==0) return sign_none;
	
	memset(field,0,7);
	
	for(unsigned i=0;i<Signs.size();i++)
	{
		if(Signs[i].sign == sign_starttrafficlight || Signs[i].sign == sign_trafficlight)
		{
			if(Signs[i].state == redlight || Signs[i].state == yellowlight)
			{
				field[Signs[i].sign] = true;
			}
		}
		field[Signs[i].sign] = true;
	}
	
	if(field[sign_stop]) return sign_stop;
	if(field[sign_trafficlight]) return sign_trafficlight;
	if(field[sign_starttrafficlight]) return sign_starttrafficlight;
	if(field[sign_crosswalk]) return sign_crosswalk;
	if(field[sign_giveway]) return sign_giveway;
	if(field[sign_mainroad]) return sign_mainroad;
	
	return sign_none;
}
