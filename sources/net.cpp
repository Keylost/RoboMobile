#include "net.hpp"


Layer::Layer(int _neuronsCount, ActivationFunctions _AFType)
{
	AFType = _AFType;
	neuronsCount = _neuronsCount;
	signals = new double[neuronsCount];
	errors  = new double[neuronsCount];
	deltas  = new double[neuronsCount];
	biases  = new double[neuronsCount];
	
	for(int i=0;i<neuronsCount;i++)
	{
		biases[i] = (double)(rand()%1000)/2500.0;
		if(biases[i]==0) biases[i] = 0.24;
	}
	
	switch(AFType)
	{
		case LOGISTIC:
		{
			activation = new ActivationLogistic();
			break;
		}
		case BINARY:
		{
			//activation = new ActivationLogistic();
			break;
		}
		default:
		{
			activation = new ActivationLogistic();
			break;
		}
	}
}


Link::Link(Layer *_inLayer, Layer *_outLayer)
{
	inLayer = _inLayer;
	outLayer = _outLayer;
	
	inLayer->outLink = this;
	outLayer->inLink = this;
	
	weights = new double*[inLayer->neuronsCount];
	for(int i=0;i<inLayer->neuronsCount;i++)
	{
		weights[i] = new double[outLayer->neuronsCount];
		for(int j = 0; j<outLayer->neuronsCount;j++)
		{
			weights[i][j] = (double)(rand()%1000)/2500.0;
			if(weights[i][j]==0) weights[i][j] = 0.07;
		}
	}
}

///--------------------------

bool Net::calculate(double *inputs, double *answer)
{
	if(inputs == NULL || answer == NULL) return false;
	
	//загрузить данные во входной слой(просто подставить указатель)
	inputLayer->signals = NULL;
	inputLayer->signals = inputs;
	
	forwardPropagation();
	
	//выгрузить результат работы сети TODO: заменить операциями с указателями = ускорить
	memcpy(answer,outputLayer->signals,outputLayer->neuronsCount*sizeof(double));
	
	return true;
}

void Net::forwardPropagation()
{
	Link *currentLink;
	
	currentLink = inputLayer->outLink;
	
	while(currentLink != NULL)
	{
		memset(currentLink->outLayer->signals, 0, currentLink->outLayer->neuronsCount*sizeof(double));
		for(int i=0;i<currentLink->inLayer->neuronsCount; i++)
		{
			double inpSignal = currentLink->inLayer->signals[i];
			int j = 4;
			for(; j < currentLink->outLayer->neuronsCount;j+=5)
			{
				currentLink->outLayer->signals[j] += inpSignal*(currentLink->weights[i][j]);
				currentLink->outLayer->signals[j-1] += inpSignal*(currentLink->weights[i][j-1]);
				currentLink->outLayer->signals[j-2] += inpSignal*(currentLink->weights[i][j-2]);
				currentLink->outLayer->signals[j-3] += inpSignal*(currentLink->weights[i][j-3]);
				currentLink->outLayer->signals[j-4] += inpSignal*(currentLink->weights[i][j-4]);
			}
			j-=4;
			for(; j < currentLink->outLayer->neuronsCount;j++)
			{
				currentLink->outLayer->signals[j] += inpSignal*(currentLink->weights[i][j]);
			}
		}
		
		for(int j = 0; j < currentLink->outLayer->neuronsCount;j++)
		{
			currentLink->outLayer->signals[j] += currentLink->outLayer->biases[j];
			currentLink->outLayer->signals[j] *= 0.01; //наклонить сигмоиду для расширения области определения
			currentLink->outLayer->signals[j] = currentLink->outLayer->activation->function(currentLink->outLayer->signals[j]);
		}
		
		currentLink = currentLink->outLayer->outLink;
	}
}

Net::Net()
{
	inputLayer = NULL;
	outputLayer = NULL;
	netError = 0.0;
	srand(time( 0 )); // автоматическая рандомизация
}

bool Net::addHiddenLayer(int neuronsCount, ActivationFunctions AFType)
{
	if(inputLayer == NULL || outputLayer != NULL) return false;
	
	Layer *newHidden = new Layer(neuronsCount, AFType);
	Link *newLink = new Link(lastAdded, newHidden);
	lastAdded = newHidden;
	
	lastAdded->layerType = HIDDEN;
	
	if(newLink) return true;
	else return false;
}

bool Net::addInputLayer(int neuronsCount)
{
	if(inputLayer != NULL) return false;	
	
	inputLayer = new Layer(neuronsCount, LOGISTIC);
	inputLayer->inLink = NULL;
	lastAdded = inputLayer;
	
	inputLayer->layerType = INPUT;
	
	//не нужно выделять память под сигналы входного слоя. 
	//для входного слоя просто будет подставляться указатель на входной массив
	delete[] inputLayer->signals;
	inputLayer->signals = NULL;
	
	return true;
}

bool Net::addOutputLayer(int neuronsCount, ActivationFunctions AFType)
{
	if(outputLayer != NULL) return false;
	
	outputLayer = new Layer(neuronsCount, AFType);
	Link *newLink = new Link(lastAdded, outputLayer);
	outputLayer->outLink = NULL;
	lastAdded = outputLayer;
	
	outputLayer->layerType = OUTPUT;
	
	if(newLink) return true;
	else return false;
}

bool Net::loadModel(const char* filename)
{
	FILE *fp;
	Link *currentLink;
	Layer *curL;
	ActivationFunctions _afType = LOGISTIC;
	LayerTypes _lrType = INPUT;
	int _nCnt = 0;
	
	fp = fopen(filename,"r");
	
	if(!fp)
	{
		printf("[E]: Can't open file %s\n", filename);
		return false;
	}
	
	fscanf(fp,"%lf\n\n", &netError);
	fscanf(fp, "model_begin\n");
	
	while(_lrType != OUTPUT)
	{
		if(fscanf(fp, "%d %d %d\n", (int*)(&_lrType), (int*)(&_afType), (int*)(&_nCnt)) == EOF) return false;
		switch(_lrType)
		{
			case INPUT:
			{
				addInputLayer(_nCnt);
				break;
			}
			case HIDDEN:
			{
				addHiddenLayer(_nCnt, _afType);
				break;
			}
			case OUTPUT:
			{
				addOutputLayer(_nCnt, _afType);
				break;
			}
			default:
			{
				return false;
			}
		}
	}
	fscanf(fp, "model_end\n\n");
	
	///начало чтения смещений
	currentLink = inputLayer->outLink;	
	while(currentLink != NULL)
	{
		curL = currentLink->outLayer;
		
		for(int i=0; i < curL->neuronsCount; i++)
		{
			fscanf(fp,"%lf ", &(curL->biases[i]));
		}
		fscanf(fp,"\n");
		currentLink = currentLink->outLayer->outLink;
	}
	///конец чтения смещений
	
	///начало чтения весов сети
	currentLink = inputLayer->outLink;	
	while(currentLink != NULL)
	{
		for(int i=0;i<currentLink->inLayer->neuronsCount; i++)
		{
			for(int j = 0; j < currentLink->outLayer->neuronsCount;j++)
			{
				fscanf(fp,"%lf ", &(currentLink->weights[i][j]));
			}
		}
		fscanf(fp, "\n");
		currentLink = currentLink->outLayer->outLink;
	}
	///конец чтения весов сети
	
	fclose(fp);
	
	return true;
}
