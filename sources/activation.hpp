#pragma once
#include <math.h> //exp()

enum ActivationFunctions
{
	LOGISTIC, //f(x)=1/(1+exp(-x)) //range: [0,1]
	BINARY,  //if(x<tresh) 0 else 1  //range: [0,1]
	IDENTYTY, //f(x) = x //range: (-Inf,+Inf)
	TanH,  //f(x)=tanh(x)=(2/(1+exp(-2*x)))-1 //range: [-1,1]
};

class Activation
{
	public:
	Activation() {}
	
	virtual double function(double x) = 0;
	virtual double derivative(double x) = 0;
};

class ActivationLogistic : public Activation //soft step(sigmoidal) activation function
{
	public:
	
	ActivationLogistic() : Activation() {}
	
	double function(double x);
	
	double derivative(double x); //where x = function(x)
};

class ActivationBinary : public Activation //soft step(sigmoidal) activation function
{
	public:
	
	double threshold;
	
	ActivationBinary() : Activation() { threshold = 0.0; }
	
	double function(double x);
	
	double derivative(double x); //where x = function(x)
};

class ActivationIdentity : public Activation //soft step(sigmoidal) activation function
{
	public:
	
	ActivationIdentity() : Activation() {}
	
	double function(double x);
	
	double derivative(double x); //where x = function(x)
};

////////////////////////////////////////////////////////

///Logistic

inline double ActivationLogistic::function(double x)
{
	return 1.0 / (1.0 + exp(-x));
}

inline double ActivationLogistic::derivative(double x) //where x = function(x)
{
	return x * (1 - x);
}

///Binary

inline double ActivationBinary::function(double x)
{
	if(x < threshold) return 0.0;
	else return 1.0;
}

inline double ActivationBinary::derivative(double x) //where x = function(x)
{
	return 0;
}

///Identity

inline double ActivationIdentity::function(double x)
{
	return x;
}

inline double ActivationIdentity::derivative(double x) //where x = function(x)
{
	return 1;
}

///
