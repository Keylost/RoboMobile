#include "pedestrian.hpp"

using namespace std;
using namespace cv;

Rect roi(Point(230,200),Point(480,345));

Net nnp;
double inputs[15*30];
double answer[1];

bool ped_state;

void rec_ped(Mat &frame, bool &ped_state)
{
	Mat rsz_gray;
	cvtColor(frame,rsz_gray, CV_BGR2GRAY);
	
	adaptiveThreshold(rsz_gray,rsz_gray,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,2);
	
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(rsz_gray, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		
	for(size_t i=0;i<contours.size();i++)
	{
		double area = fabs(contourArea((Mat)contours[i]));
		
		if (area < 50 || area > 5000)
			continue;
		Rect boundingarea = boundingRect(contours[i]);
		
		if(boundingarea.height>70 || boundingarea.height<15 || boundingarea.width>40 || boundingarea.width<10) continue;
		
		double aspect = (double)boundingarea.height/(double)boundingarea.width;
		if(aspect<1.75 || aspect>2.15) continue;
		
		Mat rr = frame(boundingarea);
		
		resize(rr,rr,Size(15,30));
		
		cvtColor(rr, rr, CV_BGR2GRAY);
		
		for(int  j=0; j<15*30;j++)
		{
			inputs[j] = (((double)rr.data[j])/255.0);
		}
		
		nnp.calculate(inputs,answer);
		
		if(answer[0]>0.6)
		{
			ped_state = true;
			//printf("here %f\n", answer[0]);
			//imshow("olo", rr);
			//waitKey(0);			
			break;
		}
		
	}
	
	return;
}

void* recognize_ped_fnc(void *ptr)
{
	if(nnp.loadModel("../data/models/modelPd.mdl"))
	{
		printf("[E]: Can't load model\n");
	}
	
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
		
		Mat img_t = (*(curObj->obj))(roi);
		Mat rsz;
		resize(img_t, rsz, Size(250/2,140/2));
		
		ped_state = false;
		rec_ped(rsz, ped_state);
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
