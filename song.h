#include "definitions.h"

struct note {
	int start;
	char note;
	int accidental;
	int octave;
};

struct note mknote(int start, char note, int accidental, int octave)
{
	struct note n = {start, note, accidental, octave};
	return n;
}

struct song {
	int channels;
	short **note_bufs;
	struct note notes[1];
};

struct song *new_song(int channels, int len)
{
	int i;
	struct song *s = (struct song *)malloc(sizeof(struct song) + len * sizeof(struct note));

	s->channels = channels;
	s->note_bufs = (short **)malloc(channels * sizeof(short *));
	for (i = 0; i < channels; i++)
		s->note_bufs[i] = (short *)malloc(BUFFER_SIZE * sizeof(short));
	s->notes[len] = mknote(0,'\0',0,0);

	printf("ch:%i, nb:%i, nb0:%i, nb1:%i\n", s->channels, s->note_bufs, s->note_bufs[0], s->note_bufs[1]);
	return s;
}

struct song *song1;
//struct note notes[10];
struct note notes2[6];
struct note notes3[8];
struct note bass[6];

void init_song(int song_length)
{
	song1 = new_song(6, 24);
	song1->notes[0] = mknote(0,'E',0,4);
	song1->notes[1] = mknote(int(0.5*SR),'G',0,4);
	song1->notes[2] = mknote(SR,'F',0,4);
	song1->notes[3] = mknote(int(1.5*SR),'D',0,4);
	song1->notes[4] = mknote(2*SR,'E',0,4);
	song1->notes[5] = mknote(int(2.25*SR),'C',0,4);
	song1->notes[6] = mknote(int(2.5*SR),'D',0,4);
	song1->notes[7] = mknote(int(2.75*SR),'B',0,3);
	song1->notes[8] = mknote(3*SR,'C',0,4);
	song1->notes[9] = mknote(song_length,'C',0,4);

	song1->notes[10] = mknote(0,'C',0,3);
	song1->notes[11] = mknote(SR,'G',0,2);
	song1->notes[12] = mknote(2*SR,'C',0,3);
	song1->notes[13] = mknote(int(2.5*SR),'G',0,2);
	song1->notes[14] = mknote(3*SR,'C',0,2);
	song1->notes[15] = mknote(song_length,'C',0,2);

	song1->notes[16] = mknote(0,'E',0,5);
	song1->notes[17] = mknote(int(0.5*SR),'C',0,5);
	song1->notes[18] = mknote(SR,'D',0,5);
	song1->notes[19] = mknote(int(1.5*SR),'F',0,5);
	song1->notes[20] = mknote(2*SR,'E',0,5);
	song1->notes[21] = mknote(int(2.5*SR),'D',0,5);
	song1->notes[22] = mknote(3*SR,'E',0,5);
	song1->notes[23] = mknote(song_length,'E',0,5);

/*	bass[0] = std::make_pair(0, 'F'); bass[1] = std::make_pair(SAMPLE_RATE, 'G');
	bass[2] = std::make_pair(SAMPLE_RATE*2, 'C'); bass[3] = std::make_pair(buf_size, 'F');*/
}
