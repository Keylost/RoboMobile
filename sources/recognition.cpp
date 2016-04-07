#include "recognition.hpp"

vector<sign_data> Signs;
sign_data mysign;
/*
 * Функция recognize_sign() занимается распознаванием знаков на изображении
 * Входные данные:
 * Mat& orig - указатель на текущее обрабатываемое цветное(BGR) изображение, полученное с камеры.
 * 
 * Функция определяет, какой из знаков есть на изображении и заполняет поле mysign объекта класса Recognition.
 * Если на изображении несколько знаков, то обработан будет только первый попавшийся.
 */

void recognize_sign(const Mat& frame)
{
	Mat result;
	frame.copyTo(result);
	cvtColor(result,result,CV_BGR2GRAY);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	simple_hist colors;
	
	equalizeHist(result, result);
	Canny(result, result, 0, 255, 3);
	
	findContours(result, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, Point(0, 0) );
	mysign.sign = sign_none;
	vector<Point> approx;
	
	for(size_t i=0;i<contours.size();i++)
	{
		approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.03, true); // Approximate contour with accuracy proportional	to the contour perimeter
		double area = fabs(contourArea((Mat)contours[i]));
		if (area < 1000 || area > 10000|| !isContourConvex(approx)) // Skip small or non-convex objects
			continue;

		Rect boundingarea = boundingRect(approx);
		Mat rr =  frame(boundingarea);
		color_counter(rr,colors);
		
		if (approx.size() == 3)
		{
			if(colors.red<1000 || colors.white<500 || colors.blue>100 || colors.yellow>100 ||colors.red>2500)	continue;
			printf("red %d white %d blue %d yellow %d black %d\n",colors.red,colors.white,colors.blue,colors.yellow,colors.black);
			LOG("[I]: Giveway sign found");
			mysign.area = boundingarea;
			mysign.sign = sign_giveway;
		}
		
		else if (approx.size() == 4) // SING?
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
					if (colors.black>area*0.4 && colors.black < area*0.9)
					{
						LOG("[I]: Traffic light found");
						mysign.area = boundingarea;
						mysign.sign = sign_trafficlight;
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
					if (colors.black>area*0.4 && colors.black < area*0.9)
					{
						LOG("[I]: Start raffic light found");
						mysign.area = boundingarea;
						mysign.sign = sign_trafficlight;
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
					printf("blue %d, black %d yellow %d\n",colors.blue,colors.black,colors.yellow);
					if (colors.blue>area*0.5 && colors.blue<area*0.92 && colors.black>area*0.05 && colors.black<area*0.37 && colors.yellow<area*0.4)
					{
						mysign.sign = sign_crosswalk;
						mysign.area = boundingarea;
						LOG("[I]: Crosswalk found");
					}
					else if(colors.yellow>900 && colors.blue<area*0.1)
					{
						mysign.sign = sign_mainroad;
						mysign.area = boundingarea;
						LOG("[I]: Mainroad sign found");
					}
				}
			}
		}

		else if (approx.size() == 8 && area>4000) //STOP SIGN
		{
			if(colors.red>1000) //check if it is really stop
			{
				mysign.sign = sign_stop;
				mysign.area = boundingarea;
				LOG("[I]: Stop found");
			}
		}
		
		if(mysign.sign != sign_none)
		{
			Signs.push_back(mysign);
			mysign.sign = sign_none;
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
void recognize_line(const Mat& roi, line_data &myline)
{
	int min=0,max=0,minp=0,maxp=0;
	
	Mat gray,bin;
	int xmin =100,xmax=540;
	int left=0,right=0;
	uint8_t *row;
	uint32_t xb=0; 
	uint32_t black=0;
	int rb_cen =0,rb_count=0;
	
	cvtColor(roi,gray,CV_BGR2GRAY);
	GaussianBlur(gray, gray, Size(7,7), 2, 0, BORDER_DEFAULT);	
	
	for(int i=gray.rows-1;i>=0;i-=3)
	{
		xb=0,black=0;
		int min_diff = 1000;
		int center = -1;
		row = (uint8_t*)gray.ptr<uint8_t>(i);
		for(int j=xmin;j<=xmax;j++)
		{
			if(row[j]<60)
			{
				xb+=j;
				black++;
				int start_point = j;
				j++;
				while(j<=xmax && row[j]<60) j++;
				int center_tmp = (j+start_point)/2;
				if(j-start_point>40 && abs(myline.robot_center-center)<min_diff)
				{
					min_diff=abs(myline.robot_center-center);
					center = center_tmp;
					left = start_point;
					right = j-1;
				}
			}
		}
		if(center!=-1)
		{
				if(right - left<200)
				{
					rb_cen += center; rb_count++;
					min = min+left;max++;
				}
				else
				{
					minp+=left; maxp++;
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

void* recognize_line_fnc(void *ptr)
{
	System &syst = *((System *)ptr);
	Rect &linearea = syst.linearea;
	
	Object<Mat> *curObj = NULL;
	Queue<Mat> &queue = syst.queue;
	
	Queue<line_data> &qline = syst.qline;
	
	while(1)
	{
		Object<line_data> *newObj = new Object<line_data>();
		curObj = queue.waitForNewObject(curObj);
		Mat frame = (*(curObj->obj))(linearea);
		recognize_line(frame,*(newObj->obj));
		qline.push(newObj);
		
		curObj->free();
	}
}

void* recognize_sign_fnc(void *ptr)
{
	System &syst = *((System *)ptr);
	Rect &signarea = syst.signarea;
	int lsz = 0,gsz=0;
	timespec cur_time;
	vector<sign_data> SignsGlobal;
	
	Object<Mat> *curObj = NULL;
	Queue<Mat> &queue = syst.queue;
	
	while(1)
	{
		curObj = queue.waitForNewObject(curObj);
		Mat frame = (*(curObj->obj))(signarea);
		
		recognize_sign(frame);
		
		clock_gettime(CLOCK_REALTIME, &cur_time);
		lsz = Signs.size();
		for(int i=0;i<lsz;i++)
		{
			signs cur_sign = Signs[i].sign;
			Signs[i].detect_time = cur_time;
			for(int j=i+1;j<lsz;j++)
			{
				if(Signs[j].sign==cur_sign)
				{
					Signs.erase(Signs.begin()+j);
					lsz--;
				}
			}
		}
		
		
		syst.signs_get(SignsGlobal);
		gsz = SignsGlobal.size();
		for(int i=0;i<gsz;i++)
		{
			signs cur_sign = SignsGlobal[i].sign;
			for(int j=0;j<lsz;j++)
			{
				if(cur_sign==Signs[j].sign)
				{
					SignsGlobal[i] = Signs[j];
					Signs.erase(Signs.begin()+j);
					lsz--;
				}
			}
			//удалить устаревшие данные
			if(((cur_time.tv_sec-SignsGlobal[i].detect_time.tv_sec)*1000 + (cur_time.tv_nsec-SignsGlobal[i].detect_time.tv_nsec)/1000000)>1000)
			{
				SignsGlobal.erase(SignsGlobal.begin()+i);
			}
		}
		
		for(int i=0;i<lsz;i++)
		{
			SignsGlobal.push_back(Signs[i]);
		}
		
		syst.signs_set(SignsGlobal);
		
		SignsGlobal.clear();
		Signs.clear();
		
		curObj->free();
	}
}
