/*********************************
����: ����¼��ʵʱ��������
	 ���ݴ�������getProcessData(),�ú��������������ش���õ�����
	 �������ã�setPercentPitch();
	 ���ػ�����ʣ����������getRemainSamples();
	 ���ݸ�ʽת����toSounTouchByte()��toSounTouchByte()��saturate()
����:zhongzhipeng
***********************************/
#pragma once
#include <memory>
#include "SoundTouch.h"


class CSoundTouchEffectPitch
{
public:
	CSoundTouchEffectPitch(int channelsSrc, int SampleRateSrc);
	~CSoundTouchEffectPitch();
	//�����ݴ��룬���ش���õ����ݣ�����δ����ã����ؿ�
	uint8_t* getProcessData(uint8_t* data, int perSamplesByte, int sampleCounts,bool threadEndFlag);
	
	int getRemainSamples();
	void setPercentPitch(float percnetPitch);

private:
	std::unique_ptr<soundtouch::SoundTouch> m_pSoundTouch;//soundTouch
	soundtouch::SAMPLETYPE* m_pSoundTouchBuff;//������(��������SoundTouch����)
	int m_iChannels;//ͨ����
	float m_fPercnetPitch = 0.0;


private:
	int toSounTouchByte(uint8_t* data, int perSamplesByte, int samples);
	void toDataBtye(uint8_t* data, int perSamplesByte, int samples);
	inline int saturate(float fvalue, float minval, float maxval);

	void process(uint8_t** data, int perSamplesByte, int sampleCounts);
	uint8_t* reciveSamples(int perSamplesByte, int sampleCounts);
	uint8_t* reciveSamplesEnd(int perSamplesByte, int sampleCounts);
	
};