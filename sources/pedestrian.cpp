bool ped_state;

void recognize_ped(Mat &frame, bool &ped_state);


void* recognize_ped_fnc(void *ptr)
{
	ped_state = false;
	robotimer tm;
	long spendTime = 0;
	System &syst = *((System *)ptr);
	
	Object<Mat> *curObj = NULL;
	Queue<Mat> &queue = syst.queue;
	
	while(1)
	{
		tm.start();
		curObj = queue.waitForNewObject(curObj);
		Mat frame = (*(curObj->obj))(signarea);	
		
		recognize_ped(frame, ped_state);
		tm.stop();
		spendTime = tm.get();
		
		
		curObj->free();
	}
	
	return NULL;
}
