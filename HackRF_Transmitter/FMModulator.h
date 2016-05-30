#ifndef _FMMODULATOR_H
#define _FMMODULATOR_H

#include <mutex>
#include "IHackRFData.h"
#include "WavSource.h"


#define BUF_LEN 262144         //hackrf tx buf

class FMModulator:public IHackRFData
{
private:
	std::mutex m_mutex;
	int count;
	int8_t ** _buf_ptr;
	bool inited;
	int tail;
	int head;
	float gain;
	uint32_t mode;
	float * audio_buf;
	float * new_audio_buf;
	float * IQ_buf;
	uint32_t m_sample_rate;
	size_t m_sample_count;
	uint32_t hackrf_sample;
	double fm_phase;
	double fm_deviation;
	float last_in_samples[4];

private:
	void interpolation(float * in_buf, uint32_t in_samples, float * out_buf, uint32_t out_samples, float last_in_samples[4]);
	void modulation(float * input, float * output, uint32_t mode);
	void work(float *input_items, uint32_t len);

public:
	FMModulator(float _gain, uint32_t _mode, uint32_t _sample);
	~FMModulator();

	int onData(int8_t* buffer, uint32_t length);
	void Start(WavSource *source);
};

#endif

