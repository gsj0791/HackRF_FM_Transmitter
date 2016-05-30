#ifndef _HACKRFDEIVCE_H
#define _HACKRFDEIVCE_H
#include "IHackRFData.h"
#include "hackrf.h"
#include <stdint.h>
#include <stdio.h>

class HackRFDevice
{
private:
	hackrf_device *_dev;
	IHackRFData *mHandler;
	bool running;
	uint32_t hackrf_sample = 2000000;
	
public:
	HackRFDevice();
	~HackRFDevice();

	int HackRFCallback(int8_t* buffer, uint32_t length);

	bool Open(IHackRFData *handler);
	void SetFrequency(uint64_t freg);
	void SetGain(float gain);
	void SetAMP(bool enableamp);
	void SetSampleRate(uint32_t sample_rate);
	bool StartTx();
	void Close();
};

#endif // !
