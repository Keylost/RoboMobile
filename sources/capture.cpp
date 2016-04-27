#include "capture.hpp"

/*
 * Функция capture_fnc() реализует поток захвата изображения с видеокамеры робота.
 * @ptr - указатель на структуру System.
 * Захватывает изображения с видеокамеры робота и заполняет очередь.
 * Может использовать видеофайл в качестве источника видеоданных вместо видеокамеры.
 */
void* capture_fnc(void *ptr)
{
	System &syst = *((System *)ptr);
	
	VideoCapture cap;
	Queue<Mat> &queue = syst.queue;
	
	if(syst.MODE==VIDEO)
	{
		cap.open(syst.videoname);
	}
	else
	{
		cap.open(0); // open the default camera		
	}
	
	if(!cap.isOpened())  // check if we succeeded
	{
        LOG("[E:] Cant't open video source.");
    	exit(EXIT_FAILURE);
    }
	cap.set(CV_CAP_PROP_FRAME_WIDTH,syst.capture_width);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT,syst.capture_height);
	cap.set(CV_CAP_PROP_FPS,30);
	
	robotimer r_timer;
	uint32_t fps=0;
	
	r_timer.start();
	while(1) // Capture frame from camera
	{
		if (cap.grab())
        {
			Object<Mat> *newObj = new Object<Mat>();
			cap.retrieve(*(newObj->obj),0);
			queue.push(newObj);	
		}
		else
		{
			LOG("[I]: End of video stream.");
			exit(EXIT_SUCCESS);
		}
		fps++;
		r_timer.stop();
		if(r_timer.get()>=1000)
		{
			printf("FPS: %d\n",fps);
			fps=0;
			r_timer.start();
		}		
	}
	
	return NULL;
}
