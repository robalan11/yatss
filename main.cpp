#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <map>
#include <utility>

#include "definitions.h"
#include "song.h"
#include "sounds.h"
/*
 * function prototypes
 */ 
static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
static WAVEHDR* allocateBlocks(int size, int count);
static void freeBlocks(WAVEHDR* blockArray);
static void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);
/*
 * module level variables
 */
static CRITICAL_SECTION waveCriticalSection;
static WAVEHDR* waveBlocks;
static volatile int waveFreeBlockCount;
static int waveCurrentBlock;

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

int main(int argc, char* argv[])
{
	HWAVEOUT hWaveOut; /* device handle */
	HANDLE hFile;/* file handle */
	WAVEFORMATEX wfx; /* look this up in your documentation */
	char buffer[1024]; /* intermediate buffer for reading */
	int i;
	
	const int buf_size = SAMPLE_RATE*4;

	short int wave_buffer[buf_size];

	init_song(buf_size);

	int curr_note = 0;
	
	for (int i = 0; i < buf_size; i++) {
		if (i >= bass[curr_note+1].first) curr_note++;
		//wave_buffer[i] = 0.5*square_wave(notes[curr_note].second,i) + 0.5*square_wave(notes2[curr_note].second,i);
		/*wave_buffer[i] = (inst_trance(bass[curr_note].second, 4, i) * (gate(2,i) ? gate(8,i) : 1) +
			              inst_bass(bass[curr_note].second, i) +
						  white_wave(0,i) * (gate(8,i) & gate(16,i))) / 2.5;*/
		float freq = get_freq(bass[curr_note].second,0,4);
		int t = i%SAMPLE_RATE+10000;
		wave_buffer[i] = sine_wave(freq+(sine_wave(3*freq+(sine_wave(1.25*freq, t)/float(MAX_AMP)), t)/float(MAX_AMP)), t);
	}

	/*for (int i = 0; i < buf_size; i++) {
		if (i < 2) continue;
		else wave_buffer[i] = low_pass(wave_buffer[i], wave_buffer[i-1], ((i < 88200) ? sqrtf(abs(88200-i)/88200.0) : powf((i-88200)/88200.0,2))) + 0.01;
	}*/

	write_wav("temp.wav", buf_size, wave_buffer, SAMPLE_RATE);
	
	/*
	 * initialise the module variables
	 */ 
	waveBlocks = allocateBlocks(BLOCK_SIZE, BLOCK_COUNT);
	waveFreeBlockCount = BLOCK_COUNT;
	waveCurrentBlock= 0;
	InitializeCriticalSection(&waveCriticalSection);
	/*
	 * try and open the file
	 */ 
	if((hFile = CreateFile(L"temp.wav", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "%s: unable to open file '%s': %d\n", argv[0], argv[1], GetLastError());
		ExitProcess(1);
	}
	/*
	 * set up the WAVEFORMATEX structure.
	 */
	wfx.nSamplesPerSec = SAMPLE_RATE; /* sample rate */
	wfx.wBitsPerSample = 16; /* sample size */
	wfx.nChannels= CHANNELS; /* channels*/
	wfx.cbSize = 0; /* size of _extra_ info */
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
	/*
	 * try to open the default wave device. WAVE_MAPPER is
	 * a constant defined in mmsystem.h, it always points to the
	 * default wave device on the system (some people have 2 or
	 * more sound cards).
	 */
	if(waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc, (DWORD_PTR)&waveFreeBlockCount, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		fprintf(stderr, "%s: unable to open wave mapper device\n", argv[0]);
		ExitProcess(1);
	}
	/*
	 * playback loop
	 */
	while(1) {
		DWORD readBytes;
		if(!ReadFile(hFile, buffer, sizeof(buffer), &readBytes, NULL))
			break;
		if(readBytes == 0)
			break;
		if(readBytes < sizeof(buffer)) {
			printf("at end of buffer\n");
			memset(buffer + readBytes, 0, sizeof(buffer) - readBytes);
			printf("after memcpy\n");
		}
		writeAudio(hWaveOut, buffer, sizeof(buffer));
	}
	/*
	 * wait for all blocks to complete
	 */
	while(waveFreeBlockCount < BLOCK_COUNT)
		Sleep(10);
	/*
	 * unprepare any blocks that are still prepared
	 */
	for(i = 0; i < waveFreeBlockCount; i++) 
		if(waveBlocks[i].dwFlags & WHDR_PREPARED)
			waveOutUnprepareHeader(hWaveOut, &waveBlocks[i], sizeof(WAVEHDR));
	DeleteCriticalSection(&waveCriticalSection);
	freeBlocks(waveBlocks);
	waveOutClose(hWaveOut);
	CloseHandle(hFile);
	return 0;
}

static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1,DWORD dwParam2)
{
	/*
	 * pointer to free block counter
	 */
	int* freeBlockCounter = (int*)dwInstance;
	/*
	 * ignore calls that occur due to openining and closing the
	 * device.
	 */
	if(uMsg != WOM_DONE)
	return;
	EnterCriticalSection(&waveCriticalSection);
	(*freeBlockCounter)++;
	LeaveCriticalSection(&waveCriticalSection);
}

WAVEHDR* allocateBlocks(int size, int count)
{
	unsigned char* buffer;
	int i;
	WAVEHDR* blocks;
	DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;
	/*
	 * allocate memory for the entire set in one go
	 */
	if((buffer = (unsigned char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalBufferSize)) == NULL) {
		fprintf(stderr, "Memory allocation error\n");
		ExitProcess(1);
	}
	/*
	 * and set up the pointers to each bit
	 */
	blocks = (WAVEHDR*)buffer;
	buffer += sizeof(WAVEHDR) * count;
	for(i = 0; i < count; i++) {
		blocks[i].dwBufferLength = size;
		blocks[i].lpData = (LPSTR)buffer;
		buffer += size;
	}
	return blocks;
}

void freeBlocks(WAVEHDR* blockArray)
{
	/* 
	 * and this is why allocateBlocks works the way it does
	 */ 
	HeapFree(GetProcessHeap(), 0, blockArray);
}

void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size)
{
	WAVEHDR* current;
	int remain;
	current = &waveBlocks[waveCurrentBlock];
	while(size > 0) {
	/* 
	 * first make sure the header we're going to use is unprepared
	 */
	if(current->dwFlags & WHDR_PREPARED) 
		waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));
	if(size < (int)(BLOCK_SIZE - current->dwUser)) {
		memcpy(current->lpData + current->dwUser, data, size);
		current->dwUser += size;
		break;
	}
	remain = BLOCK_SIZE - current->dwUser;
	memcpy(current->lpData + current->dwUser, data, remain);
	size -= remain;
	data += remain;
	current->dwBufferLength = BLOCK_SIZE;
	waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));
	EnterCriticalSection(&waveCriticalSection);
	waveFreeBlockCount--;
	LeaveCriticalSection(&waveCriticalSection);
	/*
	 * wait for a block to become free
	 */
	while(!waveFreeBlockCount)
		Sleep(10);
	/*
	 * point to the next block
	 */
	waveCurrentBlock++;
	waveCurrentBlock %= BLOCK_COUNT;
	current = &waveBlocks[waveCurrentBlock];
	current->dwUser = 0;
	}
}