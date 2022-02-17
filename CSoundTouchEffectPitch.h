/*********************************
描述: 用于录音实时变音处理
	 数据处理函数：getProcessData(),该函数并不会立马返回处理好的数据
	 音调设置：setPercentPitch();
	 返回缓冲区剩余样本数：getRemainSamples();
	 数据格式转换：toSounTouchByte()、toSounTouchByte()、saturate()
作者:zhongzhipeng
***********************************/
#pragma once
#include <memory>
#include "SoundTouch.h"


class CSoundTouchEffectPitch
{
public:
	CSoundTouchEffectPitch(int channelsSrc, int SampleRateSrc);
	~CSoundTouchEffectPitch();
	//将数据传入，返回处理好的数据，数据未处理好，返回空
	uint8_t* getProcessData(uint8_t* data, int perSamplesByte, int sampleCounts,bool threadEndFlag);
	
	int getRemainSamples();
	void setPercentPitch(float percnetPitch);

private:
	std::unique_ptr<soundtouch::SoundTouch> m_pSoundTouch;//soundTouch
	soundtouch::SAMPLETYPE* m_pSoundTouchBuff;//缓冲区(即将送入SoundTouch处理)
	int m_iChannels;//通道数
	float m_fPercnetPitch = 0.0;


private:
	int toSounTouchByte(uint8_t* data, int perSamplesByte, int samples);
	void toDataBtye(uint8_t* data, int perSamplesByte, int samples);
	inline int saturate(float fvalue, float minval, float maxval);

	void process(uint8_t** data, int perSamplesByte, int sampleCounts);
	uint8_t* reciveSamples(int perSamplesByte, int sampleCounts);
	uint8_t* reciveSamplesEnd(int perSamplesByte, int sampleCounts);
	
};