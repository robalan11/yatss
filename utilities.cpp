#include "utilities.h"

int sign(int num) { return (num >= 0) ? 1 : -1; }

float get_freq(char note, int accidental, int octave) 
{
	// -1 is flat, 0 is natural, 1 is sharp
	std::map<float,float> freq; 
	freq[67.0] = 16.35f; freq[67.5] = 17.32f; // C0, C#0/Db0
    freq[68.0] = 18.35f; freq[68.5] = 19.45f; // D0, D#0/Eb0
	freq[69.0] = 20.60f;                     // E0
    freq[70.0] = 21.83f; freq[70.5] = 23.12f; // F0, F#0/Gb0
	freq[71.0] = 24.50f; freq[71.5] = 25.96f; // G0, G#0
	freq[64.5] = 25.96f; freq[65.0] = 27.50f; // Ab0, A0
	freq[65.5] = 29.14f; freq[66.0] = 30.87f; // A#0/Bb0, B0

	if (note > 96) note -= 32;
	float index = note + 0.5f*accidental;

	return freq[index]*powf(2.0,float(octave));
}

int maximum(int num, int val) 
{
	return num > val ? num : val;
}

int minimum(int num, int val) 
{
	return num < val ? num : val;
}

int clamp(int num, int min_val, int max_val)
{
	return maximum(minimum(num, max_val), min_val);
}

int clamp(int num)
{
	return clamp(num, -MAX_AMP, MAX_AMP);
}