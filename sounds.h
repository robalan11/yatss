#pragma once

#include <stdlib.h>
#include <math.h>
#include <map>

#include "definitions.h"
#include "utilities.h"

/*
 * Noise Functions
 */
float white_noise();
float brown_noise();

/*
 * Waves
 */
int tri_wave(float freq, float i);
int saw_wave(float freq, int i);
int square_wave(float freq, int i);
int sine_wave(float freq, int i);
int white_wave(float freq, int i);
int brown_wave(float freq, int i);

/*
 * Filters
 */

int gate(int freq, int i);

/*
 * Envelopes
 */
class Env_Point{
public:
	Env_Point(){}

	void init(int _start, float _value)
	{
		start = _start;
		value = _value;
	}

	Env_Point operator=(const Env_Point& other)
	{
		start = other.start;
		value = other.value;

		return *this;
	}

	int start;
	float value;
private:
};
float envelope(Env_Point* values, int i);

/*
 * Instruments (temp)
 */
int inst_bass(char note, int i);
int inst_trance(char note, int octave, int i);
int inst_hum(char note, int octave, int i);
int inst_dial_tone(char note, int octave, int i);
int inst_fm_bass(char note, int octave, int i);
int inst_pluck(char note, int octave, int i, int j);
int inst_ks_pluck(char note, int octave, int i, int j, short* buffer);
int inst_ks_pluck2(char note, int octave, int i, int j, short* buffer); // second copy for temp
int inst_ks_pluck3(char note, int octave, int i, int j, short* buffer); // third copy for temp
int inst_fire(int i);
int inst_crickets(int i, int j);
int inst_flute(char note, int octave, int i, int j);
int inst_bell(char note, int octave, int i, int j, Env_Point* values, int size);