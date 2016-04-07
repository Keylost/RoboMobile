#include "timer.hpp"

/*
 * Функция start() отвечает за запукс таймера
 */
void robotimer::start()
{
	clock_gettime(CLOCK_REALTIME, &tstart);
}

/*
 * Функция stop() отвечает за остановку таймера
 */
void robotimer::stop()
{
	clock_gettime(CLOCK_REALTIME, &tend);
}

/*
 * Функция zero() отвечает за сброс таймера
 */
void robotimer::zero()
{
	tend.tv_sec =0;
	tstart.tv_sec=0;
	tend.tv_nsec=0;
	tstart.tv_nsec=0;
}

/*
 * Функция get() возвращает время в миллисекундах между вызовами start() и stop()
 */
long int robotimer::get()
{
	return ((tend.tv_sec-tstart.tv_sec)*1000 + (tend.tv_nsec-tstart.tv_nsec)/1000000);
}
