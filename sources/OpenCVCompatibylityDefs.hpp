#pragma once

#if(CV_MAJOR_VERSION >= 3)
	#define CV_IMWRITE_JPEG_QUALITY IMWRITE_JPEG_QUALITY
	#define CV_CAP_PROP_FRAME_WIDTH CAP_PROP_FRAME_WIDTH
	#define CV_CAP_PROP_FRAME_HEIGHT CAP_PROP_FRAME_HEIGHT
	#define CV_CAP_PROP_FPS CAP_PROP_FPS
	#define CV_BGR2GRAY COLOR_BGR2GRAY
	#define CV_RETR_TREE RETR_TREE
	#define CV_CHAIN_APPROX_SIMPLE CHAIN_APPROX_SIMPLE
	#define CV_FOURCC VideoWriter::fourcc
	#define CV_LOAD_IMAGE_COLOR IMREAD_COLOR
	#define CV_LOAD_IMAGE_GRAYSCALE IMREAD_GRAYSCALE
	#define CV_CAP_PROP_FRAME_COUNT CAP_PROP_FRAME_COUNT
	#define CV_CAP_PROP_POS_FRAMES CAP_PROP_POS_FRAMES
	#define CV_FILLED FILLED
	#define CV_GRAY2BGR COLOR_GRAY2BGR
	#define CV_BGR2YCrCb COLOR_BGR2YCrCb
	#define CV_YCrCb2BGR COLOR_YCrCb2BGR
	#define CV_THRESH_BINARY_INV THRESH_BINARY_INV
	#define CV_THRESH_OTSU THRESH_OTSU
	#define CV_BGR2HSV COLOR_BGR2HSV
	#define CV_HSV2BGR COLOR_HSV2BGR
	#define CV_BGR2Lab COLOR_BGR2Lab
#endif