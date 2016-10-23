#pragma once
#include <vector> //vector
#include <ctime> //time()
#include <cstdlib> //rand()
#include <math.h> //exp()
#include <memory.h> //memset()

#include "trainDataCollection.hpp"
#include "activation.hpp"
#include "timer.hpp"

/*
 * TODO block:
 * попытаться динамически вычислять наклон сигмоидальной активационной функции
 * добавить настройки сети, такие как вывод дебаг инфы с нужной частотой
 * вынести скорость обучения в основной класс
 * сделать скорость обучения адаптивной
 * сделать возможность отключить биасы(смещения) для определенных слоев
 * добавить распараллеливание
 * добавить возможность вычислений на GPU
 * добавить возможность гибридных вычислений Gpu + Cpu
 * 
 * добавить новые функции активации, такие как ReLU
 * добавить сверточные слои
 * обучить и протестировать сеть на сжатие изображений
 * добавить распределенные вычисления на множество компьютеров в сети
 */

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
	void backPropagation();
	
	void train(trainDataCollection& _trainCollection);
	void runTest(trainDataCollection& _trainCollection);
	
	/*
	 * Учит пример записанный в _trainData
	 * Возвращает ошибку сети на этом примере
	 */
	double learnExample(trainData& _trainData); //returns error on this example
	
	/*
	 * Производит распространение входных сигналов (inputs)
	 * через сеть и возвращает результат (answer) обработки входов сетью
	 */
	bool calculate(double* inputs, double* answer);
	
	bool saveModel(const char* filename);
	bool loadModel(const char* filename);
	
	/*
	 * выделяет из всей сети подсеть и возвращает на неё указатель
	* fromLayerNumber - номер слоя который будет входом подсети
	* toLayerNumber - номер слоя который будет выходом подсети
	* 
	* нумерация слоев начинается с нуля
	*/
	Net* getSubNet(int fromLayerNumber, int toLayerNumber);
};
