#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <unistd.h>

using namespace std;
using namespace cv;

template<typename T> void *garbage_collector_fnc(void *ptr);

template<typename T>
class Object
{
	public:
	T *obj;
	pthread_mutex_t useLock;
	int useCount;
	
	Object()
	{
		useLock = PTHREAD_MUTEX_INITIALIZER;
		obj = new T();
		useCount = 0;
	}
	~Object()
	{
		delete obj;
	}
	
	void free()
	{
		pthread_mutex_lock(&(useLock));
		useCount--;
		pthread_mutex_unlock(&(useLock));
	}
	void busy()
	{
		pthread_mutex_lock(&(useLock));
		useCount++;
		pthread_mutex_unlock(&(useLock));
	}
};

template<typename T>
class Queue
{
	pthread_mutex_t _lock;	
	Object<T> *_obj;
	
	public:
	pthread_mutex_t mutex_garbage;
	vector<Object<T> *> garbage;
	
	void add_to_garbage(Object<T> *eobj)
	{
		pthread_mutex_lock(&(mutex_garbage));
		garbage.push_back(eobj);
		pthread_mutex_unlock(&(mutex_garbage));
	}
	
	Queue()
	{
		_obj = NULL;
		mutex_garbage = PTHREAD_MUTEX_INITIALIZER;
		_lock = PTHREAD_MUTEX_INITIALIZER;
		pthread_t garbage_collector;
		pthread_create(&garbage_collector, NULL, garbage_collector_fnc<T>, this);
	}
	
	void push(Object<T> *obj)
	{
		obj->useCount=1; //очередь получает объект раньше всех потоков
		pthread_mutex_lock(&(_lock));
		if(_obj!=NULL) _obj->free();
		_obj = obj;
		pthread_mutex_unlock(&(_lock));
		add_to_garbage(obj);
		return;
	}
	
	/*
	Object<T> *pop()
	{		
		
		Object<T> *obj = _obj;
		if(_obj!=NULL) _obj->useCount++;
		
		return obj;
	}
	*/
	
	Object<T> *waitForNewObject(Object<T> *curObj)
	{
		Object<T> *oldobj = curObj;
		while(1)
		{
			pthread_mutex_lock(&(_lock));
			curObj = _obj;
			pthread_mutex_unlock(&(_lock));
			if(curObj==NULL || oldobj==curObj)
			{
				usleep(1000); //1 ms
			}
			else
			{
				curObj->busy();
				break;
			}
		}
		return curObj;
	}
};

template<typename T>
void *garbage_collector_fnc(void *ptr)
{
	pthread_mutex_t &mutex_garbage = ((Queue<T> *)ptr)->mutex_garbage;
	vector<Object<T> *> &garbage = ((Queue<T> *)ptr)->garbage;
	
	while(1)
	{
		sleep(1);
		pthread_mutex_lock(&(mutex_garbage));
		vector<Object<T> *> gcopy(garbage);
		pthread_mutex_unlock(&(mutex_garbage));

		int sz = gcopy.size();
		printf("FPS: %d \n",sz);
		for(int i=0;i<sz;i++)
		{
			if(gcopy[i]->useCount==0)
			{
				delete gcopy[i];
				gcopy.erase(gcopy.begin()+i);
				sz--;
			}
		}
	
		pthread_mutex_lock(&(mutex_garbage));
		gcopy.swap(garbage);
		pthread_mutex_unlock(&(mutex_garbage));
	}
}


/*
 * Функция блокирует выполнение потока до тех пор пока в очереди не появится
 * новый элемент отличающийся от @curObj. После чего функция вернет управление 
 * изменив адрес curObj на адрес нового объекта
 */
/*
void waitForNewObject(Object *curObj)
{
	Object *oldobj = curObj;
	while(1)
	{
		pop(obj);
		if(curObj==NULL || oldobj==curObj)
		{
			usleep(1000); //1 ms
		}
		else break;
	}
	return;
}
*/
