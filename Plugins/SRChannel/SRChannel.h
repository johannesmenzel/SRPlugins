#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "../../Classes/DSP/SRGain.h"
#include "../../Classes/DSP/SRSaturation.h"
#include "../../Classes/DSP/SRDynamics.h"
#include "../../Classes/DSP/SRBuffer.h"
#include "IControls.h"

const int kNumPresets = 1;

enum EParams
{
	// Level Stage
	kGainIn = 0,
	kGainOut,

	// Sat Stage
	kSaturationDrive,
	kSaturationAmount,
	kOversamplingRate,

	// EQ Stage

	// -- Filter
	kEqHpFreq,
	kEqLpFreq,

	// -- EQ
	kEqHfBoost,
	kEqHfCut,
	kEqHfFreq,
	kEqHfDs,
	kEqHfIsBell,

	kEqHmfGain,
	kEqHmfFreq,
	kEqHmfQ,
	kEqHmfDs,
	kEqHmfIsShelf,

	kEqLmfGain,
	kEqLmfFreq,
	kEqLmfQ,
	kEqLmfDs,
	kEqLmfIsShelf,

	kEqLfBoost,
	kEqLfCut,
	kEqLfFreq,
	kEqLfDs,
	kEqLfIsBell,

	kEqBandSolo,
	kEqAmount,

	// Compressor Stage

	// -- RMS Compressor
	kCompRmsThresh,
	kCompRmsRatio,
	kCompRmsAttack,
	kCompRmsRelease,
	kCompRmsKneeWidthDb,
	kCompRmsMakeup,
	kCompRmsMix,
	kCompRmsIsFeedback,
	kCompRmsIsExtSc,

	// -- Peak Compressor
	kCompPeakThresh,
	kCompPeakRatio,
	kCompPeakAttack,
	kCompPeakRelease,
	kCompPeakKneeWidthDb,
	kCompPeakMakeup,
	kCompPeakMix,
	kCompPeakSidechainFilterFreq,
	kCompPeakIsFeedback,
	kCompPeakIsExtSc,

	// -- Compressor Global
	kCompIsParallel,
	kCompPeakRmsRatio,

	// Output Stage
	kStereoPan,
	kStereoWidth,
	kStereoWidthLow,
	kStereoDepth,
	kStereoMonoFreq,
	kStereoIsPanMonoLow,

	kLimiterThresh,
	kClipperThresh,
	kIsAgc,

	// Bypasses
	kSatBypass,
	kEqBypass,
	kCompBypass,
	kOutputBypass,
	kBypass,

	// Number of parameters
	kNumParams
};
enum ECtrlTags {

	// Input Stage
	// Level Stage
	cGainIn = 0,
	cGainOut,

	// Sat Stage
	cSaturationDrive,
	cSaturationAmount,
	cOversamplingRate,

	// EQ Stage

	// -- Filter
	cEqHpFreq,
	cEqLpFreq,

	// -- EQ
	cEqHfBoost,
	cEqHfCut,
	cEqHfFreq,
	cEqHfDs,
	cEqHfIsBell,

	cEqHmfGain,
	cEqHmfFreq,
	cEqHmfQ,
	cEqHmfDs,
	cEqHmfIsShelf,

	cEqLmfGain,
	cEqLmfFreq,
	cEqLmfQ,
	cEqLmfDs,
	cEqLmfIsShelf,

	cEqLfBoost,
	cEqLfCut,
	cEqLfFreq,
	cEqLfDs,
	cEqLfIsBell,

	cEqBandSolo,
	cEqAmount,

	// Compressor Stage

	// -- RMS Compressor
	cCompRmsThresh,
	cCompRmsRatio,
	cCompRmsAttack,
	cCompRmsRelease,
	cCompRmsKneeWidthDb,
	cCompRmsMakeup,
	cCompRmsMix,
	cCompRmsIsFeedback,
	cCompRmsIsExtSc,

	// -- Peak Compressor
	cCompPeakThresh,
	cCompPeakRatio,
	cCompPeakAttack,
	cCompPeakRelease,
	cCompPeakKneeWidthDb,
	cCompPeakMakeup,
	cCompPeakMix,
	cCompPeakSidechainFilterFreq,
	cCompPeakIsFeedback,
	cCompPeakIsExtSc,

	// -- Compressor Global
	cCompIsParallel,
	cCompPeakRmsRatio,

	// Output Stage
	cStereoPan,
	cStereoWidth,
	cStereoWidthLow,
	cStereoDepth,
	cStereoMonoFreq,
	cStereoIsPanMonoLow,

	cLimiterThresh,
	cClipperThresh,
	cIsAgc,

	// Bypasses
	cSatBypass,
	cEqBypass,
	cCompBypass,
	cOutputBypass,
	cBypass,

	// UI Controls

	// -- Meter
	cMeterVu,
	cMeterGr,
	cMeterGrLim,
	cMeterFreqResponse,

	// Number of controls
	kNumCtrlTags,
};

using namespace iplug;
using namespace igraphics;

class SRChannel final : public Plugin
{
public:
	SRChannel(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
	void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
	void OnReset() override;
	void OnIdle() override;
	void OnParamChange(int paramIdx) override;

	void AdjustBandSolo();

private:
	void SetFreqMeterValues();

	double meterIn1, meterIn2, meterOut1, meterOut2;

	SR::DSP::SRGain fGainIn;
	SR::DSP::SRGain fGainOut;
	SR::DSP::SRGain fGainOutLow;

	SR::DSP::SRSaturation fSatInput[2] = { };

	SR::DSP::SRFilterIIR<sample, 2> fEqHp;
	SR::DSP::SRFilterIIR<sample, 2> fEqLp;
	SR::DSP::SRFilterIIR<sample, 2> fEqLfBoost;
	SR::DSP::SRFilterIIR<sample, 2> fEqLfCut;
	SR::DSP::SRFilterIIR<sample, 2> fEqHfBoost;
	SR::DSP::SRFilterIIR<sample, 2> fEqHfCut;
	SR::DSP::SRDeesser fEqLmf;
	SR::DSP::SRDeesser fEqHmf;
	SR::DSP::SRFilterIIR<sample, 2> fEqBandSolo;

	SR::DSP::SRFilterIIR<sample, 2> fSplitHp;
	SR::DSP::SRFilterIIR<sample, 2> fSplitLp;

	SR::DSP::SRCompressorRMS fCompRms;
	SR::DSP::SRCompressor fCompPeak;

	SR::DSP::SRDynamicsDetector fMeterEnvelope[4];

	IPeakSender<4, 1024> mMeterSender;
	IPeakSender<2, 1024> mMeterSenderGr;

	SR::DSP::SRBuffer<sample, 4, 1024> mBufferVu;
	SR::DSP::SRBuffer<sample, 2, 1024> mBufferMeterGr;
	SR::DSP::SRBuffer<sample, 2, 1024> mBufferLowSignal;

	float* mFreqMeterValues;

#endif
};
