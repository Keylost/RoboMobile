class PID
{
	private:
	double kp; // proportional term
	//integrator params
	double ki; // integral term
	double iMin; // min integrator value
	double iMax;  // max integrator value
	double iSum;  // sum of errors for integrator
	// differeciator params
	double kd;   // differenciator term
	double old_y; // previous mean of signal
	public:
	PID(double pt, double it,double dt, double MinI,double MaxI);
	double calculate(double error, double y);
};
