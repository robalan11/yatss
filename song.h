#include "definitions.h"

class Note{
public:
	Note(){}

	void init(int _start, char _note, int _accidental, int _octave)
	{
		start = _start;
		note = _note;
		accidental = _accidental;
		octave = _octave;
	}

	Note operator=(const Note& other)
	{
		start = other.start;
		note = other.note;
		accidental = other.accidental;
		octave = other.octave;

		return *this;
	}

	int start;
	char note;
	int accidental;
	int octave;
private:
};

Note notes[10];
Note notes2[6];
Note bass[6];

void init_song(int buf_size)
{
	notes[0].init(0,'E',0,4); notes[1].init(int(0.5*SR),'G',0,4);
	notes[2].init(SR,'F',0,4); notes[3].init(int(1.5*SR),'D',0,4);
	notes[4].init(2*SR,'E',0,4); notes[5].init(int(2.25*SR),'C',0,4);
	notes[6].init(int(2.5*SR),'D',0,4); notes[7].init(int(2.75*SR),'B',0,3);
	notes[8].init(3*SR,'C',0,4); notes[9].init(buf_size,'C',0,4);

/*	notes2[0] = std::make_pair(0,get_freq('C',0,3)); notes2[1] = std::make_pair(22050,get_freq('G',0,3));
	notes2[2] = std::make_pair(44100,get_freq('G',0,2)); notes2[3] = std::make_pair(66150,get_freq('G',0,3));
	notes2[4] = std::make_pair(88200,get_freq('C',0,3)); notes2[5] = std::make_pair(buf_size,0.0f);

	bass[0] = std::make_pair(0, 'F'); bass[1] = std::make_pair(SAMPLE_RATE, 'G');
	bass[2] = std::make_pair(SAMPLE_RATE*2, 'C'); bass[3] = std::make_pair(buf_size, 'F');*/
}