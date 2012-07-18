class Filter{
public:
	Filter(){};
	short filter(short x0, short x1, short x2, short y1, short y2);
	short filter(short x0);
	short filter(short x0, short gain);
	short filter(short x0, float frequency);
private:
protected:
	virtual void init(float frequency, float Q){}
	double a0, a1, a2, b0, b1, b2;
	double x1, x2, y1, y2;
};

class LowPassFilter: public Filter{
public:
	LowPassFilter(float frequency, float Q);
private:
protected:
	void init(float frequency, float Q);
};

class HighPassFilter: public Filter{
public:
	HighPassFilter(float frequency, float Q);
private:
};

class BandPassFilter: public Filter{
public:
	BandPassFilter(float low, float high);
private:
};