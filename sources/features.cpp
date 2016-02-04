#include "features.hpp"

/*
 * Функция init() выполняет начальную инициализацию объекта класса fps_counter,
 * отвечающего за подсчет частоты кадров
 */
void fps_counter::init()
{
      fps = 30;
      if (counter == 0)
      {
          t.start();
      }
}

/*
 * Функция get_fps() возращает частоту кадров
 * TODO: Функция работает несовсем корректно. Необходимо исправить 
 */
double fps_counter::get_fps()
{
	t.stop();
	counter++;
	if (counter > 30)
	fps = counter/(t.get()/1000);
	// overflow protection
	if (counter == (INT_MAX - 1000))
		counter = 0;
	return fps;
}

/*
 * Функция color_counter() возвращает число точек цвета описанного в @color на изображении @roi
 */
int color_counter(Mat& roi, const char* color)
{
	//colors = vector<int>(3); //0-r,1-g,2-b
	int r,g,b;
	int counter =0;
	uint8_t *row;
	if(!strcmp(color,"red"))
	{
	for(int rows=0;rows<roi.rows;rows++)
	{
		row = (uint8_t*)roi.ptr<uint8_t>(rows);
		for(int col=0;col<roi.cols;col++)
		{
			b=row[col*3];g=row[col*3+1];r=row[col*3+2];
			if(r>(g+b)*1.3) counter++;
			//if ((r - max(g, b))>0) counter++;
		}
	}
	}
	else if(!strcmp(color,"blue"))
	{
		for(int rows=0;rows<roi.rows;rows++)
			{
				row = (uint8_t*)roi.ptr<uint8_t>(rows);
				for(int col=0;col<roi.cols;col++)
				{
					b=row[col*3];g=row[col*3+1];r=row[col*3+2];
					if ((b - max(g, r))>20) counter++;
				}
			}
	}
	else if(!strcmp(color,"black"))
	{
		for(int rows=0;rows<roi.rows;rows++)
			{
				row = (uint8_t*)roi.ptr<uint8_t>(rows);
				for(int col=0;col<roi.cols;col++)
				{
					b=row[col*3];g=row[col*3+1];r=row[col*3+2];
					if (b<60&&g<60&&r<60) counter++;
				}
			}
	}	
	return counter;
}
