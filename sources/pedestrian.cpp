#include "pedestrian.hpp"

bool ped_state;

void recognize_ped(Mat &frame, bool &ped_state)
{
	Mat result;
	frame.copyTo(result);
	cvtColor(result,result,CV_BGR2GRAY);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	simple_hist colors;
	vector<Point> approx;
	
	adaptiveThreshold(result,result,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,2); //Привести изображение к чернобелому
	
	/*
	 * Привести контуры объектов к векторному виду и аппроксимировать
	 * CV_CHAIN_APPROX_SIMPLE -сжимает горизонтальные, вертикальные и диагональные сегменты и оставляет только их конечные точки
	 * CV_RETR_TREE - способ представления иерархии контуров hierarchy. CV_RETR_TREE - полная иерархия
	 * Point(0, 0) - сдвиг точек контуров относительно изображения из которого было вырезано, то на котором мы ищем контуры(здесь не используется, поэтому 0)
	 * result - чернобелое входное изображение
	 * contours и hierarchy - структуры для хранения контуров и их иерархии соответсвенно
	 */
	findContours(result, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	
	/*
	 * Обойти в цикле все найденные на изображении контуры.
	 */
	for(size_t i=0;i<contours.size();i++)
	{
		/*
		 * Апроксимировать контуры с точностью пропорциональной их периметру
		 */
		approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.03, true);
		
		/*
		 * Вычислить площадь контура
		 */
		double area = fabs(contourArea((Mat)contours[i]));
		
		/*
		 * Игнорировать слишком маленькие и слишком большие объекты, а также незамкнутые контуры
		 */
		if (area < 3000 || area > 100000 || !isContourConvex(approx))
			continue;
		
		/* Классифицировать сегменты по количеству углов, соотношению сторон и цветов 
		 * approx.size() возвращает количество углов сегмента.
		 */
		if (approx.size() == 4) 
		{
			double dy,dx,l1,l2,l3,l4; 
			dx =approx[1].x-approx[0].x; dy = approx[1].y-approx[0].y;
			l1 = sqrt((dx*dx)+(dy*dy));
			dx = approx[2].x-approx[1].x; dy = approx[2].y-approx[1].y;
			l2 = sqrt((dx*dx)+(dy*dy));
			double dl =0;
			if(l2>l1)
				dl = l2/l1;
			else
				dl = l1/l2;			
			
			if(dl>=1.63&&dl<=1.95)
			{
				dx = approx[1].x - approx[0].x; dy = approx[3].y - approx[2].y;
				l3 = sqrt((dx*dx) + (dy*dy));
				dx = approx[2].x - approx[1].x; dy = approx[3].y - approx[4].y;
				l4 = sqrt((dx*dx) + (dy*dy));
				if (abs(l4 - l2) < 0.1*l4 && abs(l3 - l1) < 0.1*l3)
				{
					
					/*
					* Выделить сегмент
					*/
					Rect boundingarea = boundingRect(approx);
					Mat rr =  frame(boundingarea);
					
					/*
					* Посчитать количество точек определенных в simple_hist цветов внутри сегмента
					*/
					color_counter(rr,colors);
					
					if (colors.white>area*0.3 && colors.black>area*0.3) //пешеход
					{
						ped_state = true;
						LOG("[I]: Pedestrian found");
						return;
					}
				}
			}
		}
	}
	ped_state = false;
	return;
}

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
		
		recognize_ped(*(curObj->obj), ped_state);
		tm.stop();
		spendTime = tm.get();
		syst.barrier_set(ped_state);
		
		curObj->free();
	}
	
	return NULL;
}
