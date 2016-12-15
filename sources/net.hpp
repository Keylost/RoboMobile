#pragma once
#include <vector> //vector
#include <ctime> //time()
#include <cstdlib> //rand()
#include <math.h> //exp()
#include <memory.h> //memset()
#include <stdio.h>

#include "activation.hpp"

enum LayerTypes
{
	INPUT = 0,
	HIDDEN = 1,
	OUTPUT = 2
};

class Link;

class Layer
{
	public:
	LayerTypes layerType;
	ActivationFunctions AFType;
	int neuronsCount;
	double *signals;
	double *errors; //хранит ошибки
	double *deltas;
	double *biases;
	
	Link *inLink;
	Link *outLink;
	Activation *activation;
	
	Layer(int _neuronsCount, ActivationFunctions _AFType);
};

class Link
{
	public:
	Layer *inLayer;
	Layer *outLayer;
	double **weights;
	
	Link(Layer *_inLayer, Layer *_outLayer);
};

class Net
{
	public:
	
	Layer *inputLayer;
	Layer *outputLayer;
	Layer *lastAdded;
	
	double netError; //поле будет хранить общую ошибку обученной сети 

	Net();
	bool addInputLayer(int neuronsCount);
	bool addHiddenLayer(int neuronsCount, ActivationFunctions AFType);
	bool addOutputLayer(int neuronsCount, ActivationFunctions AFType);
	
	void forwardPropagation();
	
	/*
	 * Производит распространение входных сигналов (inputs)
	 * через сеть и возвращает результат (answer) обработки входов сетью
	 */
	bool calculate(double* inputs, double* answer);
	
	bool loadModel(const char* filename);
};
