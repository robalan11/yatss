#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <map>
#include <time.h>

#include "definitions.h"
#include "filter.h"
#include "instrument.h"
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

void fill_buffer(short* out_buffer, int t)
{
	short in_buffer[BUFFER_SIZE];

	int curr_note = 0;
	
	for (int i = 0; i < BUFFER_SIZE; i++) {
		if (i+t > SONG_LENGTH) break;
		if (i+t >= notes[curr_note+1].start) curr_note++;
		/*in_buffer[i] = (inst_trance(notes[curr_note].note, 4, i+t) * (gate(2,i+t) ? gate(8,i+t) : 1) +
			              inst_bass(notes[curr_note].note, i+t) +
						  white_wave(0,i+t) * (gate(8,i+t) & gate(16,i+t))) / 2.5;*/
		//in_buffer[i] = inst_fm_bass(notes[curr_note].note, notes[curr_note].octave, i-bass[curr_note].start);
		//in_buffer[i] = inst_pluck(notes[curr_note].note, notes[curr_note].octave, i, i-notes[curr_note].start);
		in_buffer[i] = inst_fire(i+t, in_buffer);
		//in_buffer[i] = inst_ks_pluck(notes[curr_note].note, notes[curr_note].octave, i+t, i+t-notes[curr_note].start, in_buffer);
		//in_buffer[i] = inst_crickets(i+t, (i+t)%(SAMPLE_RATE/2));
	}

	LowPassFilter LPF(30000,1.0f/sqrt(2.0f));

	for (int i = 0; i < BUFFER_SIZE; i++) {
		out_buffer[i] = in_buffer[i];//clamp(LPF.filter(in_buffer[i]));
	}
}

int main(int argc, char* argv[])
{
	HWAVEOUT hWaveOut; /* device handle */
	WAVEFORMATEX wfx; /* look this up in your documentation */
	char buffer[1024]; /* intermediate buffer for reading */
	int i;

	short *out_buffer = new short[BUFFER_SIZE];

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

	init_scale();
	init_song(SONG_LENGTH);
	
	srand(time(NULL));
	
	fill_buffer(out_buffer, 0);

	/*
	 * initialise the module variables
	 */ 
	waveBlocks = allocateBlocks(BLOCK_SIZE, BLOCK_COUNT);
	waveFreeBlockCount = BLOCK_COUNT;
	waveCurrentBlock= 0;
	InitializeCriticalSection(&waveCriticalSection);
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
	int buf_pos = 0;
	int cycles = 0;
	short readBytes = 0;
	int buf_rem;
	while(1) {
		buf_rem = 2*(BUFFER_SIZE-buf_pos);
		if(buf_rem <= 1024)
		{
			cycles++;
			if (BUFFER_SIZE*cycles > SONG_LENGTH) break; // song's over!
			memcpy(buffer, out_buffer+buf_pos, buf_rem);
			fill_buffer(out_buffer, BUFFER_SIZE*cycles);
			readBytes = 1024-buf_rem;
			memcpy(buffer+buf_rem, out_buffer, readBytes);
			buf_pos = 0;
		} else {
			readBytes = 1024;
			memcpy(buffer, out_buffer+buf_pos, readBytes);
		}
		buf_pos += readBytes/2; // divide by 2 to convert from bytes to shorts

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