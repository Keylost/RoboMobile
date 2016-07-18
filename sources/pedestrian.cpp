#include "pedestrian.hpp"

bool ped_state;

vector<Point> pedTempl;


void rec_ped(Mat &frame, bool &ped_state)
{
	int32_t r,g,b;
	uint8_t *row,*bin;
	ped_state = false;
	
	Mat binary(frame.rows, frame.cols, CV_8UC1);
	
	for(int32_t rows=0;rows<frame.rows;rows++)
	{
		row = (uint8_t*)frame.ptr<uint8_t>(rows);
		bin = (uint8_t*)binary.ptr<uint8_t>(rows);
		for(int32_t col=0;col<frame.cols;col++)
		{
			b=row[col*3];g=row[col*3+1];r=row[col*3+2];
			if((b<=100 && abs(b-g)<25 && abs(b-r)<25 && abs(r-g)<25))
			{
				bin[col] = 255;
				//printf("hare\n");
			}
			else
			{
				bin[col] = 0;
			}
		}
	}
	
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(binary, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	
	
	for( size_t i = 0; i < contours.size(); i++ )
	{
		double area = fabs(contourArea(Mat(contours[i])));
		if(area > 2000 && area < 15000)
		{
			Rect boundingarea = boundingRect(contours[i]);
			Mat segm =  frame(boundingarea);
			
			double level = matchShapes(contours[i], pedTempl, CV_CONTOURS_MATCH_I1,0);
			
			//printf("level: %f\n", level);
			if(level<0.18)
			{
				ped_state = true;
				return;
			}
			//drawContours( frame, contours, i, Scalar(0,255,0), 2, 8, hierarchy, 0, Point() );
		}
	}
	
	return;
}



void getTempl()
{
	Mat frame = imread("ped.png",1);
	int32_t r,g,b;
	uint8_t *row,*bin;
	
	Mat binary(frame.rows, frame.cols, CV_8UC1);
	
	for(int32_t rows=0;rows<frame.rows;rows++)
	{
		row = (uint8_t*)frame.ptr<uint8_t>(rows);
		bin = (uint8_t*)binary.ptr<uint8_t>(rows);
		for(int32_t col=0;col<frame.cols;col++)
		{
			b=row[col*3];g=row[col*3+1];r=row[col*3+2];
			if((b<=100 && abs(b-g)<25 && abs(b-r)<25 && abs(r-g)<25))
			{
				bin[col] = 255;
				//printf("hare\n");
			}
			else
			{
				bin[col] = 0;
			}
		}
	}
	vector<vector<Point> > contours;
	findContours(binary, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	
	if(contours.size()==1)
	{
		pedTempl = contours[0];
	}
	else
	{
		printf("what??\n");
		exit(0);
	}
}



void* recognize_ped_fnc(void *ptr)
{
	getTempl();
	ped_state = false;
	robotimer tm;
	long spendTime = 0;
	System &syst = *((System *)ptr);
	
	Object<Mat> *curObj = NULL;
	Queue<Mat> &queue = syst.queue;
	
	vector<sign_data> Signs;
	int last_seen = 10000;
	bool fls = false;
	bool tr = true;
	
	while(1)
	{
		tm.start();
		curObj = queue.waitForNewObject(curObj);		
		rec_ped(*(curObj->obj), ped_state);
		tm.stop();
		spendTime = tm.get();
		
		if(ped_state == true)
		{
			syst.signs_get(Signs);
			unsigned i=0;
			for(;i<Signs.size();i++)
			{
				if(Signs[i].sign == sign_crosswalk) break;
			}
			if(i==Signs.size()) ped_state = false;
		}
		
		if(ped_state == true)
		{
			last_seen = 0;
		}
		else
		{
			last_seen += spendTime;
		}
		
		if(last_seen>1000)
		{
			 
			syst.barrier_set(fls);
			last_seen = 10000;			
		}
		else
		{
			syst.barrier_set(tr);
		}
		
		curObj->free();
	}
	
	return NULL;
}
