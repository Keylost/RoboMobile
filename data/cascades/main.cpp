#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <algorithm>
#include <iostream>
#include <math.h>
 
using namespace std;
using namespace cv;
 
int main(int argc, char **argv)
{
	
	VideoCapture cap(argv[1]); // open the default camera
	if(!cap.isOpened())  // check if we succeeded
		return -1;
	
	int width = 640;
	int height = 360;
	int fps = 60;
	
	cap.set(CV_CAP_PROP_FRAME_WIDTH,width);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT,height);
	cap.set(CV_CAP_PROP_FPS,fps);
	
	CascadeClassifier cascade; // Объявление каскада
	bool cascadeLoad = cascade.load("cascade.xml"); // Загрузка каскада
	
	if(!cascadeLoad)
	{
		cerr << "Cascade not load. Check your directory." << std::endl;
		return false;
	}
	
	Rect roir(Point(0,220),Point(640,480));  
	
	while(true)
	{
		Mat src,gray;
		cap >> src;
		Mat roi = src(roir);
		cvtColor(roi, gray, CV_BGR2GRAY);
		
		resize(gray,gray,Size(roi.cols/2, roi.rows/2));
		vector<cv::Rect> Objects;
		
		cascade.detectMultiScale(gray, Objects); // Поиск с помощью каскада
		
		for(auto& p : Objects)
		{
			Point symbolBegin = cv::Point(p.x*2, p.y*2);
			Point symbolEnd = cv::Point((p.x+p.width)*2, (p.y+p.height)*2);
	 
			cout << "X: " << p.x << " Y: " << p.y << " Width: " << p.width << " Height: " << p.height << std::endl;
			
			
			rectangle(roi, symbolBegin, symbolEnd, cv::Scalar(0,255,0), 2); 
		}
		
		printf("stage\n");
		
		imshow("Test", src);
	 
		waitKey(0);
	}
	return 0;
}
