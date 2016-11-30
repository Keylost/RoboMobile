#include "pedestrian.hpp"

using namespace tiny_dnn;
using namespace tiny_dnn::layers;
using namespace tiny_dnn::activation;
using namespace std;
using namespace cv;

network<sequential> net[3]; //нейросеть

Rect roi(Point(180,240),Point(590,400));
Rect roi1(Point(0,1),Point(50,51));
Rect roi2(Point(42,1),Point(92,51));
Rect roi3(Point(86,1),Point(136,51));

bool ped_state;


void loadModel(network<sequential>& net, const char *model)
{
	//загрузить модель	
	net << conv<tan_h>(50, 50, 5, 1, 6, padding::same)  // in:32x32x1, 5x5conv, 6fmaps
		<< max_pool<tan_h>(50, 50, 6, 2)                // in:32x32x6, 2x2pooling
		<< conv<tan_h>(25, 25, 5, 6, 16, padding::same) // in:16x16x6, 5x5conv, 16fmaps
		<< max_pool<tan_h>(25, 25, 16, 4)               // in:16x16x16, 2x2pooling
		<< fc<tan_h>(6*6*16, 100)                       // in:8x8x16, out:100
		<< fc<tan_h>(100, 2);                        // in:100 out:10
	
	std::ifstream ifs(model);
	ifs >> net;
	return;
}

void rec_ped(Mat &frame, bool &ped_state)
{
	vec_t x_data[3];
	vec_t y_data[3];
	
	Mat_<uint8_t> img_r[3];
	
	cvtColor(frame(roi1),img_r[0],CV_BGR2GRAY);
	cvtColor(frame(roi2),img_r[1],CV_BGR2GRAY);
	cvtColor(frame(roi3),img_r[2],CV_BGR2GRAY);
	
	std::transform(img_r[0].begin(), img_r[0].end(), std::back_inserter(x_data[0]),
				[=](uint8_t c) { return c; });
	std::transform(img_r[1].begin(), img_r[1].end(), std::back_inserter(x_data[1]),
				[=](uint8_t c) { return c; });
	std::transform(img_r[2].begin(), img_r[2].end(), std::back_inserter(x_data[2]),
				[=](uint8_t c) { return c; });
	
	y_data[0] = net[0].predict(x_data[0]);
	y_data[1] = net[1].predict(x_data[1]);
	y_data[2] = net[2].predict(x_data[2]);
	
	
	printf("r1 = %f, r2 = %f, r3 = %f\n",y_data[0][0],y_data[1][0],y_data[2][0]);
	if(y_data[0][0] > 0.5 || y_data[1][0] > 0.5 || y_data[2][0] > 0.5)
	{
		imshow("www",frame(roi1));
		waitKey(0);
		ped_state = true;
	}
	//else ped_state = false;
	
	return;
}

void* recognize_ped_fnc(void *ptr)
{
	loadModel(net[0], "../data/models/modelPed.mdl");
	loadModel(net[1], "../data/models/modelPed.mdl");
	loadModel(net[2], "../data/models/modelPed.mdl");
	
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
