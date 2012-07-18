#include <math.h>

#include "definitions.h"
#include "filter.h"

short Filter::filter(short x0, short x1, short x2, short y1, short y2)
{
	return (b0/a0)*x0 + (b1/a0)*x1 + (b2/a0)*x2 - (a1/a0)*y1 - (a2/a0)*y2;
}

short Filter::filter(short x0)
{
	double y0 = (b0/a0)*x0 + (b1/a0)*x1 + (b2/a0)*x2 - (a1/a0)*y1 - (a2/a0)*y2;
	x2=x1; x1=x0;
	y2=y1; y1=y0;
	return short(y0);
}

short Filter::filter(short x0, short gain)
{
	double y0 = (b0/a0)*x0 + (b1/a0)*x1 + (b2/a0)*x2 - (a1/a0)*y1 - (a2/a0)*y2;
	x2=x1; x1=x0;
	y2=y1; y1=y0;
	return short(y0*gain);
}

short Filter::filter(short x0, float frequency)
{
	init(frequency, 1.0f/sqrt(2.0f));

	short y0 = (b0/a0)*x0 + (b1/a0)*x1 + (b2/a0)*x2 - (a1/a0)*y1 - (a2/a0)*y2;
	x2=x1; x1=x0;
	y2=y1; y1=y0;
	return y0;
}

LowPassFilter::LowPassFilter(float frequency, float Q)
{
	init(frequency, Q);
	x1=x2=y1=y2=0;
}

void LowPassFilter::init(float frequency, float Q)
{
	double w0 = 2*M_PI*frequency/SAMPLE_RATE;
	double cw0 = cos(w0);
	double sw0 = sin(w0);
	double alpha = sw0/(2*Q);

	a0 = 1+alpha;
	a1 = -2*cw0;
	a2 = 1-alpha;
	b0 = (1-cw0)/2;
	b1 = 1-cw0;
	b2 = (1-cw0)/2;
}

HighPassFilter::HighPassFilter(float frequency, float Q)
{
	double w0 = 2*M_PI*frequency/SAMPLE_RATE;
	double cw0 = cos(w0);
	double sw0 = sin(w0);
	double alpha = sw0/(2*Q);

	a0 = 1+alpha;
	a1 = -2*cw0;
	a2 = 1-alpha;
	b0 = (1+cw0)/2;
	b1 = -(1+cw0);
	b2 = (1+cw0)/2;
}

BandPassFilter::BandPassFilter(float low, float high)
{
	double frequency = low+high/2.0f;
	double BW = high/low;
	double w0 = 2*M_PI*frequency/SAMPLE_RATE;
	double cw0 = cos(w0);
	double sw0 = sin(w0);
	double alpha = sw0*sinh(log(2.0f)/2 * BW * w0/sw0);

	a0 = 1+alpha;
	a1 = -2*cw0;
	a2 = 1-alpha;
	b0 = alpha;
	b1 = 0;
	b2 = -alpha;
}