#include "recognition.hpp"

//#define __PROFILING__
//#define __DEBUG__

//

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
									mysign.area = boundingarea;
									mysign.sign = sign_trafficlight_off;
									
									if(answers[5]>0.5 && answers[6]>0.5) mysign.sign = sign_trafficlight_yelRed;
									else if(answers[5]>0.5) mysign.sign = sign_trafficlight_red;
									else if(answers[6]>0.5) mysign.sign = sign_trafficlight_yellow;
									else if(answers[7]>0.5)mysign.sign = sign_trafficlight_green;									
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
inline bool isBlack(int x, uint8_t *row)
{
	int r = row[3 * x + 2];
	int g = row[3 * x + 1];
	int b = row[3 * x + 0];

	return ((b*0.0722 + g*0.7152 + r*0.2126) < 60);
}

bool crossroad = false;

int min_row = 0;
int max_row = 0;
int min_difference = 0;
int max_difference = 0;

int border_left = 0;
int border_right = 0;

int center = -1;
int center_prev = -1;
int center_first = -1;
int closest_center = -1;
int center_of_seria = -1;
int closest_border_left = -1;
int closest_border_right = -1;

void recognize_line(const Mat& orig, line_data &myline, int scan_row)
{
	uint8_t *row;
	row = (uint8_t*)orig.ptr<uint8_t>(scan_row);

	if (center_prev == -1)
	{
		center = -1;
		closest_center = -1;
		closest_border_left = -1;
		closest_border_right = -1;

		for (int i = 0; i < orig.cols; ++i)
		{
			if (isBlack(i, row))
			{
				int startPoint = i;
				int nBlackPoints = 3;
				while (nBlackPoints > 1 && i + 3 < orig.cols)
				{
					i += 3;
					nBlackPoints = 0;
					for (int j = 0; j < 3; ++j)
						nBlackPoints += int(isBlack(i + j, row));
				}
				if (i - startPoint < 25)
				{
					continue;
				}
				else
				{
					border_right = i;
					border_left = startPoint;
					center = (i + startPoint) / 2;
					if (abs(320 - center) < abs(320 - closest_center))
					{
						closest_center = center;
						closest_border_left = border_left;
						closest_border_right = border_right;
					}
				}
			}
		}

		center = closest_center;
		border_left = closest_border_left;
		border_right = closest_border_right;
	}

	if (center_prev != -1)
	{
		if (isBlack(center_prev, row))
		{
			int i;
			int nBlackPoints;
			for (i = center_prev + 1; i + 3 < orig.cols; i += 3)
			{
				nBlackPoints = 0;
				for (int j = 0; j < 3; ++j)
					nBlackPoints += int(isBlack(i + j, row));
				if (nBlackPoints <= 1)
					break;
			}
			border_right = i;

			for (i = center_prev - 1; i >= 3; i -= 3)
			{
				nBlackPoints = 0;
				for (int j = 0; j < 3; ++j)
					nBlackPoints += int(isBlack(i - j, row));
				if (nBlackPoints <= 1)
					break;
			}
			border_left = i;

			center = (border_left + border_right) / 2;
			if (abs(border_right - border_left) < 25)
			{
				center = -1;
				center_prev = -1;
				return;
			}
		}
		else
		{
			center = -1;
			center_prev = -1;
			return;
		}
	}

	if (center_first == -1 && center != -1)
	{
		center_first = center;
	}

	if (center != -1 && abs(center_first - center) > 150)
	{
		//printf("drop - %d, %d\n", center_first, center);
		center = -1;
		center_prev = -1;
		return;
	}

	if (center != -1)
	{
		if ((abs(border_right - border_left) < min_difference || min_difference == 0))
		{
			min_difference = double(abs(border_right - border_left));
			center_of_seria = center;
			min_row = scan_row;
		}
		//abs(border_right - border_left) > 120 && 
		if ((abs(border_right - border_left) > max_difference || max_difference == 0))
		{
			max_difference = double(abs(border_right - border_left));
			max_row = scan_row;
		}
	}

	center_prev = center;

	return;
}

robotimer last_line_seen;
bool seenPrevLine = false;
bool firstLineFound = true;

void* recognize_line_fnc(void *ptr)
{
	System &syst = *((System *)ptr);
	//Rect &linearea = syst.linearea;

	Object<Mat> *curObj = NULL;
	Queue<Mat> &queue = syst.queue;

	Queue<line_data> &qline = syst.qline;

	last_line_seen.start();

	while (1)
	{
		Object<line_data> *newObj = new Object<line_data>();
		curObj = queue.waitForNewObject(curObj);
		Mat &frame = *(curObj->obj);

		newObj->obj->on_line = true;
		newObj->obj->stop_line = false;

		center_first = -1;
		min_difference = 0;
		max_difference = 0;
		center_of_seria = -1;

		recognize_line(frame, *(newObj->obj), 470);
		center_first = center;

		for (int k = 0; k <= 80; k += 5)
		{
			recognize_line(frame, *(newObj->obj), 460 - k);
		}

		center = center_first;
		center_prev = center_first;

		last_line_seen.stop();
		if (min_difference != 0 && max_difference != 0)
		{
			//printf("%d - %d: %d - %d - %d\n", min_difference, max_difference, center_of_seria, min_row, max_row);			
			if ( ((max_difference > 300) && (max_difference > 6 * min_difference)) || ((max_difference > 330) && (min_difference < 200)) ) // || max_difference > 240 || min_difference > 120
			{
				if (max_row > 445)
				{
					crossroad = true;
					center_prev = -1;
					recognize_line(frame, *(newObj->obj), 365);
					center_prev = center;
					//printf("crossroad\n");
					seenPrevLine = false;
				}
				else if (firstLineFound || last_line_seen.get() >= 2000 || seenPrevLine || abs(max_difference - min_difference) < 20)
				{
					center_prev = -1;
					recognize_line(frame, *(newObj->obj), 365);
					center_prev = center;
					newObj->obj->stop_line = true;
					//printf("stop line2 - %d\n", last_line_seen.get());
					firstLineFound = false;
					seenPrevLine = true;
				}
				else
				{
					center_prev = -1;
					recognize_line(frame, *(newObj->obj), 365);
					center_prev = center;
					//printf("oops %d\n", center);
					seenPrevLine = false;
				}
			}
			else
			{				
				if (max_difference > 150 && max_difference > 2.5 * min_difference && (firstLineFound || last_line_seen.get() >= 2000 || seenPrevLine))
				{
					center_prev = center_of_seria;
					center = center_of_seria;
					newObj->obj->stop_line = true;
					//printf("stop line1 - %d\n", last_line_seen.get());
					firstLineFound = false;
					seenPrevLine = true;					
				}
				else
				{
					seenPrevLine = false;
				}
			}
		}

		if ((seenPrevLine == true && newObj->obj->stop_line == false) || crossroad == true)
		{
			last_line_seen.start();
		}

		//printf("center = %d(%d, %d)\n", center, center_first, center_of_seria);
		newObj->obj->center_of_line = center;

		if (crossroad)
		{
			crossroad = false;
			newObj->obj->stop_line = false;			
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
		if(!nn[i].loadModel("../data/models/modelSigns.mdl"))
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
	vector<sign_data> Signs; //local
	
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
