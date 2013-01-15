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

LowPassFilter LPF(10000,1.0f/sqrt(2.0f));
short in_buffer[BUFFER_SIZE];
short in_buffer2[BUFFER_SIZE];
short in_buffer3[BUFFER_SIZE];

void sample(short *buf, struct song *s, const int samp, const int t)
{
	int i;
    int used_channels;
	short smpl;

	for (i = 0, used_channels = 0; (s->notes[i].note != '\0') && (used_channels < s->channels); i++) {
		if ((t > s->notes[i].start) && (t < (s->notes[i].start + SR))) {
			smpl = inst_ks_pluck(s->notes[i].note, s->notes[i].octave, t, t - s->notes[i].start, s->note_bufs[used_channels]);
			s->note_bufs[used_channels][samp] = smpl;
			used_channels++;
		}
	}
	buf[samp] = 0;
	for (i = 0; i < used_channels; i++) {
		buf[samp] += s->note_bufs[i][samp] / s->channels * 2;
	}
}

void fill_buffer(short* out_buffer, int t)
{
printf("filling buffer, t=%i\n", t);
for(int i = 0; i < song1->channels; i++)
	memset(song1->note_bufs[i], 0, BUFFER_SIZE * sizeof(short));
	for (int i = 0; i < BUFFER_SIZE; i++) {
/*
		static int curr_note = 0;
		static int curr_note2 = 0;
		static int curr_note3 = 0;
		if (i+t > SONG_LENGTH) break;
		if (i+t >= notes[curr_note+1].start) curr_note++;
		if (i+t >= notes2[curr_note2+1].start) curr_note2++;
		if (i+t >= notes3[curr_note3+1].start) curr_note3++;
*/
		/*in_buffer[i] = (inst_trance(notes[curr_note].note, 4, i+t) * (gate(2,i+t) ? gate(8,i+t) : 1) +
			              inst_bass(notes[curr_note].note, i+t) +
						  white_wave(0,i+t) * (gate(8,i+t) & gate(16,i+t))) / 2.5;*/
		//in_buffer[i] = inst_fm_bass(notes[curr_note].note, notes[curr_note].octave, i-bass[curr_note].start);
		//in_buffer[i] = inst_pluck(notes[curr_note].note, notes[curr_note].octave, i, i-notes[curr_note].start);
		//in_buffer[i] = inst_fire(i+t);
		//in_buffer[i] = inst_ks_pluck(notes[curr_note].note, notes[curr_note].octave, i+t, i+t-notes[curr_note].start, in_buffer);
		sample(in_buffer, song1, i, i + t);
		//in_buffer2[i] = inst_ks_pluck(notes2[curr_note2].note, notes2[curr_note2].octave, i+t, i+t-notes2[curr_note2].start, in_buffer2);
		//in_buffer3[i] = inst_ks_pluck(notes3[curr_note3].note, notes3[curr_note3].octave, i+t, i+t-notes3[curr_note3].start, in_buffer3);
		//in_buffer[i] = inst_crickets(i+t, (i+t)%(SAMPLE_RATE/2));
		//in_buffer[i] = inst_flute(notes[curr_note].note, notes[curr_note].octave, i+t, i+t-notes[curr_note].start);
		/*Env_Point values[4];
		values[0].init(0,1.0); values[1].init(2000,1.0); values[2].init(0.7*SR, 0.1); values[3].init(0.75*SR, 0.0);
		in_buffer[i] = inst_bell(notes[curr_note].note, notes[curr_note].octave, i+t, i+t-notes[curr_note].start, values, 4);*/
	}

	for (int i = 0; i < BUFFER_SIZE; i++) {
		out_buffer[i] = clamp(LPF.filter(0.9*in_buffer[i] + 0.6*in_buffer2[i] + in_buffer3[i]));
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
			/*char filename[6];
			sprintf(filename, "%d.wav", cycles);
			write_wav(filename, BUFFER_SIZE, out_buffer, SAMPLE_RATE);*/
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
