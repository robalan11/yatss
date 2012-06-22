#include <stdlib.h>
#include <math.h>

#include "definitions.h"
#include "utilities.h"
#include "sounds.h"

/*
 * Noise Functions
 */

float white_noise() // -1 - 1
{
	return float(rand())/RAND_MAX*2 - 1;
}

float brown_ref = 0;

float brown_noise() // -1 - 1
{
	while(true) {
		float d = white_noise()/8;
		brown_ref += d;
		if (abs(brown_ref) > 1.0) brown_ref -= d;
		else break;
	}
	return brown_ref;
}

/*
 * Waves
 */

int tri_wave(float freq, float i)
{
	return abs(int(freq*i)%SR-SR/2)-SR/4;
}

int saw_wave(float freq, int i)
{
	return abs(int(freq*i)%SR/2)-SR/4;
}

int square_wave(float freq, int i)
{
	return MAX_AMP*sign(saw_wave(freq,i));
}

int sine_wave(float freq, int i)
{
	return MAX_AMP*sin(i*freq*2*M_PI/SR);
}

int white_wave(float freq, int i)
{
	return int(white_noise()*MAX_AMP);
}

int brown_wave(float freq, int i)
{
	return int(brown_noise()*MAX_AMP);
}

/*
 * Filters
 */

int low_pass(int samp, int prev, float weight)
{
	weight = powf(weight,3);
	return weight * samp + ( 1.0 - weight ) * prev;
}

int gate(int freq, int i)
{
	return (SR - (i * freq)%SR) / (SR>>1);
}

/*
 * Instruments (temp)
 */
int inst_bass(char note, int i)
{
	return sine_wave(get_freq(note,0,1),i) + 0.7*saw_wave(get_freq(note, 0, 2),i) + 0.6*saw_wave(get_freq(note, 0, 1),i) + 0.4*saw_wave(get_freq(note,0,2)+1,i);
}

int inst_trance(char note, int octave, int i)
{
	return 0.8 * saw_wave(get_freq(note, 0, octave),i) +
		   0.8 * saw_wave(get_freq(note, 0, octave)+2,i) +
		   0.7 * saw_wave(get_freq(note, 0, octave+1),i) +
		   0.2 * saw_wave(get_freq(note, 0, octave)*1.5,i) +
		   0.2 * saw_wave(get_freq(note, 0, octave)*1.5+2,i) +
		   0.4 * saw_wave(get_freq(note, 0, octave)*2.375,i);
}

int inst_hum(char note, int octave, int i)
{
	return (tri_wave(get_freq(note, 0, octave), i) + saw_wave(get_freq(note, 0, octave), i) + square_wave(get_freq(note, 0, octave), i)) / 3;
}

int inst_dial_tone(char note, int octave, int i)
{
	return (sine_wave(get_freq(note, 0, octave),i) + sine_wave((35.0/44.0)*get_freq(note, 0, octave),i))/2;
}