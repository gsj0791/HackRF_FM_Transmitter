#include "FMModulator.h"


#define BUF_NUM  256
#define BYTES_PER_SAMPLE  2
#define M_PI 3.14159265358979323846


FMModulator::FMModulator(float _gain, uint32_t _mode, uint32_t _sample)
:gain(_gain / (float)100.0), mode(_mode), inited(false), hackrf_sample(_sample)
{
	audio_buf = NULL;
	new_audio_buf = NULL;
	IQ_buf = NULL;
	memset(last_in_samples, 0, sizeof(last_in_samples));
	
	count = tail = head = 0;

	_buf_ptr = (int8_t **)malloc(BUF_NUM * sizeof(int8_t *));
	if (_buf_ptr) {
		for (unsigned int i = 0; i < BUF_NUM; ++i) {
			_buf_ptr[i] = (int8_t *)malloc(BUF_LEN*sizeof(int8_t));
		}
	}
}


FMModulator::~FMModulator()
{
	if (_buf_ptr) {
		for (unsigned int i = 0; i < BUF_NUM; ++i) {
			if (_buf_ptr[i])
				free(_buf_ptr[i]);
		}
		free(_buf_ptr);
	}
	
	delete IQ_buf;
	delete new_audio_buf;
	delete audio_buf;
}

void FMModulator::interpolation(float * in_buf, uint32_t in_samples, float * out_buf, uint32_t out_samples, float last_in_samples[4]) {
	uint32_t i;		/* Input buffer index + 1. */
	uint32_t j = 0;	/* Output buffer index. */
	float pos;		/* Position relative to the input buffer
					* + 1.0. */

	/* We always "stay one sample behind", so what would be our first sample
	* should be the last one wrote by the previous call. */
	pos = (float)in_samples / (float)out_samples;
	while (pos < 1.0)
	{
		out_buf[j] = last_in_samples[3] + (in_buf[0] - last_in_samples[3]) * pos;
		j++;
		pos = (float)(j + 1)* (float)in_samples / (float)out_samples;
	}

	/* Interpolation cycle. */
	i = (uint32_t)pos;
	while (j < (out_samples - 1))
	{

		out_buf[j] = in_buf[i - 1] + (in_buf[i] - in_buf[i - 1]) * (pos - (float)i);
		j++;
		pos = (float)(j + 1)* (float)in_samples / (float)out_samples;
		i = (uint32_t)pos;
	}

	/* The last sample is always the same in input and output buffers. */
	out_buf[j] = in_buf[in_samples - 1];

	/* Copy last samples to last_in_samples (reusing i and j). */
	for (i = in_samples - 4, j = 0; j < 4; i++, j++)
		last_in_samples[j] = in_buf[i];
}

void FMModulator::modulation(float * input, float * output, uint32_t mode) {
	if (mode == 0) {
		fm_deviation = 2.0 * M_PI * 75.0e3 / hackrf_sample; // 75 kHz max deviation WBFM
	}
	else if (mode == 1)
	{
		fm_deviation = 2.0 * M_PI * 5.0e3 / hackrf_sample; // 5 kHz max deviation NBFM
	}

	//AM mode
	if (mode == 2) {
		for (uint32_t i = 0; i < BUF_LEN; i++) {
			double	audio_amp = input[i] * gain;

			if (fabs(audio_amp) > 1.0) {
				audio_amp = (audio_amp > 0.0) ? 1.0 : -1.0;
			}

			IQ_buf[i * BYTES_PER_SAMPLE] = (float)audio_amp;
			IQ_buf[i * BYTES_PER_SAMPLE + 1] = 0;
		}
	}
	//FM mode
	else {

		for (uint32_t i = 0; i < BUF_LEN; i++) {

			double	audio_amp = input[i] * gain;

			if (fabs(audio_amp) > 1.0) {
				audio_amp = (audio_amp > 0.0) ? 1.0 : -1.0;
			}
			fm_phase += fm_deviation * audio_amp;
			while (fm_phase > (float)(M_PI))
				fm_phase -= (float)(2.0 * M_PI);
			while (fm_phase < (float)(-M_PI))
				fm_phase += (float)(2.0 * M_PI);

			output[i * BYTES_PER_SAMPLE] = (float)sin(fm_phase);
			output[i * BYTES_PER_SAMPLE + 1] = (float)cos(fm_phase);
		}
	}
}

void FMModulator::work(float *input_items, uint32_t len) {

	m_mutex.lock();
	int8_t * buf = (int8_t *)_buf_ptr[head];
	for (uint32_t i = 0; i < BUF_LEN; i++) {
		buf[i] = (int8_t)(input_items[i] * 127.0);
	}
	head = (head + 1) % BUF_NUM;
	count++;
	m_mutex.unlock();

}
int FMModulator::onData(int8_t* buffer, uint32_t length)
{
	m_mutex.lock();

	if (count == 0) {
		memset(buffer, 0, length);
	}
	else {
		memcpy(buffer, _buf_ptr[tail], length);
		tail = (tail + 1) % BUF_NUM;
		count--;
	}
	m_mutex.unlock();

	return 0;
}

void FMModulator::Start(WavSource *source)
{
	int nch = source->getChannels();
	m_sample_count = source->getSampleCount();
	//hackrf_sample = m_sample_rate*1.0 / m_sample_count*BUF_LEN;

	float * source_audio_buf = source->getData();

	if (!inited) {
		audio_buf = new float[m_sample_count]();
		new_audio_buf = new float[BUF_LEN]();
		IQ_buf = new float[BUF_LEN * BYTES_PER_SAMPLE]();
		inited = true;
	}

	if (nch == 1) {
		for (uint32_t i = 0; i < m_sample_count; i++) {

			audio_buf[i] = source_audio_buf[i];
		}
	}
	else if (nch == 2) {
		for (uint32_t i = 0; i < m_sample_count; i++) {

			audio_buf[i] = (source_audio_buf[i * 2] + source_audio_buf[i * 2 + 1]) / (float)2.0;
		}
	}

	interpolation(audio_buf, m_sample_count, new_audio_buf, BUF_LEN, last_in_samples);

	modulation(new_audio_buf, IQ_buf, mode);

	for (uint32_t i = 0; i < (BUF_LEN * BYTES_PER_SAMPLE); i += BUF_LEN) {

		work(IQ_buf + i, BUF_LEN);
	}

	//AM mode
}
