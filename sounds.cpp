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

int pluck_in_1 = 0.0;
int pluck_in_2 = 0.0;
int pluck_out_1 = 0.0;
int pluck_out_2 = 0.0;

int inst_pluck(char note, int octave, int i)
{
	float falloff = pow(max((SAMPLE_RATE/2-i)/(SAMPLE_RATE/2.0),0),2);
	LowPassFilter PluckLPF(3000*falloff,1.0f/sqrt(2.0f));
	float freq = get_freq(note,0,octave);
	int base = saw_wave(freq,i);
	int env = base*falloff;
	int lowpass = PluckLPF.filter(env, pluck_in_1, pluck_in_2, pluck_out_1, pluck_out_2);
	pluck_in_2 = pluck_in_1; pluck_in_1 = env;
	pluck_out_2 = pluck_out_1; pluck_out_1 = lowpass;
	return lowpass;
}

int inst_ks_pluck(char note, int octave, int i, int j, short* buffer) // Karplus–Strong Algorithm
{
	float freq = get_freq(note,0,octave);
	int wavelength = SAMPLE_RATE/freq;
	if (j < wavelength) return white_wave(0,i);
	else {
		return 0.5*(buffer[i-wavelength]+buffer[i-wavelength+1]);
	}
}

bool crack_on = false;
int crack_start;
int crack_amp;

int inst_fire(char note, int octave, int i)
{
	if (crack_on) {
		if (i-crack_start > 500) crack_on = false;
		return crack_amp*white_noise()*pow(1-(0.002*(i-crack_start)),2);
	} else {
	
		if (white_noise() > 0.9999) {
			crack_on = true;
			crack_amp = MAX_AMP * white_noise();
			crack_start = i;
		}

		return 0;
	}
}