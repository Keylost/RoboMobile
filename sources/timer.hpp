#pragma once
#include <time.h>


class robotimer
{
	private:
	timespec tstart, tend;
	public:	
	void zero();
	void start();
	void stop();
	long int get(); //returns time in ms
};
