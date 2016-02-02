#include "videomaker.hpp"

/*
 * Функция init() отвечает за инициализацию объекта класса VideoMaker
 * syst - ссылка на структуру System
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
