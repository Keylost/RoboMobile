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
	
	findContours(result, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	mysign.sign = sign_none;
	mysign.state = greenlight;
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
			if(dl>1.8&&dl<=2.2) //Traffic light
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
						mysign.state = get_light(rr);
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
						mysign.sign = sign_starttrafficlight;
						mysign.state = get_light(rr);					
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
//int scan_row = 350;
int center_prev=-1;
int center =-1;
int border_left=0;
int border_right =0;
int border_left_prev=0;
int border_right_prev =0;
bool crossroad = false;

void recognize_line(const Mat& orig, line_data &myline,int scan_row)
{
	center_prev = center;
	border_left_prev = border_left;
	border_right_prev = border_right;
	
	int b=0,g=0,r=0,y=0;
	uint8_t *row;
	row = (uint8_t*)orig.ptr<uint8_t>(scan_row);
	
	if(center_prev!=-1)
	{
		int j = center_prev;
		b = row[j*3+0], g = row[j*3+1], r = row[j*3+2];
		y = b*0.0722 + g*0.7152 + r*0.2126;
		if(y<60)
		{
			int i=j+1;
			for(; i<orig.cols;i++)
			{
				b = row[i*3+0], g = row[i*3+1], r = row[i*3+2];
				y = b*0.0722 + g*0.7152 + r*0.2126;
				if(y>60) break;
			}
			border_right = i;
			i=j-1;
			for(; i>=0;i--)
			{
				b = row[i*3+0], g = row[i*3+1], r = row[i*3+2];
				y = b*0.0722 + g*0.7152 + r*0.2126;
				if(y>60) break;
			}
			border_left = i;
			center = (border_right+border_left)/2;
		}
		else
		{
			center_prev=-1;
		}
	}
	if(center_prev==-1)
	{
		int mindiff=1000;
		for(int i=0;i<orig.cols;i++)
		{
			b = row[i*3+0], g = row[i*3+1], r = row[i*3+2];
			y = b*0.0722 + g*0.7152 + r*0.2126;
			if(y<60)
			{
				int startpoint = i;
				i++;
				while(y<60 && i<orig.cols)
				{
					b = row[i*3+0], g = row[i*3+1], r = row[i*3+2];
					y = b*0.0722 + g*0.7152 + r*0.2126;
					i++;
				}
				int center_tmp=(i+startpoint)/2;
				int difftmp = abs(myline.robot_center-center_tmp);
				if(difftmp<mindiff)
				{
					mindiff = difftmp;
					center = center_tmp;
					border_left = startpoint;
					border_right = i;
				}
			}
		}
	}
	
	if(center_prev!=-1)
	{
		if(abs(border_right-border_right_prev)>80)
		{
			if(abs(border_left-border_left_prev)<20)
			{
				myline.stop_line = true;
				border_right = border_right_prev + (border_left-border_left_prev);
				center = (border_left+border_right)/2;
			}
			else
			{
				printf("crossroad \n");
				crossroad = true;
				border_left = border_left_prev;
				border_right = border_right_prev;
				center = center_prev;
			}
		}
		else if(abs(border_left-border_left_prev)>40)
		{
			border_left = border_left_prev + (border_right-border_right_prev);
			center = (border_left+border_right)/2;
			printf("crossroad \n");
			crossroad = true;
		}
	}
	
	if(center!=-1)
	{
		myline.center_of_line = center;
		myline.on_line = true;
	}
	else
	{
		myline.on_line = false;
	}
	
	return;
}

int t1=0,
	t2=0,
	t3=0,
	t4=0,
	t5=0,
	t6=0;
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
		Mat &frame = *(curObj->obj);
		
		recognize_line(frame,*(newObj->obj),350);
		if((newObj->obj)->stop_line)
		{
			t1 = center_prev;
			t2 = center;
			t3 = border_left;
			t4 = border_right;
			t5 = border_left_prev;
			t6 = border_right_prev;
			
			recognize_line(frame,*(newObj->obj),340);
			recognize_line(frame,*(newObj->obj),330);
			
			center_prev = t1;
			center = t2;
			border_left = t3;
			border_right = t4;
			border_left_prev = t5;
			border_right_prev = t6;
			
			if(crossroad)
			{
				(newObj->obj)->stop_line = false;
				crossroad = false;					
			}
			else
			{
				printf("stop line\n");
			}
		}
		
		qline.push(newObj);
		
		curObj->free();
	}
	
	return NULL;
}

void* recognize_sign_fnc(void *ptr)
{
	robotimer tm;
	long spendTime = 0;
	System &syst = *((System *)ptr);
	Rect &signarea = syst.signarea;
	int lsz = 0,gsz=0;
	vector<sign_data> SignsGlobal;
	
	Object<Mat> *curObj = NULL;
	Queue<Mat> &queue = syst.queue;
	
	while(1)
	{
		tm.start();
		curObj = queue.waitForNewObject(curObj);
		Mat frame = (*(curObj->obj))(signarea);	
		
		recognize_sign(frame);
		tm.stop();
		spendTime = tm.get();
		
		lsz = Signs.size();
		for(int i=0;i<lsz;i++)
		{
			signs cur_sign = Signs[i].sign;
			Signs[i].detect_time = 0;
			for(int j=i+1;j<lsz;j++)
			{
				if(Signs[j].sign==cur_sign)
				{
					Signs.erase(Signs.begin()+j);
					lsz--;
					j--;
				}
			}
		}
		
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
			
			SignsGlobal[i].detect_time += spendTime;
			//удалить устаревшие данные
			if(SignsGlobal[i].detect_time>1000)
			{
				SignsGlobal.erase(SignsGlobal.begin()+i);
			}
		}
		
		for(int i=0;i<lsz;i++)
		{
			SignsGlobal.push_back(Signs[i]);
		}
		
		syst.signs_set(SignsGlobal);
		
		Signs.clear();
		
		curObj->free();
	}
	
	return NULL;
}


/*
 * Функция get_light() определяет активный сигнал светофора
 * Входные данные:
 * Mat& roi - указатель на изображение светофора
 * Возвращаемые значения: см. перечисление trafficlight_states в signs.hpp
 */
uint8_t get_light(Mat& roi)
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
	
	if(rb>150 && rg>150 && gb<120 ) return redlight;
	if(gb>100 && rb>100) return yellowlight;
	else return greenlight;
}
