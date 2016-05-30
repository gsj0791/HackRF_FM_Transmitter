#include "HackRFDevice.h"


int _hackrf_tx_callback(hackrf_transfer *transfer) {
	HackRFDevice *obj = (HackRFDevice *)transfer->tx_ctx;
	return obj->HackRFCallback((int8_t *)transfer->buffer, transfer->valid_length);
}

HackRFDevice::HackRFDevice()
	:_dev(NULL), running(false)
{
}


HackRFDevice::~HackRFDevice()
{
	if (_dev) {
		this->Close();
	}
}

bool HackRFDevice::Open(IHackRFData *handler){
	mHandler = handler;
	hackrf_init();

	int ret = hackrf_open(&_dev);
	if (ret != HACKRF_SUCCESS) {
		printf("Failed to open HackRF device");
		hackrf_close(_dev);
		return false;
	}
	return true;
}

int HackRFDevice::HackRFCallback(int8_t* buffer, uint32_t length)
{
	return mHandler->onData(buffer, length);
}

bool HackRFDevice::StartTx()
{
	running = true;
	int ret = hackrf_start_tx(_dev, _hackrf_tx_callback, (void *)this);
	if (ret != HACKRF_SUCCESS) {
		printf("Failed to start TX streaming");
		hackrf_close(_dev);
		running = false;
	}

	return running;
}

void HackRFDevice::SetFrequency(uint64_t freg)
{
	hackrf_set_freq(_dev, freg);
}

void HackRFDevice::SetGain(float gain)
{
	hackrf_set_txvga_gain(_dev, gain);
}

void HackRFDevice::SetAMP(bool enableamp)
{
	hackrf_set_amp_enable(_dev, enableamp);
}

void HackRFDevice::SetSampleRate(uint32_t sample_rate)
{
	hackrf_set_sample_rate(_dev, sample_rate);
	hackrf_set_baseband_filter_bandwidth(_dev, 1750000);
}

void HackRFDevice::Close() {
	running = false;
	hackrf_stop_tx(_dev);
	hackrf_close(_dev);
	_dev = NULL;
}
