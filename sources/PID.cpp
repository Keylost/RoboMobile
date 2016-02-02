#include "PID.hpp"

/*
 * Конструктор класса PID.
 * Отвечает за начальную инициализацию объекта класса PID и устанавливает коэффициенты PID(ПИД) регулятора 
 */
PID::PID(double pt, double it,double dt, double MinI,double MaxI)
{
	kp = pt; // proportional term
	//integrator params
	ki = it; // integral term
	iMin = MinI;  // min integrator value
	iMax = MaxI;   // max integrator value
	iSum = 0;  // sum of errors for integrator
	// differeciator params
	kd = dt;   // differenciator term
	old_y = 0; // previous mean of signal
}

/*
 * Функция calculate возвращает отрегулированное значение сигнала
 * @error - значение ошибки сигнала
 * @y - требуемый уровень сигнала
 */
double PID::calculate(double error, double y)
{ 
	float up, ui, ud;	
	up = kp*error;// P component
	// I component
	iSum = iSum+error; // sum
	if(iSum<iMin) 
		iSum = iMin;  // check overflow
	if(iSum>iMax)
		iSum = iMax;
	ui = ki*iSum;
	// D component
	ud= kd*(y-old_y);
	old_y = y;
	return up+ui+ud;
}
