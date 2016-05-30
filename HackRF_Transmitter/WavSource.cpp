#include "WavSource.h"

#define DEBUG 1

WavSource::WavSource(const char *filename, int inputSamples)
:_inputSamples(inputSamples)
{
	pcmHeader = wav_open_read(filename, 0);
	audioDataPos = ftell(pcmHeader->f);
	_buf = (float *)malloc(inputSamples*sizeof(float)* pcmHeader->channels);
}


WavSource::~WavSource()
{
	free(_buf);
	wav_close(pcmHeader);
}

int WavSource::readData(){
	return wav_read_float32(pcmHeader, _buf, _inputSamples * pcmHeader->channels, NULL);
}

void WavSource::reset(){
	fseek(pcmHeader->f, audioDataPos, SEEK_SET);
}


