#include "recognition.hpp"

//#define __PROFILING__
//#define __DEBUG__

vector<sign_data> Signs;

const int thresh = 50;
const int N = 11;
Net nn[N];

signs signCode[6];
pthread_mutex_t signsLock;


/*
 * Функция recognize_sign() занимается распознаванием знаков на изображении.
 * Все найденные знаки добавляются в @Signs.
 * 
 * Входные данные:
 * @image - ссылка на текущее обрабатываемое цветное(BGR) изображение
 * 
 */
Mat gray0[3];
Mat pyr, timg;

void recognize_sign(const Mat &image, vector<sign_data> &Signs)
{
    //уменьшить и увеличить изображение для избавления от шума
    pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
    pyrUp(pyr, timg, image.size());
    
    split(timg,gray0);
    
    for( int c = 0; c < 3; c++ )
    {
		#pragma omp parallel for
        for( int l = 0; l < N; l++ )
        {
			Mat gray;
            if( l == 0 )
            {
                Canny(gray0[c], gray, 0, thresh, 5);
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
                gray = gray0[c] >= (l+1)*255/N;
            }
            
			vector<vector<Point> > contours;
            findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

            vector<Point> approx;

            for( size_t i = 0; i < contours.size(); i++ )
            {
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
                
                double area = fabs(contourArea(Mat(approx)));
                
                if(area > 400 && area < 10000 && isContourConvex(Mat(approx)))
                {
					Rect boundingarea = boundingRect(approx);
					
					
					if(boundingarea.width>120 || boundingarea.height>120) continue; // 
					if(boundingarea.width<13 || boundingarea.height<13) continue;
					double dl = (double)boundingarea.width/(double)boundingarea.height;
					if(dl<0.3 || dl>1.2) continue;
					
					Mat sng;
					Mat sng_t = image(boundingarea);
					cvtColor(sng_t,sng, CV_BGR2GRAY);
					resize(sng,sng,Size(16,16));
					double inputs[16*16];
					for(int  j=0; j<sng.rows*sng.cols;j++)
					{
						inputs[j] = (((double)sng.data[j])/255.0);
					}
					double answers[8];
					nn[l].calculate(inputs,answers);
					int max = 0;
					for(int j=1;j<9;j++)
					{
						if(answers[j]>answers[max]) max=j;
					}
					if(answers[max]>0.9)
					{
						sign_data mysign;
						mysign.sign = sign_none;
						if(max<4)
						{
							if(dl>0.80 && dl<1.2)
							{
								mysign.sign = signCode[max];
								mysign.area = boundingarea;
							}
						}
						else
						{
							if(answers[4]>0.9)
							{
								if(dl<0.7 && dl>0.3)
								{
									if(answers[5]>0.5) mysign.sign = sign_trafficlight_red;
									else if(answers[6]>0.5) mysign.sign = sign_trafficlight_yellow;
									else if(answers[7]>0.5)mysign.sign = sign_trafficlight_green;
									mysign.area = boundingarea;
								}						
							}
						}
						if(mysign.sign!=sign_none)
						{
							pthread_mutex_lock(&(signsLock));
							Signs.push_back(mysign);
							pthread_mutex_unlock(&(signsLock));
						}
					}
				}
            }
        }
    }
	return;
}

/*
 * Функция recognize_line() занимается распознаванием черной линии и разметки на изображении.
 * Результат записывается в @myline
 * Входные данные:
 * @orig - указатель на текущее обрабатываемое цветное(BGR) изображение;
 * @scan_row - номер строки матрицы изображения, по которой будет вестись поиск линии.
 */
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
				if(i-startpoint<10) continue;
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
		if(mindiff == 1000) center = -1;
	}
	
	if(center_prev!=-1)
	{
		if(abs(border_right-border_right_prev)>65)
		{
			if(abs(border_left-border_left_prev)<30)
			{
				myline.stop_line = true;
				border_right = border_right_prev + (border_left-border_left_prev);
				center = (border_left+border_right)/2;
			}
			else
			{
				#ifdef __DEBUG__
				printf("crossroad \n");
				#endif
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
			#ifdef __DEBUG__
			printf("crossroad \n");
			#endif
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
		
		recognize_line(frame,*(newObj->obj),470);
		if((newObj->obj)->stop_line)
		{
			t1 = center_prev;
			t2 = center;
			t3 = border_left;
			t4 = border_right;
			t5 = border_left_prev;
			t6 = border_right_prev;
			
			recognize_line(frame,*(newObj->obj),460);
			recognize_line(frame,*(newObj->obj),450);
			
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
	//answers[4] = 0.0;
	signCode[0] = sign_stop;
	signCode[1] = sign_crosswalk;
	signCode[2] = sign_giveway;
	signCode[3] = sign_mainroad;
	
	omp_set_num_threads(N);
	
	for(int i=0;i<N;i++)
	{
		if(!nn[i].loadModel("../data/models/model_tmp.mdl"))
		{
			printf("[E]: Can't load model\n");
		}
	}
	
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
		
		
		#ifdef __PROFILING__
		robotimer tm_p;
		tm_p.start();
		recognize_sign(frame, Signs);
		tm_p.stop();
		printf("recognize_sign() - %lu us\n",tm_p.get());
		#else
		recognize_sign(frame, Signs);
		#endif
		
		
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
			if(SignsGlobal[i].detect_time>300)
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
