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
	
	Object();
	
	~Object();
	
	void free();
	void busy();
};

template<typename T>
class Queue
{
	pthread_mutex_t _lock;	
	Object<T> *_obj;
	
	public:
	pthread_mutex_t mutex_garbage;
	vector<Object<T> *> garbage;
	
	void add_to_garbage(Object<T> *eobj);
	
	Queue();
	
	void push(Object<T> *obj);
	
	Object<T> *waitForNewObject(Object<T> *curObj);
};

/* Это нужно, чтобы избежать проблем с линковкой шаблонных методов */
#include "queue.cppd"
