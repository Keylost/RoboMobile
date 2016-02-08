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
 * Функция color_counter() возвращает число точек для каждого цвета описанного в @color на изображении @roi
 */
void color_counter(Mat& roi, simple_hist &color)
{
	int32_t r,g,b;
	uint8_t *row;
	color.red = 0;
	color.blue = 0;
	color.black = 0;
	color.yellow =0;
	
	for(int32_t rows=0;rows<roi.rows;rows++)
	{
		row = (uint8_t*)roi.ptr<uint8_t>(rows);
		for(int32_t col=0;col<roi.cols;col++)
		{
			b=row[col*3];g=row[col*3+1];r=row[col*3+2];
			//bg = 
			if(b<=128 && abs(b-g)<20 && abs(b-r)<20 && abs(r-g)<20) color.black++;
			else if(b>128 && abs(b-g)<20 && abs(b-r)<20 && abs(r-g)<20) continue;
			else
			{
			//if(b>200&&g>200&&r>200) continue;
			if(r>(g+b)*1.3) color.red++;
			if((b - max(g, r))>30) color.blue++;
			if(g-b>30 && r-b>30) color.yellow++;
			}
		}
	}	
}
