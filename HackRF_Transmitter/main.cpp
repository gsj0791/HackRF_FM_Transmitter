#include <Windows.h>
#include "WavSource.h"
#include "FMModulator.h"
#include "HackRFDevice.h"
#include "write_wav.h"

#define HACKRF_SAMPLE 2000000
#define SAMPLE_COUNT 2048

int main(int argc, char *argv[])
{
	WavSource *wav = new WavSource("f:\\notouch.wav", SAMPLE_COUNT);

	uint32_t avSamplesPerSecByte = wav->getSampleByte() * wav->getChannels() * wav->getSampleRate();
	//176400
	int PerTimeStamp = 1000 / (avSamplesPerSecByte / (SAMPLE_COUNT * wav->getChannels()));
	int readTotalAudioByte = 0;
	int PerSamplesPerSecByte = SAMPLE_COUNT * wav->getSampleByte();
	int m_read_buf_size;
	int nTimeStamp = 0;
	int fTimeStamp = 0;
	double audioTimeTick = GetTickCount();

	int sample_rate = wav->getSampleRate()*1.0 / SAMPLE_COUNT*BUF_LEN;

	FMModulator *mod = new FMModulator(90, 0, sample_rate);
	HackRFDevice *device = new HackRFDevice();

	if (!device->Open(mod)) {
		return 0;
	}

	device->SetSampleRate(sample_rate);
	device->SetFrequency(2600000000);
	device->SetGain(40);
	device->SetAMP(0);
	device->StartTx();

	//WAV_Writer wavFile;
	//Audio_WAV_OpenWriter(&wavFile, "f:\\test11111111111.wav", wav->getSampleRate(), wav->getSampleByte());
	
	while (1)
	{
		nTimeStamp = GetTickCount() - audioTimeTick;
		fTimeStamp = ((readTotalAudioByte + SAMPLE_COUNT)*2 / PerSamplesPerSecByte) * PerTimeStamp;

		if (nTimeStamp > fTimeStamp){
			m_read_buf_size = wav->readData();			
			//fwrite(wav->getData(), wav->getChannels(), SAMPLE_COUNT * sizeof(float), wavFile.fid);
			//wavFile.dataSize += wav->getChannels()* SAMPLE_COUNT * sizeof(float);
			
			mod->Start(wav);
			readTotalAudioByte += m_read_buf_size;
			if (!m_read_buf_size){
				audioTimeTick = GetTickCount();
				readTotalAudioByte = 0;
				wav->reset();
			}
		}
		Sleep(5);
	}

	//Audio_WAV_CloseWriter(&wavFile);
	device->Close();
	delete(mod);
	return 0;
}