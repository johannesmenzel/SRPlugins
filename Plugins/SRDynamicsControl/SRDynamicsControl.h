#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "../../Classes/DSP/SRGain.h"
#include "../../Classes/DSP/SRDynamics.h"
#include "../../Classes/DSP/SRBuffer.h"
#include "IControls.h"

const int kNumPresets = 1;

enum EParams
{
	kInGain = 0,
	kOutGain,
	kThresh,
	kCrest,
	kRatio,
	kAttack,
	kRelease,
	kMix,
	kNumParams
};

enum ECtrlTags
{
	cInGain = 0,
	cOutGain,
	cThresh,
	cCrest,
	cRatio,
	cAttack,
	cRelease,
	cMix,
	cMeterIn,
	cMeterOut,
	cMeterGrLevel,
	cMeterGrOpto,
	cMeterGrVca,
	cMeterGrFet,
	cMeterGrLim,
	kNumCtrlTags
};

using namespace iplug;
using namespace igraphics;

class SRDynamicsControl final : public Plugin
{
public:
	SRDynamicsControl(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
	void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
	void SetCompressorValues();
	void OnReset() override;
	void OnIdle() override;
	void OnParamChange(int paramIdx) override;
private:
	SR::DSP::SRGain fInGain;
	SR::DSP::SRGain fOutGain;
	SR::DSP::SRCompressorRMS fCompLevel;
	SR::DSP::SRCompressorRMS fCompOpto;
	SR::DSP::SRCompressor fCompVca;
	SR::DSP::SRCompressor fCompFet;
	SR::DSP::SRLimiter fCompLim;
	//SR::DSP::SRDeesser fDeesser;
	//SR::DSP::SRDeesser fDeplosive;

	IPeakSender<2, 1024> mMeterSenderIn;
	IPeakSender<2, 1024> mMeterSenderOut;
	IPeakSender<1> mMeterSenderGrLevel;
	IPeakSender<1> mMeterSenderGrOpto;
	IPeakSender<1> mMeterSenderGrVca;
	IPeakSender<1> mMeterSenderGrFet;
	IPeakSender<1> mMeterSenderGrLim;
	SR::DSP::SRBuffer<sample, 1, 1024> mBufferMeterGrLevel;
	SR::DSP::SRBuffer<sample, 1, 1024> mBufferMeterGrOpto;
	SR::DSP::SRBuffer<sample, 1, 1024> mBufferMeterGrVca;
	SR::DSP::SRBuffer<sample, 1, 1024> mBufferMeterGrFet;
	SR::DSP::SRBuffer<sample, 1, 1024> mBufferMeterGrLim;
#endif
};