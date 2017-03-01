#include "barriers.hpp"

void* recognize_barrier_fnc(void *ptr)
{
	System &syst = *((System *)ptr);
	
	Object<Mat> *curObj = NULL;
	Queue<Mat> &queue = syst.queue;
	
	Object<line_data> *curLineData = NULL;
	Queue<line_data> &qline = syst.qline;
	
	while(1)
	{
		curObj = queue.waitForNewObject(curObj);
		curLineData = qline.waitForNewObject(curLineData);		
		line_data &line  = *(curLineData->obj);		
		Mat &frame = *(curObj->obj);
		
		//bool barrier = syst.barrier_get(); //Получить текущее состояние флага
		//syst.barrier_set(true); //Установить специальный флаг. Как только флаг будет установлен, машинка остановится
		//syst.barrier_set(false); //Снять специальный флаг. Как только флаг будет снят, машинка продолжит движение
		
		curObj->free();
		curLineData->free();
	}
}
