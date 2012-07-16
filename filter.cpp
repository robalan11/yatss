#include <math.h>

#include "definitions.h"
#include "filter.h"

int Filter::filter(int x0, int x1, int x2, int y1, int y2)
{
	return (b0/a0)*x0 + (b1/a0)*x1 + (b2/a0)*x2 - (a1/a0)*y1 - (a2/a0)*y2;
}

LowPassFilter::LowPassFilter(float frequency, float Q)
{
	float w0 = 2*M_PI*frequency/SAMPLE_RATE;
	float cw0 = cos(w0);
	float sw0 = sin(w0);
	float alpha = sw0/(2*Q);

	a0 = 1+alpha;
	a1 = -2*cw0;
	a2 = 1-alpha;
	b0 = (1-cw0)/2;
	b1 = 1-cw0;
	b2 = (1-cw0)/2;
}

HighPassFilter::HighPassFilter(float frequency, float Q)
{
	float w0 = 2*M_PI*frequency/SAMPLE_RATE;
	float cw0 = cos(w0);
	float sw0 = sin(w0);
	float alpha = sw0/(2*Q);

	a0 = 1+alpha;
	a1 = -2*cw0;
	a2 = 1-alpha;
	b0 = (1+cw0)/2;
	b1 = -(1+cw0);
	b2 = (1+cw0)/2;
}