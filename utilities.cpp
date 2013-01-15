#include "utilities.h"
#include <stdio.h>
#include <math.h>

int sign(int num) { return (num >= 0) ? 1 : -1; }

std::map<float,float> freq; 

void init_scale()
{
	// -1 is flat, 0 is natural, 1 is sharp
	freq[67.0] = 16.35f; freq[67.5] = 17.32f; // C0, C#0/Db0
    freq[68.0] = 18.35f; freq[68.5] = 19.45f; // D0, D#0/Eb0
	freq[69.0] = 20.60f;                     // E0
    freq[70.0] = 21.83f; freq[70.5] = 23.12f; // F0, F#0/Gb0
	freq[71.0] = 24.50f; freq[71.5] = 25.96f; // G0, G#0
	freq[64.5] = 25.96f; freq[65.0] = 27.50f; // Ab0, A0
	freq[65.5] = 29.14f; freq[66.0] = 30.87f; // A#0/Bb0, B0
}

float get_freq(char note, int accidental, int octave) 
{
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

float clamp(float num, float min_val, float max_val)
{
	return num < min_val ? min_val : num > max_val ? max_val : num;
}











void write_wav(char * filename, unsigned long num_samples, short int * data, int s_rate)
{
    FILE* wav_file;
    unsigned int sample_rate;
    unsigned int num_channels = CHANNELS;
    unsigned int bytes_per_sample = 2;
	unsigned short bits_per_sample = 8*bytes_per_sample;
	unsigned short block_align = num_channels*bytes_per_sample;
    unsigned int byte_rate;
	unsigned int file_size = 36 + bytes_per_sample* num_samples*num_channels;
	unsigned int data_size = bytes_per_sample * num_samples * num_channels;
	unsigned int SubChunk1size = 16;
	unsigned short PCM_format = 1;
    unsigned long i;    /* counter for samples */
  
    if (s_rate<=0) sample_rate = SAMPLE_RATE;
    else sample_rate = (unsigned int) s_rate;
 
    byte_rate = sample_rate*num_channels*bytes_per_sample;
 
    wav_file = fopen(filename, "wb");
 
    /* write RIFF header */
    fwrite("RIFF", 1, 4, wav_file);
    fwrite(&file_size, 4, 1, wav_file);
    fwrite("WAVE", 1, 4, wav_file);
 
    /* write fmt  subchunk */
    fwrite("fmt ", 1, 4, wav_file);
    fwrite(&SubChunk1size, 4, 1, wav_file);
    fwrite(&PCM_format, 2, 1, wav_file);
    fwrite(&num_channels, 2, 1, wav_file);
    fwrite(&sample_rate, 4, 1, wav_file);
    fwrite(&byte_rate, 4, 1, wav_file);
    fwrite(&bits_per_sample, 2, 1, wav_file);  /* block align */
    fwrite(&bits_per_sample, 2, 1, wav_file);
 
    /* write data subchunk */
    fwrite("data", 1, 4, wav_file);
    fwrite(&data_size, 4, 1, wav_file);
    for (i=0; i< num_samples; i++)
		fwrite(&(data[i]), bytes_per_sample, 1, wav_file);
 
    fclose(wav_file);
}
