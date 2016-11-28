#include "pedestrian.hpp"

using namespace tiny_dnn;
using namespace tiny_dnn::layers;
using namespace tiny_dnn::activation;
using namespace std;
using namespace cv;

network<sequential> net; //нейросеть
bool ped_state;

void rec_ped(Mat &frame, bool &ped_state)
{
	Rect roi1(Point(0,1),Point(50,51));
	Rect roi2(Point(42,1),Point(92,51));
	Rect roi3(Point(86,1),Point(136,51));
	
	vec_t x_data1;
	Mat_<uint8_t> img_r1;
	cvtColor(frame(roi1),img_r1,CV_BGR2GRAY);
	
	std::transform(img_r1.begin(), img_r1.end(), std::back_inserter(x_data1),
               [=](uint8_t c) { return c * scale; });	
	
	vec_t y_vector1 = net.predict(x_data1);
	
	if(y_vector1[0] > 0.5) ped_state = true;
	else ped_state = false;
	
	return;
}

void* recognize_ped_fnc(void *ptr)
{
	net.load("my-network"); //загрузить модель
	
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
	
	Rect roi(Point(180,240),Point(590,400));
	Rect roi1(Point(0,1),Point(50,51));
	Rect roi2(Point(42,1),Point(92,51));
	Rect roi3(Point(86,1),Point(136,51));
		
	while(1)
	{
		tm.start();
		curObj = queue.waitForNewObject(curObj);
		
		Mat img_t = image(*(curObj->obj));
        Mat rsz;
        resize(img_t, rsz, Size(410/3,160/3));
				
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
