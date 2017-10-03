/*
 * В данном файле находятся функции,
 * устанавливающие параметры движения робота
 * в зависимости от данных анализа изображения
 */
#include "userLoop.hpp"

#define ANGLE_CENTER  			90 //угол сервомотора, при котором колеса робота смотрят прямо
#define ANGLE_RANGE  			35 //максимальное отклонение сервомотора от центрального положения
#define MAX_SPEED  				30 //максимальная скорость движения в условных единицах (от 0 до 999)
#define MIN_SPEED  				10 //минимальная скорость движения в условных единицах (от 0 до 999)
#define ANGLE_MIN  				(ANGLE_CENTER - ANGLE_RANGE)
#define ANGLE_MAX  				(ANGLE_CENTER + ANGLE_RANGE)
#define speed_crosswalk 		10 //скорость при обнаружении пешеходного перехода
#define speed_stop      		0 //скорость при обнаружении знака стоп
#define speed_trafficlight		0 //скорость при обнаружении желтого или красного сигнала светофора

double delta=0,old_delta=0; //Переменные отвечают за хранение текущего и предыдущего отклонения линии от желаемого значения.
robotimer timer_line, last_line_timer; //Вспомогательные таймеры
signs signPrev = sign_none; //Предыдущий знак, который обрабатывал робот.
const double signWeightDefault = 0.1; //Влияние знака на скорость робота. Значение по умолчанию.
double signWeight = 0.1; //Текущее влияние знака на скорость.

bool stoplineInHandle = false; //начала ли машинка обрабатывать данные о стоп-линии
signs in_handle = sign_none; //хранит код знака, который робот обрабатывает в данный момент

Engine engine; //Хранит парметры движения робота
vector<sign_data> Signs; //Хранит список знаков, находящихся в поле зрения робота
bool isPedestrian = false; //Флаг. Истина, если модель пешехода была обнаружена в кадре, иначе - ложь.
bool isAutoModel = false; //Флаг. Истина, если другая модель робота была обнаружена в кадре, иначе - ложь.

robotimer holder; //таймер отвечающий за удерживание определенной скорости на определенное время
int32_t holdFor = 0; //сколько миллисекунд удерживать определенную скорость
bool hold = false; //Флаг, показывающий включено ли удерживание определенной скорости
int32_t holdSpeed =0; //скорость, которую робот должен удерживать в течении определенного времени

/*
 * Вспомогательная функция startHolding() включает удерживание роботом скорости @speed
 * в течении @holdForMs миллисекунд
 */
void startHolding(int32_t holdForMs, int32_t speed);

/*
 * Функция calcAngleAndSpeed() расчитывает нужную скорость движения и угол поворота робота, основываясь на данных датчиков,
 * и записывает их в @engine.
 * @myline - данные о положении линии
 * @syst - список знаков, находящихся в поле зрения робота
 */
void calcAngleAndSpeed(line_data &myline, Engine &engine, vector<sign_data> &Signs);


signs getMaxPrioritySign(vector<sign_data> &Signs);

/*
 * Вспомогательная функция in_signs() возвращает значение "истина", если знак @sign
 * найден в @Signs, иначе - ложь.
 */
bool in_signs(signs sign,vector<sign_data> &Signs);

/*
 * Вспомогательная функция get_signNum() возвращает номер знака @sign в векторе @Signs
 */
int get_signNum(signs sign,vector<sign_data> &Signs);

/*
 * Функция isCrossRegulated(), основываясь на данных о знаках и светофорах в @Signs,
 * определяет является ли перекресток регулируемым.
 * Возвращает истину, если перекресток регулируемый, иначе - ложь.
 */
bool isCrossRegulated(vector<sign_data> &Signs);

/*
 * Функция userLoop() занимается обработкой данных о линии, разметке и знаках
 * Функция  должна задавать параметры движения робота в engine.
 * Избегайте использования sleep или usleep в этой функции
 */
void userLoop(line_data &myline, System &syst)
{
	if(!syst.remoteControl)
	{
		syst.signs_get(Signs); //получить информацию о знаках дорожного движения, находящихся в поле зрения робота
		syst.engine_get(engine); //получить текущие параметры движения робота
		isPedestrian = syst.pedestrian_get(); //получить данные о наличии пешеходов в кадре
		isAutoModel = syst.autoModel_get(); //получить данные о наличии других моделей робота в кадре
		
		/*
		 * Функция вычисляет скорость движения и угол поворота на основании данных
		 * о линии, и записывает эти параметры в engine
		 */
		calcAngleAndSpeed(myline,engine, Signs);
		
		if(isPedestrian)
		{
			printf("pedestrian\n");
			engine.speed = 0;
			startHolding(500,0);
		}
		if(isAutoModel)
		{
			printf("Other model detected!\n");
			if(!isCrossRegulated(Signs))
			{
				engine.speed = 0;
				startHolding(500,0);
			}
			else
			{
				printf("Other model ignored coz of regulation.\n");
			}
		}
		
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
					if(current == sign_trafficlight_red || current == sign_trafficlight_yellow || current == sign_trafficlight_yelRed)
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
					if(n==-1) // Ждать пока не загорится зеленый свет
					{
						engine.speed = 0;
						timer_line.start();
					}
					else
					{
						stoplineInHandle = false;
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
	}
	return;
}

void calcAngleAndSpeed(line_data &myline, Engine &engine, vector<sign_data> &Signs)
{
	if(myline.on_line)
	{
		if(stoplineInHandle)
		{
			timer_line.stop();
			if(timer_line.get() <= 1400 && myline.stop_line)
			{
				engine.speed= MIN_SPEED;
			}
			else
			{
				myline.stop_line = false;
				stoplineInHandle = false;
			}
		}
		else
		{
			if(myline.stop_line)
			{
				stoplineInHandle = true;
				timer_line.start();
				engine.speed = MIN_SPEED;
			}
		}

		//calculate angle begin
		/* Вычислить отклонение робота от центра линии*/
		delta = myline.center_of_line-myline.robot_center;
		
		if(abs(delta-old_delta)>80) delta = old_delta*0.8 + delta*0.3;
		
		/* Вычислить угол поворота робота согласно его отклонению и заданным коэффициентам ПИД регулятора */
		engine.angle = ANGLE_CENTER - delta*1/5.9 - (delta- old_delta)*0.3; //простой PD регулятор; 1/5.9 - пропорциональная компонента, 0.3 - дифференциальная 
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

bool field[20];

/*
 * Функция getMaxPrioritySign() определяет приоритеты знаков и возвращает код самого важного.
 */
signs getMaxPrioritySign(vector<sign_data> &Signs)
{
	if(Signs.size()==0) return sign_none;
	
	memset(field,0,20);
	
	for(unsigned i=0;i<Signs.size();i++)
	{
		field[Signs[i].sign] = true;
	}
	
	if(field[sign_stop]) return sign_stop;
	if(field[sign_trafficlight_red]) return sign_trafficlight_red;
	if(field[sign_trafficlight_yellow]) return sign_trafficlight_yellow;
	if(field[sign_trafficlight_yelRed]) return sign_trafficlight_yelRed;
	if(field[sign_crosswalk]) return sign_crosswalk;
	if(field[sign_giveway]) return sign_giveway;
	if(field[sign_mainroad]) return sign_mainroad;
	
	return sign_none;
}

void startHolding(int32_t holdForMs, int32_t speed)
{
	holder.start();
	holdFor = holdForMs;
	holdSpeed = speed;
	hold = true;
}

bool in_signs(signs sign,vector<sign_data> &Signs)
{
	for(unsigned i=0;i<Signs.size();i++)
	{
		if(sign==Signs[i].sign) return true;
	}
	return false;
}

bool isCrossRegulated(vector<sign_data> &Signs)
{
	for(unsigned i=0;i<Signs.size();i++)
	{
		if(Signs[i].sign == sign_trafficlight_green || Signs[i].sign == sign_trafficlight_red || Signs[i].sign == sign_trafficlight_yellow || Signs[i].sign == sign_trafficlight_yelRed || Signs[i].sign == sign_mainroad) return true;
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
