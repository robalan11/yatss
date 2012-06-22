#pragma once

#include <stdlib.h>
#include <math.h>

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

int low_pass(int samp, int prev, float weight);
int gate(int freq, int i);

/*
 * Instruments (temp)
 */
int inst_bass(char note, int i);
int inst_trance(char note, int octave, int i);
int inst_hum(char note, int octave, int i);
int inst_dial_tone(char note, int octave, int i);