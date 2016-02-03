#include "recognition.hpp"

#define ANGLE_CENTER  90 //угол сервомотора, при котором колеса робота смотрят прямо
#define ANGLE_RANGE  35 //максимальное отклонение сервомотора от центрального положения
#define MAX_SPEED  990 //максимальная скорость движения в условных единицах (от 0 до 999)
#define MIN_SPEED  450 //минимальная скорость движения в условных единицах (от 0 до 999)
#define ANGLE_MIN  (ANGLE_CENTER - ANGLE_RANGE)
#define ANGLE_MAX  (ANGLE_CENTER + ANGLE_RANGE)
uint32_t speed_crosswalk = MIN_SPEED; //скорость при обнаружении пешеходного перехода
uint32_t speed_stop      = 0; //скорость при обнаружении знака стоп
uint32_t speed_trafficlight  = 0; //скорость при обнаружении желтого или красного сигнала светофора

//#define canny //раскоментируйте эту строчку, чтобы использовать детектор Канни для поиска объектов на изображении вместо адаптивной бинаризации

/*
 * Конструктор класса Recognition
 * Инициализирует поля начальными значениями
 */
Recognition::Recognition(Engine* eng)
{
	engine = eng;
	timer = 0;
	mysign.sign = sign_none;
}

/*
 * Функция get_light2() определяет активный сигнал светофора
 * Входные данные:
 * Mat& roi - указатель на изображение светофора
 * Возвращаемые значения:
 * 0 - зеленый
 * 1 - красный
 * 2 - желтый
 */
uint8_t get_light2(Mat& roi)
{
	int rg=0,rb=0,gb=0;
	int b=0,g=0,r=0;
	
	uint8_t *row;
	for(int rows=0;rows<roi.rows;rows++)
	{
		row = (uint8_t*)roi.ptr<uint8_t>(rows);
		for(int col=0;col<roi.cols;col++)
		{
			b = row[col*3]; g=row[col*3+1]; r=row[col*3+2];
			if(r-g>rg) rg=r-g;
			if(r-b>rb) rb=r-b;
			if(g-b>gb) gb=g-b;
		}
	}
	
	if(rb>150 && rg>150 && gb<120 ) return 1;
	if(gb>100 && rb>100) return 2;
	else return 0;
}

/*
 * Функция handle_sign() занимается обработкой информации о найденных знаках
 * Входные данные:
 * Mat& orig - указатель на текущее обрабатываемое цветное(BGR) изображение, полученное с камеры.
 * 
 * В зависимости от информации о знаке, содержащийся в поле mysign объекта класса Recognition, управляет поведением робота.
 * Например, устанавливает таймер, определяющий время, на которое робот должен сбросить скорость при найденнном знаке "пешеходный переход"
 * или время, в течении которого робот будет стоять у знака "стоп".
 */
bool red_catch = false;
void Recognition::handle_sign(const Mat& orig)
{
	switch(mysign.sign)
	{
	case sign_none:
	{		
		break;
	}
	case sign_stop:
	{
		if(timer==0) 
		{
			time(&timer);
		}
		else
		{
			time_t diff = time(NULL)-timer;
			if(diff<=0) 
			{		
				break;
			}
			else if(diff<=3)
			{
				engine->speed = speed_stop;
			}
			else if(diff<=5)
			{
				break;
			}
			else 
			{
				timer=0;
				mysign.sign = sign_none;
			}
		
		}
		break;
	}
	case sign_crosswalk:
	{		
		if(timer==0) 
		{
			time(&timer);
		}
		else
		{
			time_t diff = time(NULL)-timer;
			if(diff<=1) 
			{		
				break;
			}
			else if(diff<=4)
			{
				engine->speed = speed_crosswalk;
			}
			else 
			{
				timer=0;
				mysign.sign = sign_none;
			}
		
		}
		break;
	}
	case sign_trafficlight:
	{
		int k =0;
		Mat roi   = orig(Rect(Point(orig.cols/2 + mysign.area.x-k,mysign.area.y-k),Point(orig.cols/2 + mysign.area.x+mysign.area.width+k,mysign.area.y+mysign.area.height+k)));
		uint32_t light = get_light2(roi);
		
		switch(light)
		{
		case 0:
		{
			LOG("[I]: Green light found");
			mysign.state = greenlight;
			mysign.sign = sign_none;
			break;
		}		
		case 1:
		{
			LOG("[I]: Red light found");
			mysign.state = redlight;
			engine->speed = speed_trafficlight;
			break;
		}
		case 2:
		{
			LOG("[I]: Yellow light found");
			mysign.state = yellowlight;
			engine->speed = speed_trafficlight;
			break;
		}
		case 3:
		{
			LOG("[I]: Tracking error?");
			mysign.state = greenlight;
			mysign.sign = sign_none;
			break;
		}
		}
		if(myline.stop_line && mysign.state!=greenlight)
		{
			if(timer_line.get()>700) mysign.sign = sign_none;
			else if(timer_line.get()>0)
			{
				timer_line.start();
			}
		}
		
		
		break;
	}
	}
}


/*
 * Функция recognize_sign() занимается распознаванием знаков на изображении
 * Входные данные:
 * Mat& orig - указатель на текущее обрабатываемое цветное(BGR) изображение, полученное с камеры.
 * 
 * Функция определяет, какой из знаков есть на изображении и заполняет поле mysign объекта класса Recognition.
 * Если на изображении несколько знаков, то обработан будет только первый попавшийся.
 */

void Recognition::recognize_sign(const Mat& orig)
{ 
	Mat result,frame = orig(Rect(cv::Point(orig.cols/2,0), cv::Point(orig.cols-1,orig.rows/2)));
	frame.copyTo(result);
	cvtColor(result,result,CV_BGR2GRAY);
	vector<std::vector<cv::Point> > contours;
	vector<cv::Vec4i> hierarchy;
	
	#ifdef canny
	equalizeHist(result, result);
	Canny(result, result, 100, 200, 3);
	#else
	adaptiveThreshold(result,result,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,2);
	#endif
	
	findContours(result, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cv::Point(0, 0) );
	mysign.sign = sign_none;
	std::vector<cv::Point> approx;
	
	for(size_t i=0;i<contours.size();i++)
	{
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.03, true); // Approximate contour with accuracy proportional	to the contour perimeter
		double area = std::fabs(cv::contourArea((Mat)contours[i]));
		if (area < 1000 || area > 10000|| !cv::isContourConvex(approx)) // Skip small or non-convex objects
			continue;

		Rect boundingarea = boundingRect(approx);
		Mat rr =  frame(boundingarea);
		if (approx.size() == 4) // SING?
		{
			double dy,dx,l1,l2,l3,l4; 
			dx =approx[1].x-approx[0].x; dy = approx[1].y-approx[0].y;
			l1 = sqrt((dx*dx)+(dy*dy));
			dx = approx[2].x-approx[1].x; dy = approx[2].y-approx[1].y;
			l2 = sqrt((dx*dx)+(dy*dy));
			double dl = l2/l1;
			if(dl>=1.8&&dl<=2.2) //Traffic light
			{
				dx = approx[1].x - approx[0].x; dy = approx[3].y - approx[2].y;
				l3 = sqrt((dx*dx) + (dy*dy));
				dx = approx[2].x - approx[1].x; dy = approx[3].y - approx[4].y;
				l4 = sqrt((dx*dx) + (dy*dy));
				if (abs(l4 - l2) < 0.1*l4 && abs(l3 - l1) < 0.1*l3)
				{
					int colors = color_counter(rr, "black");
					if (colors>area*0.4 && colors < area*0.9)
					{
						LOG("[I]: Traffic light found");
						mysign.area = boundingarea;
						mysign.sign = sign_trafficlight;
						return;
					}
				}
			}
			if(dl>=1.4&&dl<=1.8) //start traffic light
			{
				dx = approx[1].x - approx[0].x; dy = approx[3].y - approx[2].y;
				l3 = sqrt((dx*dx) + (dy*dy));
				dx = approx[2].x - approx[1].x; dy = approx[3].y - approx[4].y;
				l4 = sqrt((dx*dx) + (dy*dy));
				if (abs(l4 - l2) < 0.1*l4 && abs(l3 - l1) < 0.1*l3)
				{
					int colors = color_counter(rr, "black");
					if (colors>area*0.4 && colors < area*0.9)
					{
						LOG("[I]: Start raffic light found");
						mysign.area = boundingarea;
						mysign.sign = sign_trafficlight;
						return;
					}
				}
			}				
			else if(dl>=0.9&&dl<=1.1) //crosswalk
			{
				dx = approx[1].x - approx[0].x; dy = approx[3].y - approx[2].y;
				l3 = sqrt((dx*dx) + (dy*dy));
				dx = approx[2].x - approx[1].x; dy = approx[3].y - approx[4].y;
				l4 = sqrt((dx*dx) + (dy*dy));
				if (abs(l4 - l2) < 0.1*l4 && abs(l3 - l1) < 0.1*l3)
				{
					int colors = color_counter(rr,"blue");
					if (colors>area*0.5 && colors<area*0.92)
					{
						colors = color_counter(rr,"black");
						if(colors>area*0.05 && colors<area*0.37)
						{
							mysign.sign = sign_crosswalk;
							mysign.area = boundingarea;
							LOG("[I]: Crosswalk found");
							return;
						}
					}
				}
			}
		}

		if (approx.size() == 8 && area>4000) //STOP SIGN
		{			
			int colors = color_counter(rr,"red");
			if(colors>1000) //check if it is really stop  && colors<6200
			//if(colors>0.3*area)
			{
				mysign.sign = sign_stop;
				mysign.area = boundingarea;
				LOG("[I]: Stop found");
				return;
			}
		}
	}
	return;
}

/*
 * Функция recognize_line() занимается распознаванием черной линии и разметки на изображении
 * Входные данные:
 * Mat& orig - указатель на текущее обрабатываемое цветное(BGR) изображение, полученное с камеры.
 * 
 * Функция определяет наличие линии на изображении и заполняет поле myline объекта класса Recognition.
 */
void Recognition::recognize_line(const Mat& orig)
{
	int min=0,max=0,minp=0,maxp=0;
	
	Mat gray,bin,roi;
	int xmin =100,xmax=540;
	int left=0,right=0;
	uint8_t *row;
	uint32_t xb=0; 
	uint32_t black=0;
	int rb_cen =0,rb_count=0;
	
	roi = orig(Rect(Point(0,360),Point(639,479)));;
	cvtColor(roi,gray,CV_BGR2GRAY);
	GaussianBlur(gray, gray, Size(7,7), 2, 0, BORDER_DEFAULT);	
	
	for(int i=gray.rows-1;i>=0;i-=5)
	{
		xb=0,black=0;
		row = (uint8_t*)gray.ptr<uint8_t>(i);
		for(int j=xmin;j<=xmax;j++)
		{
			if(row[j]<60)
			{
				xb+=j;
				black++;
			}
		}
		if(black>15)
		{
			int center = xb/black;
			if(row[center]<60)
			{
				right=center,left=center;
				int k=center;
				for(;k<gray.cols;k++) // right side
				{
					if(row[k]>=60)
					{
						right=k;
						k=gray.cols+1;
					}
				}
				if(k==gray.cols)
				{
					right=k-1;
				}
				
				k=center;
				for(;k>0;k--) // left side
				{
					if(row[k]>=60)
					{
						left=k;
						k=-1;
					}
				}
				if(k==0)
				{
					left=k;
				}
				
				if(right - left<200)
				{
					rb_cen += (center); rb_count++;
					min = min+left;max++;
				}
				else
				{
					minp+=left; maxp++;
				}
			}
			
		}
		
		
		xmin+=2;
		xmax-=2;
	}
	
	
	if(rb_count>0)
	{
		if(maxp>0)//stop or cross
		{
			if(min/max-minp/maxp<20)
			{
				myline.stop_line =true;
				printf("stopline %d %d\n",minp/maxp,min/max);
			}
		}
		int mcenter = rb_cen/rb_count;
		myline.on_line=true;
		myline.center_of_line =mcenter;
	}
	else
	{
		myline.on_line=false;
	}
}

/*
 * Функция handle_line() занимается обработкой данных о черной линии и разметке, хранящихся в поле
 * myline объекта класса Recognition
 * 
 * Функция задает параметры движения робота от данных хранящихся в myline.
 */
double delta=0,old_delta=0;
void Recognition::handle_line()
{
	if(myline.on_line)
	{
		if(myline.stop_line)
		{
			timer_line.stop();
			if(timer_line.get()<=0)
			{
				timer_line.start();
				engine->speed=0;
				return;
			}
			else
			{
				if(timer_line.get()<=700)
				{
					engine->speed=0;
					return;
				}
				//else if(timer_line.get()>700 && timer_line.get()<9000) sl_passed = true;
				else if(timer_line.get()>=9000)
				{
					timer_line.zero();
					myline.stop_line = false;
				}
			}
		}

		//calculate angle begin
		/* Вычислить отклонение робота от центра линии*/
		delta = myline.center_of_line-myline.robot_center;
		
		/* Вычислить угол поворота робота согласно его отклонению и заданным коэффициентам ПИД регулятора */
		//engine->angle = 90 - PID(delta,(double)engine.angle); //PID regulator
		engine->angle = ANGLE_CENTER - delta*1/6.7 - (delta- old_delta)*0.2; //простой PD регулятор; 6.7 - пропорциональная компонента, 0.2 - дифференциальная 
		old_delta = delta;
		
		/* Проверить вычисленное значение угла на выход за границы диапазона доступных углов сервопривода*/
		if(engine->angle<ANGLE_MIN) engine->angle =ANGLE_MIN;
		else if(engine->angle>ANGLE_MAX) engine->angle =ANGLE_MAX;
		//calculate angle end
				
		//calculate speed begin								
		else if(mysign.sign==sign_none)
		{
			int ee = abs(engine->angle - ANGLE_CENTER);
			engine->speed = MAX_SPEED - ee*((MAX_SPEED-MIN_SPEED)/ANGLE_RANGE); //
		}
		//calculate speed end				
	}
	else
	{
		engine->speed = 0;
		LOG("[W]: black line not found");
	}
}
