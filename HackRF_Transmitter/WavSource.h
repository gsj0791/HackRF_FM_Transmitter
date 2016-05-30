#ifndef _WAVSOURCE_H
#define _WAVSOURCE_H

#include "input.h"

class WavSource
{
private:
	pcmfile_t *pcmHeader;
	float *_buf;
	int _inputSamples;
	int audioDataPos = 0;

public:
	WavSource(const char *filename, int inputSamples);
	~WavSource();

	int readData();
	int getChannels(){ return pcmHeader->channels; };
	int getSampleRate(){ return pcmHeader->samplerate; }
	int getSampleByte(){ return pcmHeader->samplebytes; }
	int getSamples(){ return pcmHeader->samples; }
	int getSampleCount(){ return _inputSamples; }
	float *getData() { return _buf; };
	void reset();
};

#endif // !_WAVSOURCE_H

