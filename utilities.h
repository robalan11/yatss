#pragma once

#include <map>

#include "definitions.h"

int sign(int num);

void init_scale();

float get_freq(char note, int accidental, int octave);

int minimum(int num, int val);
int maximum(int num, int val);
int clamp(int num, int min_val, int max_val);
int clamp(int num);
float clamp(float num, float min_val, float max_val);


void write_wav(char * filename, unsigned long num_samples, short int * data, int s_rate);