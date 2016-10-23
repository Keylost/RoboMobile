#pragma once
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <vector>

class trainData
{
	public:
	int inputsCount;
	int outputsCount; 
	
	double *inputs;
	double *outputs;
	
	trainData(int inpCount, int outCount);
};

class trainDataCollection
{
	public:
	size_t collectionSize;
	int inputsCount;
	int outputsCount; 
	
	std::vector<trainData> trainCollection;
	
	//---------------
	
	bool loadFromFile(const char* fileName);
	bool loadImagesFromLst(const char* fileName, int width, int height);
	bool loadImageForCompress(const char* fileName);
	
	void printData();
	
	inline size_t size();
	
};

///inline functions definitions

size_t trainDataCollection::size()
{
	return collectionSize;
}
