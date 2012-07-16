class Filter{
public:
	Filter(){};
	int filter(int x0, int x1, int x2, int y1, int y2);
private:
protected:
	float a0, a1, a2, b0, b1, b2;
};

class LowPassFilter: public Filter{
public:
	LowPassFilter(float frequency, float Q);
private:
};

class HighPassFilter: public Filter{
public:
	HighPassFilter(float frequency, float Q);
private:
};