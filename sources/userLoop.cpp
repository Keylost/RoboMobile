/*
 * В данном файле находятся функции,
 * устанавливающие параметры движения робота
 * в зависимости от данных анализа изображения
 */
#include "userLoop.hpp"

#define ANGLE_CENTER  			90 //угол сервомотора, при котором колеса робота смотрят прямо
#define ANGLE_RANGE  			35 //максимальное отклонение сервомотора от центрального положения
#define MAX_SPEED  				650 //максимальная скорость движения в условных единицах (от 0 до 999)
#define MIN_SPEED  				450 //минимальная скорость движения в условных единицах (от 0 до 999)
#define ANGLE_MIN  				(ANGLE_CENTER - ANGLE_RANGE)
#define ANGLE_MAX  				(ANGLE_CENTER + ANGLE_RANGE)
#define speed_crosswalk 		500 //скорость при обнаружении пешеходного перехода
#define speed_stop      		0 //скорость при обнаружении знака стоп
#define speed_trafficlight		0 //скорость при обнаружении желтого или красного сигнала светофора

double delta=0,old_delta=0;
robotimer timer_line, last_line_timer;
signs signPrev = sign_none;
double signWeightDefault = 0.1;
double signWeight = 0.1; //текущее влияние знака на скорость

bool stoplineInHandle = false;

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

void calcAngleAndSpeed(line_data &myline, Engine &engine, vector<sign_data> &Signs);
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
 * Избегайте использования sleep или usleep в этой функции
 */

Engine engine;
vector<sign_data> Signs;
bool isPedestrian = false;
bool isAutoModel = false;

void userLoop(line_data &myline, System &syst)
{
	syst.signs_get(Signs); //получить информацию о знаках дорожного движения, находящихся в поле зрения робота
	syst.engine_get(engine); //получить текущие параметры движения робота
	isPedestrian = syst.pedestrian_get(); //получить данные о наличии пешеходов в кадре
	isAutoModel = syst.autoModel_get(); //получить данные о наличии пешеходов в кадре
	
	/*
	 * Функция вычисляет скорость движения и угол поворота на основании данных
	 * о линии, и записывает эти параметры в engine
	 */
	calcAngleAndSpeed(myline,engine, Signs);
	
	//test area
	if(isPedestrian || isAutoModel)
	{
		printf("barrier\n");
		engine.speed = 0;
		startHolding(500,0);
	}
	//test area
	
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
				if(current == sign_trafficlight_red || current == sign_trafficlight_yellow)
				{
					
					if(stoplineInHandle || myline.stop_line)
					{
						in_handle = sign_trafficlight_red;
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
					startHolding(4000,speed_stop); //сохранять указанную скорость движения 4000 миллисекунд
				}
				break;
			};
			case sign_crosswalk:
			{
				if(in_signs(sign_crosswalk,Signs))
				{
					engine.speed = engine.speed - (engine.speed - speed_crosswalk)*signWeight;				
					double tmpw = signWeight+0.03;
					if(tmpw<1.0)
					{
						signWeight = tmpw;
					}
				}
				else
				{
					signWeight = signWeightDefault;
					engine.speed = speed_crosswalk;
					startHolding(2000,speed_crosswalk); //сохранять указанную скорость движения 2000 миллисекунд
				}
				break;
			};
			case sign_trafficlight_red:
			{
				int n = get_signNum(sign_trafficlight_green,Signs);
				if(n==-1 && myline.stop_line)
				{
					engine.speed = 0;
					timer_line.start();
				}
				else
				{
					stoplineInHandle = false;
					engine.speed = MAX_SPEED;
					in_handle = sign_none;
				}
				break;
			};
			case sign_giveway:
			{
				in_handle = sign_none;
				break;
			};
			case sign_mainroad:
			{
				in_handle = sign_none;
				break;
			};
			default:
			{
				break;
			};
		}
	}
	
	syst.engine_set(engine); //задать новые параметры движения робота
	
	return;
}

bool stopLineSeen = false;

void calcAngleAndSpeed(line_data &myline, Engine &engine, vector<sign_data> &Signs)
{
	if(myline.on_line)
	{
		//last_line_timer.stop();
		if (in_signs(sign_trafficlight_green, Signs)) { //  || (last_line_timer.get() < 3500 && stopLineSeen)
			engine.speed = MAX_SPEED;
			myline.stop_line = false;
			stoplineInHandle = false;
		} else {
			if(stoplineInHandle)
			{
				timer_line.stop();// last_line_timer.stop();
				if(timer_line.get() <= 1400 && myline.stop_line) //2000; && (last_line_timer.get() > 3500 || !stopLineSeen) 
				{
					if (engine.speed > MIN_SPEED)
						engine.speed /= 2;
					else
						engine.speed= MIN_SPEED;
					//return;
				}
				else
				{
					//last_line_timer.start();
					myline.stop_line = false;
					stoplineInHandle = false;
				}
			}
			else
			{
				// last_line_timer.stop();
				if(myline.stop_line) //  && (last_line_timer.get() > 3500 || !stopLineSeen)
				{
					stopLineSeen = true;
					stoplineInHandle = true;
					timer_line.start();
					engine.speed= MIN_SPEED;
					//return;
				}
			}
		}

		//calculate angle begin
		/* Вычислить отклонение робота от центра линии*/
		delta = myline.center_of_line-myline.robot_center;
		
		if(abs(delta-old_delta)>80) delta = old_delta*0.8 + delta*0.3;
		
		/* Вычислить угол поворота робота согласно его отклонению и заданным коэффициентам ПИД регулятора */
		//engine->angle = 90 - PID(delta,(double)engine.angle); //PID regulator
		engine.angle = ANGLE_CENTER - delta*1/5.9 - (delta- old_delta)*0.3; //простой PD регулятор; 6.7 - пропорциональная компонента, 0.2 - дифференциальная 
		old_delta = delta;
		
		/* Проверить вычисленное значение угла на выход за границы диапазона доступных углов сервопривода*/
		if(engine.angle<ANGLE_MIN) engine.angle =ANGLE_MIN;
		else if(engine.angle>ANGLE_MAX) engine.angle =ANGLE_MAX;
		//calculate angle end
		
		//calculate speed begin
		if(!stoplineInHandle)
		{
			int ee = abs((int32_t)engine.angle - ANGLE_CENTER);
			engine.speed = MAX_SPEED - ee*((MAX_SPEED-MIN_SPEED)/ANGLE_RANGE);
		}
		//calculate speed end
	}
	else
	{
		engine.speed = 0;
		LOG("[W]: black line not found");
	}
}

bool field[10];

signs getMaxPrioritySign(vector<sign_data> &Signs)
{
	if(Signs.size()==0) return sign_none;
	
	memset(field,0,10);
	
	for(unsigned i=0;i<Signs.size();i++)
	{
		if(Signs[i].sign == sign_trafficlight_red || Signs[i].sign == sign_trafficlight_yellow)
		{
			{
				field[Signs[i].sign] = true;
			}
		}
		else
		{
			field[Signs[i].sign] = true;
		}
	}
	
	if(field[sign_stop]) return sign_stop;
	if(field[sign_trafficlight_red]) return sign_trafficlight_red;
	if(field[sign_trafficlight_yellow]) return sign_trafficlight_yellow;
	if(field[sign_crosswalk]) return sign_crosswalk;
	if(field[sign_giveway]) return sign_giveway;
	if(field[sign_mainroad]) return sign_mainroad;
	
	return sign_none;
}
