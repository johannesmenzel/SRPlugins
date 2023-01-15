#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "../../Classes/DSP/SRGain.h"
#include "../../Classes/DSP/SRDynamics.h"
#include "../../Classes/DSP/SRBuffer.h"
#include "IControls.h"

const int kNumPresets = 1;

enum EParams
{
	// Level Stage
	kInputGain = 0,
	kOutputGain,
	
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
	kEqHfIsSolo,

	kEqHmfGain,
	kEqHmfFreq,
	kEqHmfQ,
	kEqHmfDs,
	kEqHmfIsSolo,

	kEqLmfGain,
	kEqLmfFreq,
	kEqLmfQ,
	kEqLmfDs,
	kEqLmfIsSolo,

	kEqLfBoost,
	kEqLfCut,
	kEqLfFreq,
	kEqLfDs,
	kEqLfIsBell,
	kEqLfIsSolo,

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
	cInputGain = 0,
	cOutputGain,

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
	cEqHfIsSolo,

	cEqHmfGain,
	cEqHmfFreq,
	cEqHmfQ,
	cEqHmfDs,
	cEqHmfIsSolo,

	cEqLmfGain,
	cEqLmfFreq,
	cEqLmfQ,
	cEqLmfDs,
	cEqLmfIsSolo,

	cEqLfBoost,
	cEqLfCut,
	cEqLfFreq,
	cEqLfDs,
	cEqLfIsBell,
	cEqLfIsSolo,

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
	cMeterIn,
	cMeterOut,
	cMeterGrRms,
	cMeterGrPeak,
	cMeterGrLim,
	cMeterEqGraph,

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

#endif
};
