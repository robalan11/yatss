#include "definitions.h"
#include "utilities.h"

std::pair<int,float> notes[6];
std::pair<int,float> notes2[6];
std::pair<int,char> bass[6];

void init_song(int buf_size)
{
	notes[0] = std::make_pair(0,get_freq('E',0,4)); notes[1] = std::make_pair(22050,get_freq('G',0,4));
	notes[2] = std::make_pair(44100,get_freq('F',0,4)); notes[3] = std::make_pair(66150,get_freq('D',0,4));
	notes[4] = std::make_pair(88200,get_freq('E',0,4)); notes[5] = std::make_pair(buf_size,0.0f);

	notes2[0] = std::make_pair(0,get_freq('C',0,3)); notes2[1] = std::make_pair(22050,get_freq('G',0,3));
	notes2[2] = std::make_pair(44100,get_freq('G',0,2)); notes2[3] = std::make_pair(66150,get_freq('G',0,3));
	notes2[4] = std::make_pair(88200,get_freq('C',0,3)); notes2[5] = std::make_pair(buf_size,0.0f);

	bass[0] = std::make_pair(0, 'F'); bass[1] = std::make_pair(SAMPLE_RATE, 'G');
	bass[2] = std::make_pair(SAMPLE_RATE*2, 'C'); bass[3] = std::make_pair(buf_size, 'F');
}