#pragma once

#include <map>

#include "definitions.h"

int sign(int num);

float get_freq(char note, int accidental, int octave);

int minimum(int num, int val);
int maximum(int num, int val);
int clamp(int num, int min_val, int max_val);
int clamp(int num);