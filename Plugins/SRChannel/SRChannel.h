#pragma once

// Switch which filter library to use, if set up
// 1: Using SRFilters
// 2: Using DspFilters (Vinnie Falco)
// 3: Using Iir (Bernd Porr)
#define FLT 3
// Switch using passive or biquad filtering
// True: Process passive eq as parallel filters (Dry + Lowpass Boost + Lowpass Cut (flipped))
// False: Use ordinary biquad filters (Shelf, Peak)
// Switch using dummy parameters for modeling or use already modeled values
#define DUMMY true


#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#if FLT == 2
#include "DspFilters/Dsp.h"
#elif FLT == 3
#include "Iir.h"
#endif // !FLT
#include "../../Classes/DSP/SRFilters.h"
#include "../../Classes/DSP/SRGain.h"
#include "../../Classes/DSP/SRSaturation.h"
#include "../../Classes/DSP/SRDynamics.h"
#include "../../Classes/DSP/SRBuffer.h"
#include "../../Classes/DSP/AWTube2.h"

const int kNumPresets = 1;

enum EParams
{
	// 10 Dummy params for testing
	kDummy1 = 0,
	kDummy2,
	kDummy3,
	kDummy4,
	kDummy5,
	kDummy6,
	kDummy7,
	kDummy8,
	kDummy9,
	kDummy10,

	// Level Stage
	kGainIn,
	kGainOut,

	// Sat Stage
	kSaturationDrive,
	kTube,
	kOversamplingRate,

	// EQ Stage

	// -- Filter
	kEqHpFreq,
	kEqLpFreq,

	// -- Parametric EQ
	kEqLmfGain,
	kEqLmfFreq,
	kEqLmfQ,
	kEqLmfDs,
	kEqLmfIsShelf,

	kEqHmfGain,
	kEqHmfFreq,
	kEqHmfQ,
	kEqHmfDs,
	kEqHmfIsShelf,

	// -- Passive EQ
	kEqLfBoost,
	kEqLfCut,
	kEqLfFreq,
	kEqLfIsBell,

	kEqHfBoost,
	kEqHfCut,
	kEqHfBoostFreq,
	kEqHfBoostQ,
	kEqHfCutFreq,
	kEqHfIsBell,

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
	// 10 Dummy params for testing
	cDummy1 = 0,
	cDummy2,
	cDummy3,
	cDummy4,
	cDummy5,
	cDummy6,
	cDummy7,
	cDummy8,
	cDummy9,
	cDummy10,

	// Input Stage
	// Level Stage
	cGainIn,
	cGainOut,

	// Sat Stage
	cSaturationDrive,
	cTube,
	cOversamplingRate,

	// EQ Stage

	// -- Filter
	cEqHpFreq,
	cEqLpFreq,

	// -- Parametric EQ
	cEqLmfGain,
	cEqLmfFreq,
	cEqLmfQ,
	cEqLmfDs,
	cEqLmfIsShelf,

	cEqHmfGain,
	cEqHmfFreq,
	cEqHmfQ,
	cEqHmfDs,
	cEqHmfIsShelf,

	// -- Passive EQ
	cEqLfBoost,
	cEqLfCut,
	cEqLfFreq,
	cEqLfIsBell,
	
	cEqHfBoost,
	cEqHfCut,
	cEqHfBoostFreq,
	cEqHfBoostQ,
	cEqHfCutFreq,
	cEqHfIsBell,

	// -- Global EQ
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
	cMeterGrLevel,
	cMeterGrPeak,
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
	//~SRChannel(); // Actually we must delete the dynamically allocated mFreqMeterValues, but causes heap error

#if IPLUG_DSP // http://bit.ly/2S64BDd
	void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
	void OnReset() override;
	void OnIdle() override;
	void OnParamChange(int paramIdx) override;

private:
	void SetFreqMeterValues();
	void AdjustEqPassive();
	void AdjustBandSolo();

	// Statically allocated stereo pairs
	double mMeterIn[2], mMeterOut[2];
	
	SR::DSP::SRGain fGainIn;
	SR::DSP::SRGain fGainOut;
	SR::DSP::SRGain fGainOutLow;

	SR::DSP::SRSaturation fSatInput[2] = { };
	SR::DSP::Airwindows::Tube2 fSatTube2;

	SR::DSP::SRFilterIIR<sample, 2> fEqHp;
	SR::DSP::SRFilterIIR<sample, 2> fEqLp;

	SR::DSP::SRDeesser fEqLmf;
	SR::DSP::SRDeesser fEqHmf;


	// Statically allocated stereo pairs
	Iir::RBJ::LowPass fEqLfBoost[2];
	Iir::RBJ::LowPass fEqLfCut[2];
	Iir::RBJ::BandPass1 fEqHfBoost[2];
	Iir::RBJ::HighPass fEqHfCut[2];
	// Must be double vals for now, because must be calculated on parallel eq process.
	// Better having SRGain with param smooth, even better these filters had passband gain (with param smooth)
	SR::DSP::SRParamSmoothRamp fGainLfBoost, fGainLfCut, fGainHfBoost, fGainHfCut;
	//double mGainLfBoost, mGainLfCut, mGainHfBoost, mGainHfCut;

	SR::DSP::SRFilterIIR<sample, 2> fEqBandSolo;
	SR::DSP::SRFilterIIR<sample, 2> fSplitHp;
	SR::DSP::SRFilterIIR<sample, 2> fSplitLp;

	SR::DSP::SRCompressorRMS fCompRms;
	SR::DSP::SRCompressor fCompPeak;

	SR::DSP::SRDynamicsDetector fMeterEnvelope[4];

	IPeakSender<4, 1024> mMeterSender;
	IPeakSender<1, 1024> mMeterSenderGrLevel;
	IPeakSender<1, 1024> mMeterSenderGrPeak;

	// TODO: Add VU on top of Peak Meter
	SR::DSP::SRBuffer<sample, 4, 1024> mBufferMeterPeak;
	SR::DSP::SRBuffer<sample, 1, 1024> mBufferMeterGrLevel;
	SR::DSP::SRBuffer<sample, 1, 1024> mBufferMeterGrPeak;
	SR::DSP::SRBuffer<sample, 2, 1024> mBufferLowSignal; // Eval if needed, maybe a double var is sufficient

	// Dynamically allocated values, must be deleted in ~
	float* mFreqMeterValues;

#endif
};
