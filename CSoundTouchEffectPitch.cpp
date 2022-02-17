#include "stdafx.h"
#include "CSoundTouchEffectPitch.h"

//�����ֽ���
#ifdef BYTE_ORDER
// In gcc compiler detect the byte order automatically
#if BYTE_ORDER == BIG_ENDIAN
	// big-endian platform.
#define _BIG_ENDIAN_
#endif
#endif

#ifdef _BIG_ENDIAN_
	// big-endian CPU, swap bytes in 16 & 32 bit words

	// helper-function to swap byte-order of 32bit integer
static inline int _swap32(int &dwData)
{
	dwData = ((dwData >> 24) & 0x000000FF) |
		((dwData >> 8) & 0x0000FF00) |
		((dwData << 8) & 0x00FF0000) |
		((dwData << 24) & 0xFF000000);
	return dwData;
}

// helper-function to swap byte-order of 16bit integer
static inline short _swap16(short &wData)
{
	wData = ((wData >> 8) & 0x00FF) |
		((wData << 8) & 0xFF00);
	return wData;
}

// helper-function to swap byte-order of buffer of 16bit integers
static inline void _swap16Buffer(short *pData, int numWords)
{
	int i;

	for (i = 0; i < numWords; i++)
	{
		pData[i] = _swap16(pData[i]);
	}
}

#else   // BIG_ENDIAN
	// little-endian CPU, WAV file is ok as such

	// dummy helper-function
static inline int _swap32(int &dwData)
{
	// do nothing
	return dwData;
}

// dummy helper-function
static inline short _swap16(short &wData)
{
	// do nothing
	return wData;
}

// dummy helper-function
static inline void _swap16Buffer(short *pData, int numBytes)
{
	// do nothing
}
#endif  // BIG_ENDIAN

/**
 * ��ʼ������
 * ����:
 * channelsSrc��������
 * SampleRateSrc��������
 */
CSoundTouchEffectPitch::CSoundTouchEffectPitch(int channelsSrc, int SampleRateSrc)
	:m_pSoundTouch{ std::make_unique<soundtouch::SoundTouch>() }, m_pSoundTouchBuff{ nullptr }, m_iChannels(channelsSrc)
{
	m_pSoundTouch->setChannels(channelsSrc);//����ͨ����
	m_pSoundTouch->setSampleRate(SampleRateSrc);//���ò�����

	m_pSoundTouch->setTempoChange(0);
	m_pSoundTouch->setRateChange(0);

	m_pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, 0);
	m_pSoundTouch->setSetting(SETTING_USE_AA_FILTER, 1);//���ñ�����еĿ�����˲���
}

CSoundTouchEffectPitch::~CSoundTouchEffectPitch()
{
	if (this->m_pSoundTouchBuff)
	{
		delete[] this->m_pSoundTouchBuff;
		this->m_pSoundTouchBuff = nullptr;
	}
	
}

/**
 * ���ݴ���
 * ����:
 * data�������������
 * perSamplesByte��ÿ��������ռ�ֽ���
 * sampleCounts��������
 */
void CSoundTouchEffectPitch::process(uint8_t** data, int perSamplesByte, int sampleCounts)
{
	this->m_pSoundTouchBuff = (soundtouch::SAMPLETYPE*) realloc(this->m_pSoundTouchBuff, sizeof(soundtouch::SAMPLETYPE) * m_iChannels * sampleCounts);
	toSounTouchByte(*data, perSamplesByte, sampleCounts * m_iChannels);
	this->m_pSoundTouch->putSamples(this->m_pSoundTouchBuff, sampleCounts);
	memset(this->m_pSoundTouchBuff, 0, sizeof(soundtouch::SAMPLETYPE) * m_iChannels * sampleCounts);
	delete[](*data);
	(*data) = NULL;
}

/**
 * ��ȡ����õ�����
 * ����:
 * perSamplesByte��ÿ��������ռ�ֽ���
 * sampleCounts��������
 *
 * ����ֵ�����������أ�SoundTouch��Ҫ����һ�����������Ż᷵������
 */
uint8_t* CSoundTouchEffectPitch::reciveSamples(int perSamplesByte, int sampleCounts)
{
	int nSamples = 0;
	uint8_t* data = nullptr;
	//��SoundTouch��ȡ������õ�����
	nSamples = this->m_pSoundTouch->numSamples();
	if (nSamples >= sampleCounts) {
		int num = this->m_pSoundTouch->receiveSamples(this->m_pSoundTouchBuff, sampleCounts);
		int size = sampleCounts * this->m_iChannels * perSamplesByte;
		data = new uint8_t[size]{ 0 };
		this->toDataBtye(data, perSamplesByte, sampleCounts * this->m_iChannels);
	}
	return data;
}

/**
 * ��ȡ�߳̽���ʱ��������
 * ����:
 * perSamplesByte��ÿ��������ռ�ֽ���
 * sampleCounts��������
 *
 * ����ֵ�����������أ�SoundTouch��Ҫ����һ�����������Ż᷵������
 *
 * ��ע:��ʼ��ʱ������ֵΪ����ˢ��������
 */
uint8_t* CSoundTouchEffectPitch::reciveSamplesEnd(int perSamplesByte, int sampleCounts)
{
	if(this->m_pSoundTouch->numUnprocessedSamples() > 0)
		this->m_pSoundTouch->flush();//pitch��ֵΪ��ʱ��flush����������ݣ�numSamples()����0;

	uint8_t* data = nullptr;
	int numSamples = m_pSoundTouch->numSamples();
	if(numSamples > sampleCounts)
	{
		//��SoundTouch��ȡ������õ�����
		this->m_pSoundTouch->receiveSamples(this->m_pSoundTouchBuff, sampleCounts);
		int size = sampleCounts * this->m_iChannels * perSamplesByte;
		data = new uint8_t[size]{ 0 };
		this->toDataBtye(data, perSamplesByte, sampleCounts * this->m_iChannels);
	} 
	return data;
}

//����SoundTouch��������ʣ�������
int CSoundTouchEffectPitch::getRemainSamples()
{
	return this->m_pSoundTouch->numUnprocessedSamples() + this->m_pSoundTouch->numSamples();
}

//��������,�������Ĳ���Ӧ����-100~100֮��
void CSoundTouchEffectPitch::setPercentPitch(float percnetPitch)
{
	m_fPercnetPitch = (percnetPitch) / 10.0f;//m_fPercentPitch�ķ�Χ��[-10,10]
	m_pSoundTouch->setPitchSemiTones(m_fPercnetPitch);
}


/**
 * �����øú������б�������
 * ������
 * data: Դ���ݣ������������
 * perSamplesByte: Դ����ÿ��������ռ���ֽ���
 * sampleCounts����Ҫ�����������
 * threadEndFlag���߳̽�����־
 * 
 * ����ֵ�����ش���õ����ݣ�������ɵ������봫���������ݸ�ʽ����һ��
 * 
 * ��ע�����Դ���SoundTouch�����첽�������ݽ���soundTouch����󣬲��������ش���õ����ݣ�������Ҫ��Դ�����ͷ�
 */
uint8_t* CSoundTouchEffectPitch::getProcessData(uint8_t* data, int perSamplesByte, int sampleCounts, bool threadEndFlag)
{
	if (m_fPercnetPitch == 0.0)
	{
		return data;
	}
	if (data)
	{
		this->process(&data, perSamplesByte, sampleCounts);
	}
	if (!threadEndFlag)//�鿴�߳��Ƿ����
	{
		data = this->reciveSamples(perSamplesByte, sampleCounts);
	}
	else
	{
		data = this->reciveSamplesEnd(perSamplesByte, sampleCounts);
	}

	return data;
}

//�����������ת����SoundTouch��Ҫ�����ݸ�ʽ
int CSoundTouchEffectPitch::toSounTouchByte(uint8_t* data, int perSamplesByte, int samples)
{
	int bytesPerSample = perSamplesByte;
	int numBytes = samples * bytesPerSample;
	int numElems = numBytes / bytesPerSample;
	// swap byte ordert & convert to float, depending on sample format
	switch (bytesPerSample)
	{
		case 1:
		{
			unsigned char *temp2 = (unsigned char*)data;
			double conv = 1.0 / 128.0;
			for (int i = 0; i < numElems; i++)
			{
				this->m_pSoundTouchBuff[i] = (float)(temp2[i] * conv - 1.0);
			}
			break;
		}
		case 2:
		{
			short *temp2 = (short*)data;
			double conv = 1.0 / 32768.0;
			for (int i = 0; i < numElems; i++)
			{
				short value = temp2[i];
				this->m_pSoundTouchBuff[i] = (float)(_swap16(value) * conv);
			}
			break;
		}
		case 3:
		{
			char *temp2 = (char *)data;
			double conv = 1.0 / 8388608.0;
			for (int i = 0; i < numElems; i++)
			{
				int value = *((int*)temp2);
				value = _swap32(value) & 0x00ffffff;
				value |= (value & 0x00800000) ? 0xff000000 : 0;
				this->m_pSoundTouchBuff[i] = (float)(value * conv);
				temp2 += 3;
			}
			break;
		}
		case 4:
		{
			int *temp2 = (int *)data;
			double conv = 1.0 / 2147483648.0;
			assert(sizeof(int) == 4);
			for (int i = 0; i < numElems; i++)
			{
				int value = temp2[i];
				this->m_pSoundTouchBuff[i] = (float)(_swap32(value) * conv);
			}
			break;
		}
	}
	return numElems;
}

//��SoundTouch����õ�����ת����Ŀ�����ݸ�ʽ
void CSoundTouchEffectPitch::toDataBtye(uint8_t* data, int perSamplesByte, int samples)
{
	int bytesPerSample = perSamplesByte;
	int numBytes = samples * bytesPerSample;
	if (samples == 0) return;
	switch (bytesPerSample)
	{
		case 1:
		{
			unsigned char *temp2 = (unsigned char *)data;
			for (int i = 0; i < samples; i++)
			{
				temp2[i] = (unsigned char)saturate(this->m_pSoundTouchBuff[i] * 128.0f + 128.0f, 0.0f, 255.0f);
			}
			break;
		}

		case 2:
		{
			short *temp2 = (short *)data;
			for (int i = 0; i < samples; i++)
			{
				short value = (short)saturate(this->m_pSoundTouchBuff[i] * 32768.0f, -32768.0f, 32767.0f);
				temp2[i] = _swap16(value);
			}
			break;
		}

		case 3:
		{
			char *temp2 = (char *)data;
			for (int i = 0; i < samples; i++)
			{
				int value = saturate(this->m_pSoundTouchBuff[i] * 8388608.0f, -8388608.0f, 8388607.0f);
				*((int*)temp2) = _swap32(value);
				temp2 += 3;
			}
			break;
		}

		case 4:
		{
			int *temp2 = (int *)data;
			for (int i = 0; i < samples; i++)
			{
				int value = saturate(this->m_pSoundTouchBuff[i] * 2147483648.0f, -2147483648.0f, 2147483647.0f);
				temp2[i] = _swap32(value);
			}
			break;
		}
	}

}

//��������ת��Ϊ���������б��ʹ���
inline int CSoundTouchEffectPitch::saturate(float fvalue, float minval, float maxval)
{
	if (fvalue > maxval)
	{
		fvalue = maxval;
	}
	else if (fvalue < minval)
	{
		fvalue = minval;
	}
	return (int)fvalue;
}
