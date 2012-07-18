#include <stdlib.h>
#include <math.h>

#include "definitions.h"
#include "filter.h"
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

int inst_fm_bass(char note, int octave, int i)
{
	float freq = get_freq(note,0,octave+2);
	int t = i+60000;
	return sine_wave(freq+(0.1*sine_wave(3*freq+(sine_wave(1.25*freq, t)/float(MAX_AMP)), t)/float(MAX_AMP)), t) * (gate(2,i) ? gate(8,i) : 1);
}

LowPassFilter PluckLPF(3000,1.0f/sqrt(2.0f));

int inst_pluck(char note, int octave, int i, int j, short* in_buffer, short* out_buffer)
{
	float falloff = powf(maximum((SAMPLE_RATE/2-j)/(SAMPLE_RATE/2.0),0),2);
	float freq = get_freq(note,0,octave);
	int base = saw_wave(freq,j);
	int env = base*falloff;
	int lowpass = PluckLPF.filter(env, 3000*falloff);
	return lowpass;
}

int inst_ks_pluck(char note, int octave, int i, int j, short* buffer) // Karplus–Strong Algorithm
{
	float freq = get_freq(note,0,octave);
	int wavelength = SAMPLE_RATE/freq;
	if (j < wavelength) return brown_wave(freq,i);
	else {
		return 0.5*(buffer[i-wavelength]+buffer[i-wavelength+1]);
	}
}

bool crack_on = false;
int crack_start;
int crack_amp;
LowPassFilter FlamesLPF(30,1.0f/sqrt(2.0f));
HighPassFilter FlamesHPF(25,1.0f/sqrt(2.0f));

int inst_fire(int i)
{
	float noise = white_noise();
	int crack = 0;
	if (crack_on) {
		if (i-crack_start > 500) crack_on = false;
		crack = crack_amp*brown_noise()*pow(1-(0.002*(i-crack_start)),2);
	} else {
	
		if (noise > 0.99999) {
			crack_on = true;
			crack_amp = 4 * MAX_AMP * white_noise();
			crack_start = i;
		}
	}
	return clamp(FlamesHPF.filter(clamp(FlamesLPF.filter(noise*MAX_AMP)*10))*3 + crack);
}

int inst_crickets(int i, int j)
{
	int wavelength = 0.03*SAMPLE_RATE;
	int duty = 0.02*SAMPLE_RATE;
	if (i % wavelength < duty) return (0.65*sine_wave(4170,i)+0.2*sine_wave(8340,i)+0.1*sine_wave(12480,i)+0.05*sine_wave(16680,i)) * maximum(1-pow(10*(j/float(SAMPLE_RATE))-1,4),0);
	else return 0;
}