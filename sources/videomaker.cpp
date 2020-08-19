#include "videomaker.hpp"
#include "OpenCVCompatibylityDefs.hpp"

/*
 * Функция init() отвечает за инициализацию объекта класса VideoMaker
 * syst - ссылка на объект класса System
 * Возвращает значение "истина" в случае успешной инициализации, "ложь" в ином случае.
 */
bool VideoMaker::init(System &syst)
{
	fps = 25;
	size = Size(640, 480);
	iscolor = true;
	output.open(syst.videoname, CV_FOURCC('M','J','P','G'), fps, size, iscolor);
    if(!output.isOpened())
    {
		return false;
	}
	
	return true;
}

/*
 * Функция write() записывает изображение @frame в видеофайл
 */
void VideoMaker::write(Mat& frame)
{
	output.write(frame);
}

/*
 * Закрывает поток записи видео в файл
 */
void VideoMaker::deinit()
{
	output.release();
}

/*
 * Функция videomaker_fnc() реализует поток записи видео с камеры робота в файл.
 * @ptr - указатель на объект класса System.
 * Начинает запись видео, если поле videomaker объекта ptr истина
 */
void* videomaker_fnc(void *ptr)
{
	System &syst = *((System *)ptr);
	VideoMaker clip;
	Object<Mat> *curObj = NULL;
	Queue<Mat> &queue = syst.queue;
	
	if(!syst.videomaker || syst.MODE == VIDEO)
	{	
		return NULL;
	}
	else
	{
		LOG("[I]: VideoMaker started.");
	}
	
	if(!clip.init(syst))
	{
		LOG("[E]: Videomaker: Can't open output stream."); //chmod or codecs problem???
		exit(EXIT_FAILURE);
	}
	
	while(1)
    {
		curObj = queue.waitForNewObject(curObj);
		
		clip.write(*(curObj->obj));
		
		curObj->free();
	}
	clip.deinit();
	return NULL;
}
