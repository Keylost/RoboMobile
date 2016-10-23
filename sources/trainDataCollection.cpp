#include "trainDataCollection.hpp"

//using 
trainData::trainData(int inpCount, int outCount)
{
	inputsCount = inpCount;
	outputsCount = outCount;
	
	inputs = new double[inputsCount];
	outputs = new double[outputsCount];
}

/*
 * Загружает данные из файла 
 * Формат входного файла:
 * В первой строке последовательно записаны два числа(разделены пробелом): 
 * 1-ое - это число входных данных для одного примера
 * 2-ое - это число выходных данных для одного примера
 * В последующих строках записываются примеры (1 -пример на одну строку)
 * сначала перечисляются все входы, после них выходы. Допускается 
 * использование только чисел целых и с плавающей точкой. 
 * Все числа одного примера разделены пробелами
 */
bool trainDataCollection::loadFromFile(const char* fileName)
{
	FILE *fp;
	fp = fopen(fileName,"r");
	if(!fp)
	{
		return false;
	}
	
	fscanf(fp,"%d %d",&inputsCount,&outputsCount);
	
	int inpR=0,outR=0;
	double cur = 0.0;
	trainData *dt = new trainData(inputsCount, outputsCount);
	
	while(fscanf(fp,"%lf", &cur) != EOF)
	{
		if(inpR<inputsCount)
		{
			dt->inputs[inpR] = cur;
			inpR++;
		}
		else if(outR<outputsCount)
		{
			dt->outputs[outR] = cur;
			outR++;
		}
		
		if(outR == outputsCount)
		{
			inpR = 0;
			outR = 0;
			trainCollection.push_back(*dt);
			dt = new trainData(inputsCount, outputsCount);
		}
	}
	
	collectionSize = trainCollection.size();
	
	fclose(fp);
	
	return true;
}

/*
 * Загружает изображения для обучения сети из файла списка.
 * Формат входного файла:
 * В первой строке последовательно записано два числа(разделены пробелом):
 * 2-ое - это число выходных данных сети(количество классов изображений)
 * 
 * путь_к_файлу класс
 * 
 * если класс = -1, то изображение не должно быть классифицировано сетью(мусор)
 */
bool trainDataCollection::loadImagesFromLst(const char* fileName, int width, int height)
{
	FILE *fp;
	fp = fopen(fileName,"r");
	if(!fp)
	{
		return false;
	}
	
	uint32_t exampleCount = 0;
	uint32_t classCount = 0;
	char path[256];
	int curClass = 0;
	std::vector<int> classList;
	
	while(fscanf(fp, "%s %d\n", path, &curClass) != EOF)
	{
		exampleCount++;
		if(curClass!=-1)
		{
			unsigned i=0;
			while(i<classList.size())
			{
				if(curClass == classList[i]) break;
				i++;
			}
			if(i == classList.size()) classList.push_back(curClass);
		}
	}
	classCount = classList.size();
	
	inputsCount = width*height; //if grayscale
	outputsCount = classCount;
	
	fclose(fp);
	fp = fopen(fileName,"r");
	
	trainData *dt = new trainData(inputsCount, outputsCount);
	
	while(fscanf(fp, "%s %d\n", path, &curClass) != EOF)
	{
		cv::Mat img = cv::imread(path,1);
		
		if(img.empty())
		{
			printf("[W]: can't read file %s\n",path);
		}
		
		if(img.rows!=width || img.cols !=height)
		{
			cv::resize(img,img,cv::Size(width,height));
		}
		
		//cv::imshow("test",img);
		//cv::waitKey(0);
		
		cv::cvtColor(img,img,CV_BGR2GRAY);
		for(int  i=0; i<img.rows*img.cols;i++)
		{
			dt->inputs[i] = (((double)img.data[i])/255.0);
		}
		
		for(int i=1;i<=classCount;i++)
		{
			if(i==curClass)
			{
				dt->outputs[i-1] = 1.0;
			}
			else
			{
				dt->outputs[i-1] = 0.0;
			}
		}
		
		trainCollection.push_back(*dt);
		dt = new trainData(inputsCount, outputsCount);
	}	
	
	printf("Examples loaded: %d\n", exampleCount);
	printf("Classes loaded: %d\n", classCount);
	
	collectionSize = exampleCount;
		
	fclose(fp);
	
	return true;	
}

bool trainDataCollection::loadImageForCompress(const char* fileName)
{
	int sz = 0;
	trainData *dt;
	
	cv::Mat image;
	image = cv::imread(fileName,1);
	
	sz = image.cols*image.rows*3;
	dt = new trainData(sz, sz);
	
	for(int i=0;i<sz;i++)
	{
		dt->inputs[i] = image.data[i]/255.0;
		dt->outputs[i] = dt->inputs[i];
	}
	
	trainCollection.push_back(*dt);
	
	collectionSize = trainCollection.size();
	inputsCount = sz;
	outputsCount = sz;
	
	return true;	
}

void trainDataCollection::printData()
{
	for(unsigned i=0;i<trainCollection.size();i++)
	{
		printf("\nCollection number %d\n", i);
		printf("Inputs:\n");
		for(int j=0;j<trainCollection[i].inputsCount;j++)
		{
			printf(" %f" ,trainCollection[i].inputs[j]);
		}
		printf("\n");
		
		printf("Outputs:\n");
		for(int j=0;j<trainCollection[i].outputsCount;j++)
		{
			printf(" %f" ,trainCollection[i].outputs[j]);
		}
		printf("\n");
	}
	
	return;
}
