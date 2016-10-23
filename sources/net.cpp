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

double Net::learnExample(trainData &_trainData)
{
	double error = 0;
	int maxIt = 3;
	
	//загрузить данные во входной слой(просто подставить указатель)
	inputLayer->signals = _trainData.inputs;
	
	for (int it = 0; it < maxIt; it++)
	{
		//распространить сигнал вперед и расчитать выход
		forwardPropagation();
		
		//рассчитать ошибку на выходном слое
		for (int i = 0; i < outputLayer->neuronsCount; i++)
		{
			outputLayer->errors[i] = _trainData.outputs[i] - outputLayer->signals[i];
		}
		
		//распространить ошибку назад и обновить веса
		backPropagation();
	}
	
	//получить оценку ошибки на данном примере
	for (int i = 0; i < outputLayer->neuronsCount; i++)
	{
		error += outputLayer->errors[i]*outputLayer->errors[i];
	}
	
	return error/2;	
}

void Net::train(trainDataCollection &_trainCollection)
{
	printf("\n----Training process started-----\n");
	
	double error = 1;
	int it = 0, maxIt = 10000000;
	
	while (error > 0.01 && it<maxIt) //&& Math.Abs(error - prevError) > 0.00001
	{
		error = 0;
		for (unsigned f = 0; f < _trainCollection.size(); f++)
		{
			double sq = learnExample(_trainCollection.trainCollection[f]);
			if(sq>error) error = sq;
		}
		error = sqrt(error);
		
		//if(it%25 == 0)
		{
			printf("error: %f at iter %d\n", error, it);
		}
		if(it!=0 && it%10 == 0)
		{
			printf("model saved at iter %d\n", it);
			saveModel("../models/model_tmp.mdl");
		}
		it++;
	}
	
	netError = error;
	
	printf("\n----Training process finished-----\n");
	printf("Statistic:\n");
	printf("Iterations: %d\n", it);
	printf("Error:      %f\n", error);
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

void Net::backPropagation()
{
	double learnSpeed = 0.5; //скорость обучения !!!TODO: переместить в класс
	Link *currentLink;
	
	memcpy(outputLayer->deltas,outputLayer->errors, outputLayer->neuronsCount*sizeof(double));
	
	currentLink = outputLayer->inLink;
	
	///распространить ошибку через сеть и обновить веса///
	while(currentLink != NULL)
	{
		for(int i=0;i<currentLink->outLayer->neuronsCount;i++)
		{
			currentLink->outLayer->deltas[i] = currentLink->outLayer->deltas[i]*currentLink->outLayer->activation->derivative(currentLink->outLayer->signals[i]);
			currentLink->outLayer->biases[i] = currentLink->outLayer->biases[i] + learnSpeed*currentLink->outLayer->deltas[i]; //*1.0 coz bias always is 1.0
		}
		
		if(currentLink->inLayer != inputLayer)
		{
			for(int i=0;i<currentLink->inLayer->neuronsCount;i++)
			{
				currentLink->inLayer->deltas[i] = 0;
				for(int j=0;j<currentLink->outLayer->neuronsCount;j++)
				{
					currentLink->inLayer->deltas[i] += currentLink->outLayer->deltas[j]*currentLink->weights[i][j];
				}
			}
		}
		
		for(int i=0;i<currentLink->outLayer->neuronsCount;i++)
		{
			for(int j=0;j<currentLink->inLayer->neuronsCount;j++)
			{
				currentLink->weights[j][i] = currentLink->weights[j][i] + learnSpeed*currentLink->outLayer->deltas[i]*currentLink->inLayer->signals[j];
			}
		}
		
		currentLink = currentLink->inLayer->inLink;
	}
	
	return;
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

/*
 * Cохраняет текущую обученнную модель в файл
 * Формат файла:
 * в самой первой строке записывается ошибка сети, обученнной по данной модели
 * в виде числа с плавающей точкой
 * описывается общая модель слоев сети:
 * #####
 * model_begin
 * layerType activationType neuronsCount
 * model_end
 * #####
 * 
 * model_begin - начало описания модели сети
 * model_end - конец описания модели сети
 * layerType - тип слоя, см. LayerTypes в net.hpp
 * activationType - LOGISTIC|BINARY|...| - функция активации для нейронов слоя (см. ActivationFunctions в activation.hpp)
 * neuronsCount - количество нейронов слоя (обязательно больше нуля)
 * 
 * После описания модели следуют связей сети.
 */
bool Net::saveModel(const char* filename)
{
	FILE *fp;
	
	fp = fopen(filename,"w");
	
	if(!fp)
	{
		printf("[E]: Can't open file %s\n", filename);
		return false;
	}
	
	if(inputLayer == NULL || outputLayer == NULL)
	{
		printf("[E]: Input or output layer aren't exist. Model can't be saved.\n");
		return false;		
	}
	
	///запись ошибки сети
	fprintf(fp,"%lf\n\n", netError);
	
	Link *currentLink;
	Layer *curL;
	///начало записи модели
	fprintf(fp,"model_begin\n");
	fprintf(fp,"%d %d %d\n", (int)inputLayer->layerType, (int)inputLayer->AFType, inputLayer->neuronsCount);
	
	currentLink = inputLayer->outLink;	
	while(currentLink != NULL)
	{
		curL = currentLink->outLayer;
		fprintf(fp,"%d %d %d\n", (int)curL->layerType, (int)curL->AFType, curL->neuronsCount);	
		currentLink = currentLink->outLayer->outLink;
	}
	fprintf(fp,"model_end\n\n");
	///конец записи модели
	
	///начало записи смещений
	currentLink = inputLayer->outLink;	
	while(currentLink != NULL)
	{
		curL = currentLink->outLayer;
		
		for(int i=0; i < curL->neuronsCount; i++)
		{
			fprintf(fp,"%lf ", curL->biases[i]);
		}
		fprintf(fp,"\n");
		currentLink = currentLink->outLayer->outLink;
	}
	///конец записи смещений
	
	///начало записи весов сети
	currentLink = inputLayer->outLink;	
	while(currentLink != NULL)
	{
		for(int i=0;i<currentLink->inLayer->neuronsCount; i++)
		{
			for(int j = 0; j < currentLink->outLayer->neuronsCount;j++)
			{
				fprintf(fp,"%lf ", currentLink->weights[i][j]);
			}
		}
		fprintf(fp,"\n");
		currentLink = currentLink->outLayer->outLink;
	}
	///конец записи весов сети
	
	fclose(fp);
	return true;
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

/*
void Net::runTest(trainDataCollection &_trainCollection)
{
	printf("\n----Testing process started-----\n");
	
	RTimer tm;
	size_t sumTime_sec =0;
	size_t sumTime_ms  =0;
	size_t sumTime_us  =0;
	double error = 1;
	double sumError = 0;
	double *answer = new double[outputLayer->neuronsCount];
	printf("|---------------------------|\n");
	printf("| Example №  |    Error     |\n");
	printf("|---------------------------|\n");
	
	for (unsigned f = 0; f < _trainCollection.size(); f++)
	{
		error = 0.0;
		
		tm.start();
		calculate(_trainCollection.trainCollection[f].inputs, answer);
		tm.stop();
		
		sumTime_sec += tm.get(rtimer_sec);
		sumTime_ms  += tm.get(rtimer_ms);
		sumTime_us  += tm.get(rtimer_us);
		
		for(int i=0;i<outputLayer->neuronsCount;i++)
		{
			double lError =  _trainCollection.trainCollection[f].outputs[i] - answer[i];
			error += (lError*lError)/2;
		}
		sumError += error;
		
		printf("| %10d | %12f |\n", f, error);
		printf("|---------------------------|\n");
	}
	
	printf("\n----Testing process finished-----\n");
	printf("Statistic:\n");
	printf("Average error:      %f\n", sumError/(double)_trainCollection.size());
	printf("Average time(sec):  %f\n", sumTime_sec/(double)_trainCollection.size());
	printf("Average time(ms):   %f\n", sumTime_ms/(double)_trainCollection.size());
	printf("Average time(us):   %f\n", sumTime_us/(double)_trainCollection.size());
	
	return;
}
*/

Net* Net::getSubNet(int fromLayerNumber, int toLayerNumber)
{
	Layer *curL;
	Net *nn = new Net();
	int lNumber = 0;
	
	nn->netError = netError;
	
	
	
	curL = inputLayer;
	
	//add input layer
	while(curL!=NULL)
	{
		if(fromLayerNumber == lNumber) break;
		curL = curL->outLink->outLayer;
		lNumber++;
	}	
	if(curL == NULL) return NULL;
	else nn->addInputLayer(curL->neuronsCount);
	fromLayerNumber++;
	
	//add hiddens
	curL = curL->outLink->outLayer;
	while(curL!=NULL)
	{
		if(fromLayerNumber!=toLayerNumber)
		{
			nn->addHiddenLayer(curL->neuronsCount, curL->AFType);
		}
		else
		{
			nn->addOutputLayer(curL->neuronsCount, curL->AFType);
		}
		memcpy(nn->lastAdded->biases,curL->biases,curL->neuronsCount*sizeof(double));
		for(int i=0;i<curL->inLink->inLayer->neuronsCount;i++)
		{
			memcpy(&(nn->lastAdded->inLink->weights[i][0]),&(curL->inLink->weights[i][0]),curL->neuronsCount*sizeof(double));
		}
		
		if(fromLayerNumber==toLayerNumber || curL->outLink == NULL) break;
		curL = curL->outLink->outLayer;
		fromLayerNumber++;
	}
	
	if(nn->outputLayer == NULL) return NULL;
	else return nn;
}
