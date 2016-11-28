#include "barrierRec.hpp"

bool barrierLocal = false;

void* barrier_fnc(void *ptr)
{
	System &syst = *((System *)ptr);
	Object<Mat> *prevObj = NULL;
	Object<Mat> *curObj = NULL;
	Queue<Mat> &queue = syst.queue;
	
	syst.barrier_get(barrierLocal);
	
	while(1)
    {
		prevObj = queue.waitForNewObject(prevObj);
		curObj = queue.waitForNewObject(curObj);
		
		if(checkBarrier(*(curObj->obj),*(prevObj->obj)) != barrierLocal)
		{
			barrierLocal = (!barrierLocal);
			syst.barrier_set(barrierLocal);
		}
		
		prevObj->free();
		curObj->free();
	}	
	
	return NULL;
}

bool checkBarrier(Mat &cur, Mat &prev)
{
	int32_t counter = 0;
	Mat image = cur - prev;
	
	uint8_t *row;	
	for(int i=300;i<image.rows;i++)
	{
		row = (uint8_t*)image.ptr<uint8_t>(i);
		for(int j=0;j<image.cols;j++)
		{
			int b=row[j*3],g=row[j*3+1],r=row[j*3+2];
			int bg = abs(b-g),br = abs(b-r), rg=abs(r-g);
			if(bg>40 || br >40 || rg>40) counter++;
		}
	}
	
	if(counter>1250) return true;
	else return false;
}
